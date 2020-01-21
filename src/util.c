
#ifdef HAVE_MEMMEM
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif !defined alloca
# ifdef __GNUC__
#  define alloca __builtin_alloca
# elif defined _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# elif !defined HAVE_ALLOCA
#  ifdef  __cplusplus
extern "C"
#  endif
void *alloca (size_t);
# endif
#endif
#ifndef WIN32
#include <pwd.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <processenv.h>
#include <shellapi.h>
#include <wchar.h>
#include <wtypes.h>
#else
#include <signal.h>
#include <spawn.h>
#endif


#include "util.h"
#include "jq.h"
#include "jv_alloc.h"

#ifdef WIN32
FILE *fopen(const char *fname, const char *mode) {
  size_t sz = sizeof(wchar_t) * MultiByteToWideChar(CP_UTF8, 0, fname, -1, NULL, 0);
  wchar_t *wfname = alloca(sz + 2); // +2 is not needed, but just in case
  MultiByteToWideChar(CP_UTF8, 0, fname, -1, wfname, sz);

  sz = sizeof(wchar_t) * MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);
  wchar_t *wmode = alloca(sz + 2); // +2 is not needed, but just in case
  MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, sz);
  return _wfopen(wfname, wmode);
}
#endif

#ifndef HAVE_MKSTEMP
int mkstemp(char *template) {
  size_t len = strlen(template);
  int tries=5;
  int fd;

  // mktemp() truncates template when it fails
  char *s = alloca(len + 1);
  assert(s != NULL);
  strcpy(s, template);

  do {
    // Restore template
    strcpy(template, s);
    (void) mktemp(template);
    fd = open(template, O_CREAT | O_EXCL | O_RDWR, 0600);
  } while (fd == -1 && tries-- > 0);
  return fd;
}
#endif

jv expand_path(jv path) {
  assert(jv_get_kind(path) == JV_KIND_STRING);
  const char *pstr = jv_string_value(path);
  jv ret = path;
  if (jv_string_length_bytes(jv_copy(path)) > 1 && pstr[0] == '~' && pstr[1] == '/') {
    jv home = get_home();
    if (jv_is_valid(home)) {
      ret = jv_string_fmt("%s/%s",jv_string_value(home),pstr+2);
      jv_free(home);
    } else {
      jv emsg = jv_invalid_get_msg(home);
      ret = jv_invalid_with_msg(jv_string_fmt("Could not expand %s. (%s)", pstr, jv_string_value(emsg)));
      jv_free(emsg);
    }
    jv_free(path);
  }
  return ret;
}

jv get_home() {
  jv ret;
  char *home = getenv("HOME");
  if (!home) {
#ifndef WIN32
    struct passwd* pwd = getpwuid(getuid());
    if (pwd)
      ret = jv_string(pwd->pw_dir);
    else
      ret = jv_invalid_with_msg(jv_string("Could not find home directory."));
#else
    home = getenv("USERPROFILE");
    if (!home) {
      char *hd = getenv("HOMEDRIVE");
      if (!hd) hd = "";
      home = getenv("HOMEPATH");
      if (!home) {
        ret = jv_invalid_with_msg(jv_string("Could not find home directory."));
      } else {
        ret = jv_string_fmt("%s%s",hd,home);
      }
    } else {
      ret = jv_string(home);
    }
#endif
  } else {
    ret = jv_string(home);
  }
  return ret;
}


jv jq_realpath(jv path) {
  int path_max;
  char *buf = NULL;
#ifdef _PC_PATH_MAX
  path_max = pathconf(jv_string_value(path),_PC_PATH_MAX);
#else
  path_max = PATH_MAX;
#endif
  if (path_max > 0) {
     buf = jv_mem_alloc(path_max);
  }
#ifdef WIN32
  char *tmp = _fullpath(buf, jv_string_value(path), path_max);
#else
  char *tmp = realpath(jv_string_value(path), buf);
#endif
  if (tmp == NULL) {
    free(buf);
    return path;
  }
  jv_free(path);
  path = jv_string(tmp);
  free(tmp);
  return path;
}

const void *_jq_memmem(const void *haystack, size_t haystacklen,
                       const void *needle, size_t needlelen) {
#ifdef HAVE_MEMMEM
  return (const void*)memmem(haystack, haystacklen, needle, needlelen);
#else
  const char *h = haystack;
  const char *n = needle;
  size_t hi, hi2, ni;

  if (haystacklen < needlelen || haystacklen == 0)
    return NULL;
  for (hi = 0; hi < (haystacklen - needlelen + 1); hi++) {
    for (ni = 0, hi2 = hi; ni < needlelen; ni++, hi2++) {
      if (h[hi2] != n[ni])
        goto not_this;
    }

    return &h[hi];

not_this:
    continue;
  }
  return NULL;
#endif /* !HAVE_MEMMEM */
}

struct jq_util_input_state {
  jq_util_msg_cb err_cb;
  void *err_cb_data;
  jv_parser *parser;
  FILE* current_input;
  char **files;
  int nfiles;
  int curr_file;
  int failures;
  jv slurped;
  char buf[4096];
  size_t buf_valid_len;
  jv current_filename;
  size_t current_line;
};

