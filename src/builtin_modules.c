#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>

#include "jv.h"
#include "jv_unicode.h"
#include "jq.h"
#include "jq_plugin.h"
#include "jv_alloc.h"
#include "linker.h"
#include "util.h"
#include "builtin_modules.h"

/*
 * This file defines builtin modules that can be imported or included by
 * jq programs or jq modules.
 */

/*
 * I/O module
 */

struct io_handle {
  FILE *f;
  jv_parser *parser;
  jv fn;
  jv cmd;
  jv mode;
  enum jv_parse_flags in_options;
  enum jv_print_flags out_options;
  char buf[4096 - (sizeof(FILE *) + sizeof(jv_parser *) + sizeof(jv))];
};

static jv allow_io(jq_state *jq, const char *what, jv req, jv s, jv mode) {
  jv ret = jq_io_policy_check(jq, req);

  if (jv_get_kind(ret) == JV_KIND_TRUE)
    return ret;

  const char *reason = "<unspecified>";
  jv junk = jv_null();

  if (jv_get_kind(ret) == JV_KIND_INVALID && jv_invalid_has_msg(jv_copy(ret))) {
    junk = jv_invalid_get_msg(jv_copy(ret));
    if (jv_get_kind(junk) == JV_KIND_STRING)
      reason = jv_string_value(junk);
  }
  jv_free(ret);
  ret = jv_invalid_with_msg(jv_string_fmt("%s: Use of %s (%s) rejected by policy: %s", what,
                                          jv_string_value(s), jv_string_value(mode), reason));
  jv_free(junk);
  return ret;
}

static jv f_close(jq_state *jq, jv handle, void *d) {
  struct io_handle *h = d;
  jv ret = jv_invalid();

  if (!h) {
    jv_free(handle);
    return ret;
  }
  if (h->f && h->f != stdin && h->f != stdout && h->f != stderr)
    ret = jv_number(fclose(h->f));
  if (h->parser)
    jv_parser_free(h->parser);
  jv_free(h->mode);
  jv_free(h->fn);
  jv_free(h->cmd);
  jv_free(handle);
  free(h);
  return jv_is_valid(ret) ? ret : jv_number(0);
}

static jv builtin_jq_fhclose(jq_state *jq, jv input) {
  return jq_handle_close(jq, input);
}

static jv f_reset(jq_state *jq, jv handle, void *vh) {
  struct io_handle *h = vh;
  if (!h || !h->f)
    return jv_invalid_with_msg(jv_string("No such file handle"));
  clearerr(h->f);
  fseek(h->f, 0, SEEK_SET);
  return jv_true();
}

#ifndef HAVE_GETLINE
#ifdef getline
#undef getline
#endif
#define getline my_getline
static ssize_t getline(char **lp, size_t *np, FILE *f) {
  size_t len = 0;
  int save_errno = errno;

  if (*lp == 0 && *np == 0)
    *lp = jv_mem_calloc(1, (*np = 128));

  while (strchr((*lp) + len, '\n') == 0) {
    if (len == (*np) - 1) {
      size_t addition = (*np >> 1) + 32;
      *lp = jv_mem_realloc(*lp, (*np) + addition);
      *np += addition;
      memset((*lp) + len, 0, addition);
    }
    if (*np > INT_MAX) {
      errno = ERANGE;
      return -1;
    }
    if (fgets((*lp) + len, (*np) - len, f)) {
      len += strlen((*lp) + len);
      if ((*lp)[len - 1] == '\n')
        break;
      continue;
    }
    if (errno || ferror(f))
      return -1;
    /* EOF */
    if (len == 0)
      return -1;
    break;
  }
  errno = save_errno;
  return len;
}
#endif

