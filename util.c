
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
#include <stdlib.h>
#include <unistd.h>
#ifndef WIN32
#include <pwd.h>
#endif


#include "util.h"
#include "jq.h"
#include "jv_alloc.h"

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
     buf = malloc(sizeof(char) * path_max);
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
  jq_msg_cb err_cb;
  void *err_cb_data;
  jv_parser *parser;
  FILE* current_input;
  jv files;
  int open_failures;
  jv slurped;
  char buf[4096];
};

static void fprinter(void *data, jv fname) {
  fprintf((FILE *)data, "jq: error: Could not open file %s: %s\n", jv_string_value(fname), strerror(errno));
  jv_free(fname);
}

// If parser == NULL -> RAW
jq_util_input_state jq_util_input_init(jq_msg_cb err_cb, void *err_cb_data) {
  if (err_cb == NULL) {
    err_cb = fprinter;
    err_cb_data = stderr;
  }
  jq_util_input_state new_state = jv_mem_alloc(sizeof(*new_state));
  memset(new_state, 0, sizeof(*new_state));
  new_state->err_cb = err_cb;
  new_state->err_cb_data = err_cb_data;
  new_state->parser = NULL;
  new_state->current_input = NULL;
  new_state->files = jv_array();
  new_state->slurped = jv_invalid();
  new_state->buf[0] = 0;

  return new_state;
}

void jq_util_input_set_parser(jq_util_input_state state, jv_parser *parser, int slurp) {
  assert(!jv_is_valid(state->slurped));
  state->parser = parser;

  if (parser == NULL && slurp)
    state->slurped = jv_string("");
  else if (slurp)
    state->slurped = jv_array();
  else
    state->slurped = jv_invalid();
}

void jq_util_input_free(jq_util_input_state *state) {
  jq_util_input_state old_state = *state;
  *state = NULL;
  if (old_state == NULL)
    return;

  if (old_state->parser != NULL)
    jv_parser_free(old_state->parser);
  jv_free(old_state->files);
  jv_free(old_state->slurped);
  jv_mem_free(old_state);
}

void jq_util_input_add_input(jq_util_input_state state, jv fname) {
  state->files = jv_array_append(state->files, fname);
}

int jq_util_input_open_errors(jq_util_input_state state) {
  return state->open_failures;
}

static jv next_file(jq_util_input_state state) {
  jv next = jv_array_get(jv_copy(state->files), 0);
  if (jv_array_length(jv_copy(state->files)) > 0)
    state->files = jv_array_slice(state->files, 1, jv_array_length(jv_copy(state->files)));
  return next;
}

int jq_util_input_read_more(jq_util_input_state state) {
  if (!state->current_input || feof(state->current_input) || ferror(state->current_input)) {
    if (state->current_input && ferror(state->current_input)) {
      // System-level input error on the stream. It will be closed (below).
      // TODO: report it. Can't use 'state->err_cb()' as it is hard-coded for
      //       'open' related problems.
      fprintf(stderr,"Input error: %s\n", strerror(errno));
    }
    if (state->current_input) {
      if (state->current_input == stdin) {
        clearerr(stdin); // perhaps we can read again; anyways, we don't fclose(stdin)
      } else {
        fclose(state->current_input);
      }
      state->current_input = NULL;
    }
    jv f = next_file(state);
    if (jv_is_valid(f)) {
      if (!strcmp(jv_string_value(f), "-")) {
        state->current_input = stdin;
      } else {
        state->current_input = fopen(jv_string_value(f), "r");
        if (!state->current_input) {
          state->err_cb(state->err_cb_data, jv_copy(f));
          state->open_failures++;
        }
      }
      jv_free(f);
    }
  }

  state->buf[0] = 0;
  if (state->current_input) {
    if (!fgets(state->buf, sizeof(state->buf), state->current_input))
      state->buf[0] = 0;
  }
  return jv_array_length(jv_copy(state->files)) == 0 && (!state->current_input || feof(state->current_input));
}

jv jq_util_input_next_input_cb(jq_state *jq, void *data) {
  return jq_util_input_next_input((jq_util_input_state)data);
}

// Blocks to read one more input from stdin and/or given files
// When slurping, it returns just one value
jv jq_util_input_next_input(jq_util_input_state state) {
  int is_last = 0;
  jv value = jv_invalid(); // need more input
  do {
    if (state->parser == NULL) {
      // Raw input
      is_last = jq_util_input_read_more(state);
      if (state->buf[0] == '\0')
        continue;
      int len = strlen(state->buf); // Raw input doesn't support NULs
      if (len > 0) {
        if (jv_is_valid(state->slurped)) {
          // Slurped raw input
          state->slurped = jv_string_concat(state->slurped, jv_string(state->buf));
        } else {
          if (!jv_is_valid(value))
            value = jv_string("");
          if (state->buf[len-1] == '\n') {
            // whole line
            state->buf[len-1] = 0;
            return jv_string_concat(value, jv_string(state->buf));
          }
          value = jv_string_concat(value, jv_string(state->buf));
          state->buf[0] = '\0';
        }
      }
    } else {
      if (jv_parser_remaining(state->parser) == 0) {
        is_last = jq_util_input_read_more(state);
        jv_parser_set_buf(state->parser, state->buf, strlen(state->buf), !is_last);
      }
      value = jv_parser_next(state->parser);
      if (jv_is_valid(state->slurped)) {
        if (jv_is_valid(value)) {
          state->slurped = jv_array_append(state->slurped, value);
          value = jv_invalid();
        } else if (jv_invalid_has_msg(jv_copy(value)))
          return value; // Not slurped parsed input
      } else if (jv_is_valid(value) || jv_invalid_has_msg(jv_copy(value))) {
        return value;
      }
    }
  } while (!is_last);

  if (jv_is_valid(state->slurped)) {
    value = state->slurped;
    state->slurped = jv_invalid();
  }
  return value;
}