static void fprinter(void *data, const char *fname) {
  fprintf((FILE *)data, "jq: error: Could not open file %s: %s\n", fname, strerror(errno));
}

// If parser == NULL -> RAW
jq_util_input_state *jq_util_input_init(jq_util_msg_cb err_cb, void *err_cb_data) {
  if (err_cb == NULL) {
    err_cb = fprinter;
    err_cb_data = stderr;
  }
  jq_util_input_state *new_state = jv_mem_calloc(1, sizeof(*new_state));
  new_state->err_cb = err_cb;
  new_state->err_cb_data = err_cb_data;
  new_state->slurped = jv_invalid();
  new_state->current_filename = jv_invalid();

  return new_state;
}

void jq_util_input_set_parser(jq_util_input_state *state, jv_parser *parser, int slurp) {
  assert(!jv_is_valid(state->slurped));
  state->parser = parser;

  if (parser == NULL && slurp)
    state->slurped = jv_string("");
  else if (slurp)
    state->slurped = jv_array();
  else
    state->slurped = jv_invalid();
}

void jq_util_input_free(jq_util_input_state **state) {
  jq_util_input_state *old_state = *state;
  *state = NULL;
  if (old_state == NULL)
    return;

  if (old_state->parser != NULL)
    jv_parser_free(old_state->parser);
  for (int i = 0; i < old_state->nfiles; i++)
    free(old_state->files[i]);
  free(old_state->files);
  jv_free(old_state->slurped);
  jv_free(old_state->current_filename);
  jv_mem_free(old_state);
}

void jq_util_input_add_input(jq_util_input_state *state, const char *fname) {
  state->files = jv_mem_realloc(state->files, (state->nfiles + 1) * sizeof(state->files[0]));
  state->files[state->nfiles++] = jv_mem_strdup(fname);
}

int jq_util_input_errors(jq_util_input_state *state) {
  return state->failures;
}

static const char *next_file(jq_util_input_state *state) {
  if (state->curr_file < state->nfiles)
    return state->files[state->curr_file++];
  return NULL;
}

static int jq_util_input_read_more(jq_util_input_state *state) {
  if (!state->current_input || feof(state->current_input) || ferror(state->current_input)) {
    if (state->current_input && ferror(state->current_input)) {
      // System-level input error on the stream. It will be closed (below).
      // TODO: report it. Can't use 'state->err_cb()' as it is hard-coded for
      //       'open' related problems.
      fprintf(stderr,"jq: error: %s\n", strerror(errno));
    }
    if (state->current_input) {
      if (state->current_input == stdin) {
        clearerr(stdin); // perhaps we can read again; anyways, we don't fclose(stdin)
      } else {
        fclose(state->current_input);
      }
      state->current_input = NULL;
      jv_free(state->current_filename);
      state->current_filename = jv_invalid();
      state->current_line = 0 ;
    }
    const char *f = next_file(state);
    if (f != NULL) {
      if (!strcmp(f, "-")) {
        state->current_input = stdin;
        state->current_filename = jv_string("<stdin>");
      } else {
        state->current_input = fopen(f, "r");
        state->current_filename = jv_string(f);
        if (!state->current_input) {
          state->err_cb(state->err_cb_data, f);
          state->failures++;
        }
      }
      state->current_line = 0;
    }
  }

  state->buf[0] = 0;
  state->buf_valid_len = 0;
  if (state->current_input) {
    char *res;
    memset(state->buf, 0xff, sizeof(state->buf));

    while (!(res = fgets(state->buf, sizeof(state->buf), state->current_input)) &&
           ferror(state->current_input) && errno == EINTR)
      clearerr(state->current_input);
    if (res == NULL) {
      state->buf[0] = 0;
      if (ferror(state->current_input))
        state->failures++;
    } else {
      const char *p = memchr(state->buf, '\n', sizeof(state->buf));

      if (p != NULL)
        state->current_line++;

      if (p == NULL && state->parser != NULL) {
        /*
         * There should be no NULs in JSON texts (but JSON text
         * sequences are another story).
         */
        state->buf_valid_len = strlen(state->buf);
      } else if (p == NULL && feof(state->current_input)) {
        size_t i;

        /*
         * XXX We don't know how many bytes we've read!
         *
         * We can't use getline() because there need not be any newlines
         * in the input.  The only entirely correct choices are: use
         * fgetc() or fread().  Using fread() will complicate buffer
         * management here.
         *
         * For now we check how much fgets() read by scanning backwards for the
         * terminating '\0'. This only works because we previously memset our
         * buffer with something nonzero.
         */
        for (p = state->buf, i = sizeof(state->buf) - 1; i > 0; i--) {
          if (state->buf[i] == '\0')
            break;
        }
        state->buf_valid_len = i;
      } else if (p == NULL) {
        state->buf_valid_len = sizeof(state->buf) - 1;
      } else {
        state->buf_valid_len = (p - state->buf) + 1;
      }
    }
  }
  return state->curr_file == state->nfiles &&
      (!state->current_input || feof(state->current_input) || ferror(state->current_input));
}

jv jq_util_input_next_input_cb(jq_state *jq, void *data) {
  return jq_util_input_next_input((jq_util_input_state *)data);
}