static jv f_getline(jq_state *jq, struct io_handle *h) {
  if (!h || !h->f)
    return jv_invalid_with_msg(jv_string("No such file handle"));

  char *s = 0;
  size_t n = 0;
  ssize_t bytes;
  int save_errno = errno;

  errno = 0;
  if ((bytes = getline(&s, &n, h->f)) < 0) {
    free(s);
    if (feof(h->f))
      return jv_invalid_with_msg(jv_string("EOF"));
    if (errno)
      return jv_invalid_with_msg(jv_string(strerror(errno)));
    return jv_invalid_with_msg(jv_string("Unspecified I/O error"));
  }
  if (bytes > 1 && s[bytes-2] == '\r' && s[bytes-1] == '\n')
    s[bytes-2] = '\0';
  else if (bytes > 0 && s[bytes-1] == '\n')
    s[bytes-1] = '\0';
  errno = save_errno;
  jv ret = jv_string(s);
  free(s);
  return ret;
}

static jv f_read(jq_state *jq, jv input, void *vh) {
  struct io_handle *h = vh;
  jv_free(input);
  if (!h || !h->f)
    return jv_invalid_with_msg(jv_string("No such file handle"));
  if (!h->parser)
    return f_getline(jq, h);

  jv v = jv_parser_next(h->parser);
  if (jv_is_valid(v) || jv_invalid_has_msg(jv_copy(v)))
    return v;

  /* Need to read more */

  // To avoid mangling UTF-8 multi-byte sequences that cross the end of our read
  // buffer, we need to be able to read the remainder of a sequence and add that
  // before appending.
  const int max_utf8_len = 4;
  while (!feof(h->f) && !ferror(h->f)) {
    h->buf[0] = 0;
    fgets(h->buf, sizeof(h->buf) - max_utf8_len, h->f);
    size_t n = strlen(h->buf);
    if (n == 0)
      continue; /* breaks anyways bc EOF or error */

    int missing = 0;
    if (jvp_utf8_backtrack(h->buf+(n-1), h->buf, &missing) && missing > 0 &&
        !feof(h->f) && !ferror(h->f)) {
      off_t off = ftello(h->f);
      n += fread(h->buf+n, 1, missing, h->f);
      /* Check that we got what we needed! */
      jvp_utf8_backtrack(h->buf+(n-1), h->buf, &missing);
      if (missing)
        return jv_invalid_with_msg(jv_string_fmt("File contents appears truncated in %s at offset %lld",
                                                 jv_string_value(h->fn), (long long)off));
    }
    jv_parser_set_buf(h->parser, h->buf, n, !feof(h->f));
    if (jv_is_valid((v = jv_parser_next(h->parser))) || jv_invalid_has_msg(jv_copy(v)))
      return v;
  }
  return jv_invalid_with_msg(jv_string("EOF"));
}

static jv f_write(jq_state *jq, jv input, void *vh, jv output) {
  struct io_handle *h = vh;

  if (!h->f) {
    jv_free(output);
    return jv_invalid_with_msg(jv_string("fhwrite: no such file handle"));
  }
  if (!jv_is_valid(output)) {
    jv_free(output);
    return jv_invalid_with_msg(jv_string("fhwrite: invalid output"));
  }

  jv actual_output;
  if (jv_get_kind(output) == JV_KIND_STRING &&
      (h->out_options & JV_PRINT_RAW))
    actual_output = jv_copy(output);
  else
    actual_output = jv_dump_string(jv_copy(output), h->out_options);

  size_t bytes;
  int len = jv_string_length_bytes(jv_copy(actual_output));
  const char *p = jv_string_value(actual_output);
  while (len > 0) {
    if ((bytes = fwrite(p, 1, len, h->f)) > 0) {
      len -= bytes;
      p += bytes;
    }
  }
  if (len == 0 && !(h->out_options & JV_PRINT_NOLF)) {
    if ((bytes = fwrite("\n", 1, 1, h->f)) == 0)
      len = 1;
  }

  jv ret = len > 0 ? jv_invalid_with_msg(jv_string("Partial write")) :
                     jv_copy(output);
  jv_free(actual_output);
  jv_free(output);
  return ret;
}

