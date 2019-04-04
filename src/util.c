
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