// Return the current_filename:current_line
jv jq_util_input_get_position(jq_state *jq) {
  jq_input_cb cb = NULL;
  void *cb_data = NULL;
  jq_get_input_cb(jq, &cb, &cb_data);
  assert(cb == jq_util_input_next_input_cb);
  if (cb != jq_util_input_next_input_cb)
    return jv_invalid_with_msg(jv_string("Invalid jq_util_input API usage"));
  jq_util_input_state *s = (jq_util_input_state *)cb_data;

  // We can't assert that current_filename is a string because if
  // the error was a JSON parser error then we may not have set
  // current_filename yet.
  if (jv_get_kind(s->current_filename) != JV_KIND_STRING)
    return jv_string("<unknown>");

  jv v = jv_string_fmt("%s:%lu", jv_string_value(s->current_filename), (unsigned long)s->current_line);
  return v;
}

jv jq_util_input_get_current_filename(jq_state* jq) {
  jq_input_cb cb=NULL;
  void *cb_data=NULL;
  jq_get_input_cb(jq, &cb, &cb_data);
  if (cb != jq_util_input_next_input_cb)
    return jv_invalid_with_msg(jv_string("Unknown input filename"));
  jq_util_input_state *s = (jq_util_input_state *)cb_data;
  jv v = jv_copy(s->current_filename);
  return v;
}

jv jq_util_input_get_current_line(jq_state* jq) {
  jq_input_cb cb=NULL;
  void *cb_data=NULL;
  jq_get_input_cb(jq, &cb, &cb_data);
  if (cb != jq_util_input_next_input_cb)
    return jv_invalid_with_msg(jv_string("Unknown input line number"));
  jq_util_input_state *s = (jq_util_input_state *)cb_data;
  jv v = jv_number(s->current_line);
  return v;
}


// Blocks to read one more input from stdin and/or given files
// When slurping, it returns just one value
jv jq_util_input_next_input(jq_util_input_state *state) {
  int is_last = 0;
  int has_more = 0;
  jv value = jv_invalid(); // need more input
  do {
    if (state->parser == NULL) {
      // Raw input
      is_last = jq_util_input_read_more(state);
      if (state->buf_valid_len == 0)
        continue;
      if (jv_is_valid(state->slurped)) {
        // Slurped raw input
        state->slurped = jv_string_concat(state->slurped, jv_string_sized(state->buf, state->buf_valid_len));
      } else {
        if (!jv_is_valid(value))
          value = jv_string("");
        if (state->buf[state->buf_valid_len-1] == '\n') {
          // whole line
          state->buf[state->buf_valid_len-1] = 0;
          return jv_string_concat(value, jv_string_sized(state->buf, state->buf_valid_len-1));
        }
        value = jv_string_concat(value, jv_string_sized(state->buf, state->buf_valid_len));
        state->buf[0] = '\0';
        state->buf_valid_len = 0;
      }
    } else {
      if (jv_parser_remaining(state->parser) == 0) {
        is_last = jq_util_input_read_more(state);
        if (is_last && state->buf_valid_len == 0)
          value = jv_invalid();
        jv_parser_set_buf(state->parser, state->buf, state->buf_valid_len, !is_last);
      }
      value = jv_parser_next(state->parser);
      if (jv_is_valid(state->slurped)) {
        // When slurping an input that doesn't have a trailing newline,
        // we might have more than one value on the same line, so let's check
        // to see if we have more data to parse.
        has_more = jv_parser_remaining(state->parser);
        if (jv_is_valid(value)) {
          state->slurped = jv_array_append(state->slurped, value);
          value = jv_invalid();
        } else if (jv_invalid_has_msg(jv_copy(value)))
          return value; // Not slurped parsed input
      } else if (jv_is_valid(value) || jv_invalid_has_msg(jv_copy(value))) {
        return value;
      }
    }
  } while (!is_last || has_more);

  if (jv_is_valid(state->slurped)) {
    value = state->slurped;
    state->slurped = jv_invalid();
  }
  return value;
}

#ifdef WIN32
/*
 * For CreateProcess() calls we need to encode an argv into a command-line.
 *
 * See https://stackoverflow.com/questions/31838469/!
 */