static jv f_eof(jq_state *jq, jv input, void *vh) {
  struct io_handle *h = vh;
  jv_free(input);
  if (!h || !h->f)
    return jv_invalid_with_msg(jv_string("No such file handle"));
  return feof(h->f) ? jv_true() : jv_false();
}

static jv builtin_jq_fhname(jq_state *jq, jv fh) {
  struct io_handle *h = jq_handle_get(jq, fh);

  if (h && jv_is_valid(h->fn))
    return jv_copy(h->fn);
  return jv_invalid_with_msg(jv_string("filename for filehandle not known"));
}

struct jq_io_table iovt = {
  .kind    = "FILE",
  .fhclose = f_close,
  .fhreset = f_reset,
  .fhwrite = f_write,
  .fhread  = f_read,
  .fhstat  = 0,
  .fheof   = f_eof,
};


static jv builtin_jq_fopen(jq_state *jq, jv input, jv fn, jv mode, jv in_opts, jv out_opts) {
  jv_free(input);
  jv ret = jv_true();

  if (jv_get_kind(fn) != JV_KIND_STRING)
    ret = jv_invalid_with_msg(jv_string("fopen: filename must be a string"));
  if (jv_is_valid(ret) && jv_get_kind(mode) != JV_KIND_STRING)
    ret = jv_invalid_with_msg(jv_string("fopen: mode must be a string"));
  if (jv_is_valid(ret) && !jv_is_valid((in_opts = jv_parse_options(in_opts))))
    ret = jv_invalid_with_msg(jv_string("fopen: invalid input options"));
  if (jv_is_valid(ret) && !jv_is_valid((out_opts = jv_dump_options(out_opts))))
    ret = jv_invalid_with_msg(jv_string("fopen: invalid output options"));
  if (jv_is_valid(ret)) {
    ret = allow_io(jq, "fopen",
                   JV_OBJECT(jv_string("kind"), jv_string("file"),
                             jv_string("name"), jv_copy(fn),
                             jv_string("mode"), jv_copy(mode)),
                   fn, mode);
  }
  if (!jv_is_valid(ret))
    goto out;

  FILE *f = 0;
  /*
   * On some systems calling fopen("/dev/stdin","r") when stdin is a
   * pipe can fail!  Besides, this lets us support these three "devices"
   * on Windows.  And we avoid the Linux /dev/fd semantics.
   */
  if (strcmp(jv_string_value(fn), "/dev/stdin") == 0)
    f = stdin;
  else if (strcmp(jv_string_value(fn), "/dev/stdout") == 0)
    f = stdout;
  else if (strcmp(jv_string_value(fn), "/dev/stderr") == 0)
    f = stderr;
  else
    f = fopen(jv_string_value(fn), jv_string_value(mode));
  if (f) {
    struct io_handle *h = jv_mem_alloc(sizeof(*h));
    h->f = f;
    h->fn = jv_copy(fn);
    h->cmd = jv_invalid();
    h->mode = jv_copy(mode);
    h->in_options = jv_number_value(in_opts);
    h->out_options = jv_number_value(out_opts);
    if (h->in_options & JV_PARSE_RAW)
      h->parser = 0;
    else
      h->parser = jv_parser_new(0);
    ret = jq_handle_new(jq, "FILE", &iovt, h);
  } else {
    ret = jv_invalid_with_msg(jv_string_fmt("fopen: Failed to open %s: %s", jv_string_value(fn), strerror(errno)));
  }

out:
  jv_free(out_opts);
  jv_free(in_opts);
  jv_free(mode);
  jv_free(fn);
  return ret;
}


static jv builtin_jq_fhcmd(jq_state *jq, jv fh) {
  struct io_handle *h = jq_handle_get(jq, fh);

  if (h && jv_is_valid(h->cmd))
    return jv_copy(h->cmd);
  return jv_invalid_with_msg(jv_string("command for filehandle not known"));
}