static int quote_arg(char **cmdline, size_t *cmdlinesz, const char *arg) {
  size_t cmdlen = strlen(*cmdline);
  size_t arglen = strlen(arg);
  size_t nbackslashes = 0;
  size_t newsz;
  const char *p;
  char *tmp;

  if (strpbrk(arg, " \t\n\v\"") == NULL) {
    /* No quoting needed */
    if (cmdlen + arglen + 2 >= *cmdlinesz) {
      newsz = cmdlen + arglen + 128;
      if ((tmp = realloc(*cmdline, newsz)) == NULL)
        return ENOMEM;
      memset(tmp + cmdlen, 0, newsz - cmdlen);
      *cmdline = tmp;
      *cmdlinesz = newsz;
    }

    /* Add " ${arg}" */
    (*cmdline)[cmdlen] = ' ';
    memcpy((*cmdline) + cmdlen + 1, arg, arglen);
    return 0;
  }

  /* Quoting needed */
  if (cmdlen + 3 * arglen + 2 >= *cmdlinesz) {
    newsz = cmdlen + 3 * arglen + 128;
    if ((tmp = realloc(*cmdline, newsz)) == NULL)
      return ENOMEM;
    memset(tmp + cmdlen, 0, newsz - cmdlen);
    *cmdline = tmp;
    *cmdlinesz = newsz;
  }

  /* Add " \"" */
  (*cmdline)[cmdlen++] = ' ';
  (*cmdline)[cmdlen++] = '"';

  for (p = arg; *p; p++) {
    for (p++; *p && *p == '\\'; p++)
      nbackslashes++;
    if (*p == '"') {
      /* output 2 * nbackslashes */
      while (nbackslashes) {
        (*cmdline)[cmdlen++] = '\\';
        (*cmdline)[cmdlen++] = '\\';
      }
      /* output "\\\"" */
      (*cmdline)[cmdlen++] = '\\';
      (*cmdline)[cmdlen++] = '"';
    } else {
      /* output nbackslashes */
      while (nbackslashes)
        (*cmdline)[cmdlen++] = '\\';
      (*cmdline)[cmdlen++] = *p;
    }
    nbackslashes = 0;
  }

  while (nbackslashes) {
    /* arg ended in a sequence of backslashes */
    (*cmdline)[cmdlen++] = '\\';
    (*cmdline)[cmdlen++] = '\\';
  }

  (*cmdline)[cmdlen++] = '"';
  return 0;
}

static char *encode_argv(char **argv) {
  size_t cmdlinesz = 0;
  size_t i;
  char *cmdline = NULL;

  for (i = 0; argv[i]; i++) {
    if (quote_arg(&cmdline, &cmdlinesz, argv[i])) {
      free(cmdline);
      return NULL;
    }
  }
  return cmdline;
}

jv jq_spawn_process(jq_state *jq, jv file, jv file_actions, jv attrs, jv argv, jv env) {
  /*
   * XXX Finish
   *
   * Similar to the non-WIN32 version... but see
   *
   * https://github.com/rprichard/win32-console-docs
   *
   * Basically:
   *
   *  - encode the argv as one string
   *  - support only stdin/out/err redirections
   *  - open files or make pipes the usual way, except using _pipe()
   *  - get HANDLEs for FDs using _get_osfhandle()
   *  - setup STARTUPINFO struct with STARTF_USESTDHANDLES if we're redirecting I/O
   *  - call CreateProcess()
   *  - fdopen() our ends of pipes
   *  - ...
   */
}
#else

struct pipes {
  int pin[2];
  int pout[2];
  int perr[2];
};

static int jv2oflags(int *oflagsp, int fd, jv oflags, jv *res) {
  *oflagsp = 0;

  if (!jv_is_valid(oflags)) {
    if (fd == STDIN_FILENO)
      *oflagsp = O_RDONLY;
    else
      *oflagsp = O_WRONLY | O_CREAT;
    return 0;
  }
  if (jv_get_kind(oflags) != JV_KIND_ARRAY) {
    *res = jv_invalid_with_msg(jv_string("spawn file actions oflags must be array of O_* flag names"));
    jv_free(oflags);
    return EINVAL;
  }

  jv_array_foreach(oflags, i, oflag) {
    const char *s;

    if (jv_get_kind(oflag) != JV_KIND_STRING) {
      *res = jv_invalid_with_msg(jv_string("spawn file actions oflags must be array of O_* flag names"));
      jv_free(oflags);
      jv_free(oflag);
      return EINVAL;
    }
    s = jv_string_value(oflag);
    if      (strcmp(s, "O_RDONLY") == 0)
      *oflagsp |= O_RDONLY;
    else if (strcmp(s, "O_WRONLY") == 0)
      *oflagsp |= O_WRONLY;
    else if (strcmp(s, "O_RDWR")   == 0)
      *oflagsp |= O_RDWR;
    else if (strcmp(s, "O_CREAT")  == 0)
      *oflagsp |= O_CREAT;
    else if (strcmp(s, "O_EXCL")   == 0)
      *oflagsp |= O_EXCL;
    else if (strcmp(s, "O_APPEND") == 0)
      *oflagsp |= O_APPEND;
    else if (strcmp(s, "O_TRUNC")  == 0)
      *oflagsp |= O_TRUNC;
    else {
      jv_free(oflags);
      jv_free(oflag);
      *res = jv_invalid_with_msg(jv_string("spawn file actions oflags: only O_RDONLY, "
                                           "O_WRONLY, O_RDWR, O_CREAT, O_EXCL, O_APPEND, and "
                                           "O_TRUNC supported"));
      return EINVAL;
    }
  }
  jv_free(oflags);
  return 0;
}

static int jv2mode(mode_t *modep, jv mode, jv *res) {
  *modep = 0600;
  if (!jv_is_valid(mode))
    return 0;
  if (jv_get_kind(mode) != JV_KIND_NUMBER) {
    *res = jv_invalid_with_msg(jv_string("spawn file actions mode must be a 16-bit unsigned number"));
    jv_free(mode);
    return EINVAL;
  }
  *modep = jv_number_value(mode);
  if (jv_number_value(mode) != (double)*modep) {
    *res = jv_invalid_with_msg(jv_string("spawn file actions mode must be a 16-bit unsigned number"));
    jv_free(mode);
    return EINVAL;
  }
  jv_free(mode);
  return 0;
}

/*
 * This is an insane function :(
 *
 * The goal is to allow "file actions" of various types:
 *
 *  - {f: "stdin/out/err"}              --> setup a  pipe for stdin/out/err
 *  - {f: 0/1/2}                        --> setup a  pipe for stdin/out/err
 *  - {f: <number>, target: -1}         --> setup a  close action
 *  - {f: <number>, target: <number>}   --> setup a  dup2  action
 *  - {f: ..., target: <path>,
 *     oflags: [<oflag>], mode: <mode>} --> setup an open action
 *
 * <mode> has to be an array of O_* flag names
 *
 * XXX We really want something like posix_spawn_file_actions_addclosefrom(),
 * or something similar.  Might need to have an exec helper, perhaps jq itself
 * as an exec helper, to make it so.
 */
static int jv2pfac(jq_state *jq, jv fac, struct pipes *p, posix_spawn_file_actions_t *pfacs, jv *res) {
  static int devnull = -1;
  int fd = -1;

  if (devnull == -1 && (devnull = open("/dev/null", O_RDWR)) == -1) {
    jv_free(fac);
    return (*res = jv_invalid_with_msg(jv_string_fmt("could not open /dev/null: %s", strerror(errno)))),
           EINVAL;
  }
  if (jv_get_kind(fac) != JV_KIND_OBJECT) {
    jv_free(fac);
    return (*res = jv_invalid_with_msg(jv_string("spawn file action descriptors must be objects"))),
           EINVAL;
  }

  jv v = jv_object_get(jv_copy(fac), jv_string("f"));
  if (!jv_is_valid(v)) {
    jv_free(fac);
    return (*res = jv_invalid_with_msg(jv_string("spawn file action descriptors must have f field"))),
           EINVAL;
  }
  if (jv_get_kind(v) == JV_KIND_STRING) {
    if (!(strcmp(jv_string_value(v), "stdin")  == 0 && ((fd = STDIN_FILENO), 1)) &&
        !(strcmp(jv_string_value(v), "stdout") == 0 && (fd = STDOUT_FILENO)) &&
        !(strcmp(jv_string_value(v), "stderr") == 0 && (fd = STDERR_FILENO)) ) {
      jv_free(fac);
      jv_free(v);
      return (*res = jv_invalid_with_msg(jv_string("spawn file action descriptors must have f field with stdin/stdout/stderr or fd number"))),
             EINVAL;
    }
  } else if (jv_get_kind(v) == JV_KIND_NUMBER) {
    fd = jv_number_value(v);
    if (fd < 0) {
      jv_free(fac);
      jv_free(v);
      return (*res = jv_invalid_with_msg(jv_string("spawn file action descriptors must have f field with stdin/stdout/stderr or fd number"))),
             EINVAL;
    }
  } else {
    jv_free(fac);
    jv_free(v);
    return (*res = jv_invalid_with_msg(jv_string("spawn file action descriptors must have f field with stdin/stdout/stderr or fd number"))),
           EINVAL;
  }
  jv_free(v);

  v = jv_object_get(jv_copy(fac), jv_string("target"));
  if (jv_get_kind(v) == JV_KIND_NULL) {
    /* null -> /dev/null */
    if ((errno = posix_spawn_file_actions_adddup2(pfacs, devnull, fd))) {
      jv_free(fac);
      jv_free(v);
      return (*res = jv_invalid_with_msg(jv_string_fmt("could not set up spawn /dev/null fd: %s (%d)",
                                                      strerror(errno), errno))),
             errno;
    }
  } else if (jv_get_kind(v) == JV_KIND_NUMBER) {
    int target = jv_number_value(v);

    if (target < 0)
      errno = posix_spawn_file_actions_addclose(pfacs, fd);
    else
      errno = posix_spawn_file_actions_adddup2(pfacs, target, fd);
    if (errno) {
      jv_free(fac);
      jv_free(v);
      return (*res = jv_invalid_with_msg(jv_string_fmt("could not set up spawn pipe actions: %s (%d)",
                                                      strerror(errno), errno))),
             errno;
    }
  } else if (jv_get_kind(v) == JV_KIND_INVALID) {
    int *pp = NULL;
    /* No target -> pipe */
    if ((fd == STDIN_FILENO  && (pp = p->pin)  && p->pin[0]  != -1) ||
        (fd == STDOUT_FILENO && (pp = p->pout) && p->pout[0] != -1) ||
        (fd == STDERR_FILENO && (pp = p->perr) && p->perr[0] != -1)) {
      jv_free(fac);
      jv_free(v);
      return (*res = jv_invalid_with_msg(jv_string("duplicate spawn file action descriptors"))),
             EINVAL;
    }
    if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO) {
      jv_free(fac);
      jv_free(v);
      return (*res = jv_invalid_with_msg(jv_string("only stdin/out/err can have pipes"))),
             EINVAL;
    }
    if (pipe(pp) == -1) {
      jv_free(fac);
      jv_free(v);
      return (*res = jv_invalid_with_msg(jv_string_fmt("could not create pipe: %s (%d)",
                                                      strerror(errno), errno))),
             errno;
    }
    if (fd == STDIN_FILENO) {
      errno = posix_spawn_file_actions_adddup2(pfacs, p->pin[0], STDIN_FILENO);
      if (errno == 0)
        errno = posix_spawn_file_actions_addclose(pfacs, p->pin[0]);
      if (errno == 0)
        errno = posix_spawn_file_actions_addclose(pfacs, p->pin[1]);
      if (errno) {
        jv_free(fac);
        jv_free(v);
        return (*res = jv_invalid_with_msg(jv_string_fmt("could not set up spawn pipe actions: %s (%d)",
                                                        strerror(errno), errno))),
               errno;
      }
    } else if (fd == STDOUT_FILENO) {
      errno = posix_spawn_file_actions_adddup2(pfacs, p->pout[1], STDOUT_FILENO);
      if (errno == 0)
        errno = posix_spawn_file_actions_addclose(pfacs, p->pout[0]);
      if (errno == 0)
        errno = posix_spawn_file_actions_addclose(pfacs, p->pout[1]);
      if (errno) {
        jv_free(fac);
        jv_free(v);
        return (*res = jv_invalid_with_msg(jv_string_fmt("could not set up spawn pipe actions: %s (%d)",
                                                        strerror(errno), errno))),
               errno;
      }
    } else if (fd == STDERR_FILENO) {
      errno = posix_spawn_file_actions_adddup2(pfacs, p->perr[1], STDERR_FILENO);
      if (errno == 0)
        errno = posix_spawn_file_actions_addclose(pfacs, p->perr[0]);
      if (errno == 0)
        errno = posix_spawn_file_actions_addclose(pfacs, p->perr[1]);
      if (errno) {
        jv_free(fac);
        jv_free(v);
        return (*res = jv_invalid_with_msg(jv_string_fmt("could not set up spawn pipe actions: %s (%d)",
                                                        strerror(errno), errno))),
               errno;
      }
    }
  } else if (jv_get_kind(v) == JV_KIND_STRING) {
    mode_t mode;
    int oflags;

    if (jv2oflags(&oflags, fd, jv_object_get(jv_copy(fac), jv_string("oflags")), res) ||
        jv2mode(&mode, jv_object_get(jv_copy(fac), jv_string("mode")), res)) {
      jv_free(fac);
      jv_free(v);
      return EINVAL;
    }

    *res = jq_io_policy_check(jq, JV_OBJECT(jv_string("kind"), jv_string("fileaction"),
                                           jv_string("action"), jv_copy(fac)));
    if (jv_get_kind(*res) != JV_KIND_TRUE) {
      jv_free(fac);
      jv_free(v);
      return EACCES;
    }

    if (fd == STDIN_FILENO)
      oflags = O_RDONLY;
    else if ((fd == STDOUT_FILENO || fd == STDERR_FILENO) && !(oflags & O_WRONLY)) {
      jv_free(fac);
      jv_free(v);
      return (*res = jv_invalid_with_msg(jv_string("invalid oflags for stdout or stderr"))),
             EINVAL;
    }

    errno = posix_spawn_file_actions_addopen(pfacs, fd, jv_string_value(v), oflags, mode);
    if (errno) {
      jv_free(fac);
      jv_free(v);
      return (*res = jv_invalid_with_msg(jv_string_fmt("could not set up spawn pipe actions: %s (%d)",
                                                      strerror(errno), errno))),
             errno;
    }
  } else {
    jv_free(fac);
    jv_free(v);
    return (*res = jv_invalid_with_msg(jv_string("spawn file action descriptors can only have target field with path string or fd number"))),
           EINVAL;
  }

  *res = jv_true();
  jv_free(fac);
  jv_free(v);
  return 0;
}