static jv builtin_jq_popen(jq_state *jq, jv input, jv cmd, jv mode, jv opts) {
  jv_free(input);

  jv ret = jv_null();
  if (jv_get_kind(cmd) != JV_KIND_STRING)
    ret = jv_invalid_with_msg(jv_string("popen: filename must be a string"));
  if (jv_is_valid(ret) && jv_get_kind(mode) != JV_KIND_STRING)
    ret = jv_invalid_with_msg(jv_string("popen: mode must be a string"));

  char type = 0;
  if (jv_is_valid(ret)) {
    type = jv_string_value(mode)[0];
    if (type != 'r' && type != 'w')
      ret = jv_invalid_with_msg(jv_string("popen: mode must be \"r\" or \"w\""));
  }
  if (type == 'r' && !jv_is_valid((opts = jv_parse_options(opts))))
    ret = jv_invalid_with_msg(jv_string("popen: invalid input options"));
  else if (type == 'w' && !jv_is_valid((opts = jv_dump_options(opts))))
    ret = jv_invalid_with_msg(jv_string("popen: invalid output options"));
  if (jv_is_valid(ret)) {
    ret = allow_io(jq, "popen",
                   JV_OBJECT(jv_string("kind"), jv_string("popen"),
                             jv_string("name"), jv_copy(cmd),
                             jv_string("mode"), jv_copy(mode)),
                   cmd, mode);
  }
  if (!jv_is_valid(ret))
    goto out;

  FILE *f = 0;
  f = popen(jv_string_value(cmd), jv_string_value(mode));
  if (f) {
    struct io_handle *h = jv_mem_alloc(sizeof(*h));
    h->f = f;
    h->fn = jv_invalid();
    h->cmd = jv_copy(cmd);
    h->mode = jv_copy(mode);
    if (type == 'r') {
      h->in_options = jv_number_value(opts);
      h->out_options = 0;
    } else {
      h->in_options = 0;
      h->out_options = jv_number_value(opts);
    }
    if (h->in_options & JV_PARSE_RAW)
      h->parser = 0;
    else
      h->parser = jv_parser_new(0);
    ret = jq_handle_new(jq, "FILE", &iovt, h);
  } else {
    ret = jv_invalid_with_msg(jv_string_fmt("popen: Failed to execute %s: %s", jv_string_value(cmd), strerror(errno)));
  }

out:
  jv_free(opts);
  jv_free(mode);
  jv_free(cmd);
  return ret;
}

static jv builtin_jq_system(jq_state *jq, jv input) {
  if (jv_get_kind(input) != JV_KIND_STRING) {
    jv_free(input);
    return jv_invalid_with_msg(jv_string("system: input must be a string"));
  }
  return jv_number(system(jv_string_value(input)));
}