static int jv2attrflags(short *flagsp, jv flags, jv *res) {
  *flagsp = 0;

  if (jv_get_kind(flags) != JV_KIND_ARRAY) {
    *res = jv_invalid_with_msg(jv_string("spawn attribute flags must be array of flag names"));
    jv_free(flags);
    return EINVAL;
  }
  jv_array_foreach(flags, i, flag) {
    if (jv_get_kind(flag) != JV_KIND_STRING) {
      *res = jv_invalid_with_msg(jv_string("spawn attribute flags must be array of flag names"));
      jv_free(flags);
      return EINVAL;
    }
    const char *s = jv_string_value(flag);
    if      (strcmp(s, "RESETIDS") == 0) *flagsp |= POSIX_SPAWN_RESETIDS;
    else if (strcmp(s, "SETPGROUP") == 0) *flagsp |= POSIX_SPAWN_SETPGROUP;
    else if (strcmp(s, "SETSIGDEF") == 0) *flagsp |= POSIX_SPAWN_SETSIGDEF;
    else if (strcmp(s, "SETSIGMASK") == 0) *flagsp |= POSIX_SPAWN_SETSIGMASK;
    else {
      *res = jv_invalid_with_msg(jv_string_fmt("spawn attribute flag %s not supported", s));
      jv_free(flags);
      jv_free(flag);
      return EINVAL;
    }
    jv_free(flag);
  }
  jv_free(flags);
  return 0;
}

static int jv2sigset(sigset_t *ssp, jv ss, jv *res) {
  (void) sigemptyset(ssp);

  if (jv_get_kind(ss) != JV_KIND_ARRAY) {
    *res = jv_invalid_with_msg(jv_string("spawn attribute sigset must be array of signal names"));
    jv_free(ss);
    return EINVAL;
  }
  jv_array_foreach(ss, i, signame) {
    if (jv_get_kind(signame) != JV_KIND_STRING) {
      *res = jv_invalid_with_msg(jv_string("spawn attribute sigset must be array of signal names"));
      jv_free(ss);
      return EINVAL;
    }
    const char *s = jv_string_value(signame);
#define sig1(name) \
    do { if (strcmp(s, #name) == 0) { sigaddset(ssp, name); jv_free(signame); continue; } } while (0)
    sig1(SIGHUP);
    sig1(SIGINT);
    sig1(SIGQUIT);
    sig1(SIGILL);
    sig1(SIGTRAP);
#ifdef SIGIOT
    sig1(SIGIOT);
#endif
    sig1(SIGBUS);
    sig1(SIGFPE);
    sig1(SIGKILL);
    sig1(SIGUSR1);
    sig1(SIGSEGV);
    sig1(SIGUSR2);
    sig1(SIGPIPE);
    sig1(SIGALRM);
    sig1(SIGTERM);
#ifdef SIGSTKFLT
    sig1(SIGSTKFLT);
#endif
    sig1(SIGCHLD);
    sig1(SIGCONT);
    sig1(SIGSTOP);
    sig1(SIGTSTP);
    sig1(SIGTTIN);
    sig1(SIGTTOU);
    sig1(SIGURG);
#ifdef SIGSTKFLT
    sig1(SIGXCPU);
#endif
#ifdef SIGXFSZ
    sig1(SIGXFSZ);
#endif
#ifdef SIGVTALRM
    sig1(SIGVTALRM);
#endif
#ifdef SIGPROF
    sig1(SIGPROF);
#endif
#ifdef SIGWINCH
    sig1(SIGWINCH);
#endif
#ifdef SIGPOLL
    sig1(SIGPOLL);
#endif
#ifdef SIGPWR
    sig1(SIGPWR);
#endif
#ifdef SIGSYS
    sig1(SIGSYS);
#endif
    *res = jv_invalid_with_msg(jv_string_fmt("spawn attribute signal %s not supported", s));
    jv_free(signame);
    jv_free(ss);
    return EINVAL;
  }
  jv_free(ss);
  return 0;
}

static int jv2pattr(jv attr, posix_spawnattr_t *pattrs, jv *res) {
  int ret;

  if (jv_get_kind(attr) != JV_KIND_OBJECT) {
    *res = jv_invalid_with_msg(jv_string("spawn attributes must be objects"));
    jv_free(attr);
    return EINVAL;
  }
  jv v = jv_object_get(jv_copy(attr), jv_string("value"));
  attr = jv_object_get(attr, jv_string("attr"));
  
  if (jv_get_kind(attr) != JV_KIND_STRING) {
    *res = jv_invalid_with_msg(jv_string("spawn attributes names must be strings"));
    jv_free(attr);
    return EINVAL;
  }

  const char *s = jv_string_value(attr);
  if (strcmp(s, "flags") == 0) {
    short flags;

    if (jv2attrflags(&flags, v, res)) {
      jv_free(attr);
      return EINVAL;
    }
    if ((ret = posix_spawnattr_setflags(pattrs, flags))) {
      *res = jv_invalid_with_msg(jv_string_fmt("could not set spawn flags: %s", strerror(ret)));
      jv_free(attr);
      return ret;
    }
  } else if (strcmp(s, "pgroup") == 0) {
    if (jv_get_kind(v) != JV_KIND_NUMBER) {
      *res = jv_invalid_with_msg(jv_string("spawn pgroup attribute must be numeric"));
      jv_free(attr);
      return EINVAL;
    }
    if ((ret = posix_spawnattr_setpgroup(pattrs, jv_number_value(v)))) {
      *res = jv_invalid_with_msg(jv_string_fmt("could not set spawn pgroup: %s", strerror(ret)));
      jv_free(v);
      jv_free(attr);
      return ret;
    }
    jv_free(v);
  } else if (strcmp(s, "sigdefault") == 0 || strcmp(s, "sigmask") == 0) {
    sigset_t ss;

    if (!jv2sigset(&ss, v, res)) {
      jv_free(attr);
      return EINVAL;
    }
    if (strcmp(s, "sigdefault") == 0 && (ret = posix_spawnattr_setsigdefault(pattrs, &ss))) {
      *res = jv_invalid_with_msg(jv_string_fmt("could not set spawn sigdefault: %s", strerror(ret)));
      jv_free(attr);
      return ret;
    } else if (strcmp(s, "sigmask") == 0 && (ret = posix_spawnattr_setsigmask(pattrs, &ss))) {
      *res = jv_invalid_with_msg(jv_string_fmt("could not set spawn sigmask: %s", strerror(ret)));
      jv_free(attr);
      return ret;
    }
  } else {
    *res = jv_invalid_with_msg(jv_string_fmt("spawn attribute %s not supported", s));
    jv_free(attr);
    return EINVAL;
  }

  return 0;
}

static const char **jv2charvec(jv vec, jv *res) {
  const char **charvec;
  size_t n = 0;

  if ((charvec = calloc(jv_array_length(vec) + 1, sizeof(charvec[0])))) {
    jv_array_foreach(vec, i, v) {
      if (jv_get_kind(v) != JV_KIND_STRING) {
        jv_free(v);
        free(charvec);
        *res = jv_invalid_with_msg(jv_string("argument vector must be an array of strings"));
        return NULL;
      }
      charvec[i] = jv_string_value(v);
      jv_free(v); // we still have a reference via vec
      n++;
    }
    jv_free(vec);
    charvec[n] = NULL;
  }
  return charvec;
}

#ifndef WIN32
#  ifdef __APPLE__
#    include <crt_externs.h>
#    define environ (*_NSGetEnviron())
#  else
     extern char ** environ;
#  endif
#endif

jv jq_spawn_process(jq_state *jq, jv file, jv file_actions, jv attrs, jv argv, jv env) {
  posix_spawn_file_actions_t pfacs;
  posix_spawnattr_t pattrs;
  struct pipes p;
  const char **argvp = NULL;
  const char **envp = NULL;
  pid_t pid;
  int ret;
  jv res = jv_invalid();

  p.pin[0] = p.pin[1] = p.pout[0] = p.pout[1] = p.perr[0] = p.perr[1] = -1;

  if ((ret = posix_spawn_file_actions_init(&pfacs))) {
    res = jv_invalid_with_msg(jv_string_fmt("Could not spawn process: %s", strerror(ret)));
    goto out3;
  }
  if ((ret = posix_spawnattr_init(&pattrs))) {
    res = jv_invalid_with_msg(jv_string_fmt("Could not spawn process: %s", strerror(ret)));
    goto out2;
  }

  if (jv_get_kind(file) != JV_KIND_STRING) {
    res = jv_invalid_with_msg(jv_string("spawn file must be a string"));
    goto out;
  }
  if (jv_get_kind(argv) != JV_KIND_ARRAY) {
    res = jv_invalid_with_msg(jv_string("argument vector must be an array of strings"));
    goto out;
  }
  res = jq_io_policy_check(jq, JV_OBJECT(jv_string("kind"), jv_string("spawn"),
                                         jv_string("file"), jv_copy(file),
                                         jv_string("file_actions"), jv_copy(file_actions),
                                         jv_string("attributes"), jv_copy(attrs),
                                         jv_string("argv"), jv_copy(argv),
                                         jv_string("env"), jv_copy(env)));
  if (jv_get_kind(res) != JV_KIND_TRUE)
    goto out;
  if ((argvp = jv2charvec(jv_copy(argv), &res)) == NULL)
    goto out;
  if (jv_get_kind(env) != JV_KIND_NULL && jv_get_kind(env) != JV_KIND_ARRAY) {
    res = jv_invalid_with_msg(jv_string("environment vector must be an array of strings"));
    goto out;
  }
  if (jv_get_kind(env) == JV_KIND_ARRAY && (envp = jv2charvec(jv_copy(env), &res)) == NULL)
    goto out;

  if (jv_get_kind(file_actions) == JV_KIND_ARRAY) {
    jv_array_foreach(file_actions, i, v) {
      if (jv2pfac(jq, v, &p, &pfacs, &res))
        goto out;
    }
  }

  if (jv_get_kind(attrs) == JV_KIND_ARRAY) {
    jv_array_foreach(attrs, i, v) {
      if (jv2pattr(jv_copy(v), &pattrs, &res))
        goto out;
    }
  }

  if ((ret = posix_spawnp(&pid, jv_string_value(file), &pfacs, &pattrs,
                          (char * const *)argvp,
                          envp ? (char * const *)envp : environ))) {
    res = jv_invalid_with_msg(jv_string_fmt("Could not spawn process: %s", strerror(ret)));
    goto out;
  }

  res = JV_OBJECT(jv_string("pid"), jv_number(pid),
                  jv_string("ppid"), jv_number(getpid()),
                  jv_string("pgroup"), jv_number(getpgid(pid)));
  if (p.pin[0] != -1)
    res = jv_object_set(res, jv_string("stdin"), JV_ARRAY(jv_number(p.pin[0]), jv_number(p.pin[1])));
  if (p.pout[0] != -1)
    res = jv_object_set(res, jv_string("stdout"), JV_ARRAY(jv_number(p.pout[0]), jv_number(p.pout[1])));
  if (p.perr[0] != -1)
    res = jv_object_set(res, jv_string("stderr"), JV_ARRAY(jv_number(p.perr[0]), jv_number(p.perr[1])));

out:
out2:
out3:
  jv_free(file_actions);
  jv_free(attrs);
  jv_free(argv);
  jv_free(env);
  free(argvp);
  free(envp);
  if (jv_get_kind(res) == JV_KIND_INVALID) {
    (void) close(p.pin[0]);
    (void) close(p.pin[1]);
    (void) close(p.pout[0]);
    (void) close(p.pout[1]);
    (void) close(p.perr[0]);
    (void) close(p.perr[1]);
  }
  return res;

}
#endif