JQ_BUILTIN_INIT_FUN(builtin_jq_io_init,
                 "def fopen($fn; $mode; $inopts; $outopts): \n"
                 "    label $out | _fopen($fn; $mode; $inopts; $outopts) as $fh\n"
                 "  | try $fh catch (\n"
                 "        . as $error|$fh|_fhname as $fn|fhclose|$error\n"
                 "      | if . == \"EOF\" then break $out\n"
                 "        else \"Error reading from \\($fn): \\($error)\"|error end);\n"
                 "def fopen($fn; $mode): fopen($fn; $mode; null; null);\n"
                 "def fopen($fn): fopen($fn; \"r\");\n"
                 "def fopen: fopen(.; \"r\");\n"
                 "def streamfile($fn; $opts):\n"
                 "    label $out |\n"
                 "    fopen($fn; \"r\"; $opts; null)|\n"
                 "    repeat(try fhread catch if .==\"EOF\" then break $out else error end);\n"
                 "def streamfile: streamfile(.; null);\n"
                 "def slurpfile($fn; $opts): [streamfile($fn; $opts)];\n"
                 "def slurpfile: [streamfile];\n"
                 "def appendfile($fn; $opts; contents):\n"
                 "    fhwrite(fopen($fn; \"a\"; null; $opts); contents);\n"
                 "def appendfile(contents): appendfile(.; null; contents);\n"
                 "def writefile($fn; $opts; contents):\n"
                 "    fhwrite(fopen($fn; \"w\"; null; $opts); contents);\n"
                 "def writefile(contents): writefile(.; null; contents);\n",
                 {
                   .fptr = (cfunction_ptr)builtin_jq_fopen,
                   .name = "_fopen",
                   .nargs = 5,
                   .pure = 0,
                   .exported = 0
                 },
                 {
                   .fptr = (cfunction_ptr)builtin_jq_fhname,
                   .name = "_fhname",
                   .nargs = 1,
                   .pure = 0,
                   .exported = 1
                 },
                 )

JQ_BUILTIN_INIT_FUN(builtin_jq_proc_init,
                 "def popen($fn; $mode; $opts): \n"
                 "    label $out | _popen($fn; $mode; $opts) as $fh\n"
                 "  | try $fh catch (\n"
                 "        . as $error|$fh|_fhcmd as $fn|fhclose|$error\n"
                 "      | if . == \"EOF\" then break $out\n"
                 "        else \"Error reading from \\($fn): \\($error)\"|error end);\n"
                 "def popen($fn; $mode): popen($fn; $mode; null);\n"
                 "def popen($fn): popen($fn; \"r\");\n"
                 "def popen: popen(.; \"r\");\n"
                 "def streamcmd($fn; $opts):\n"
                 "    label $out |\n"
                 "    popen($fn; \"r\"; $opts)|\n"
                 "    repeat(try fhread catch if .==\"EOF\" then break $out else error end);\n"
                 "def streamcmd: streamcmd(.; null);\n"
                 "def slurpcmd($fn; $opts): [streamcmd($fn; $opts)];\n"
                 "def slurpcmd: [streamcmd];\n"
                 "def writecmd($fn; $opts; contents):\n"
                 "    fhwrite(popen($fn; \"w\"; $opts); contents);\n"
                 "def writecmd(contents): writecmd(.; null; contents);\n",
                 {
                   .fptr = (cfunction_ptr)builtin_jq_popen,
                   .name = "_popen",
                   .nargs = 4,
                   .pure = 0,
                   .exported = 0
                 },
                 {
                   .fptr = (cfunction_ptr)builtin_jq_system,
                   .name = "system",
                   .nargs = 1,
                   .pure = 0,
                   .exported = 1
                 },
                 {
                   .fptr = (cfunction_ptr)builtin_jq_fhcmd,
                   .name = "_fhcmd",
                   .nargs = 1,
                   .pure = 0,
                   .exported = 1
                 },
                 )

/* Builtin module "files" */
static struct jq_builtin_module builtin_modules[] = {
  {
    .name = "jq" JQ_PATH_SEP "io.jq",
    .contents = "module {cfunctions:\"io\"};",
    .init = 0,
  },
  {
    .name = "jq" JQ_PATH_SEP "io" JQ_DLL_EXT,
    .contents = 0,
    .init = builtin_jq_io_init,
  },
  {
    .name = "jq" JQ_PATH_SEP "proc.jq",
    .contents = "module {cfunctions:\"proc\"};",
    .init = 0,
  },
  {
    .name = "jq" JQ_PATH_SEP "proc" JQ_DLL_EXT,
    .contents = 0,
    .init = builtin_jq_proc_init,
  },
};

struct jq_builtin_module *jq_builtin_modules = builtin_modules;
size_t jq_builtin_nmodules = sizeof(builtin_modules) / sizeof(builtin_modules[0]);
