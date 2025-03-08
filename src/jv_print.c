#include <assert.h>
#include <stdio.h>
#include <float.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include <io.h>
#include <fileapi.h>
#endif

#include "jv.h"
#include "jv_dtoa.h"
#include "jv_dtoa_tsd.h"
#include "jv_unicode.h"
#include "jv_alloc.h"
#include "jv_private.h"

#ifndef MAX_PRINT_DEPTH
#define MAX_PRINT_DEPTH (256)
#endif

#define ESC "\033"
#define COL(c) (ESC "[" c "m")
#define COLRESET (ESC "[0m")

// Color table. See https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
// for how to choose these. The order is same as jv_kind definition, and
// the last color is used for object keys.
#define DEFAULT_COLORS \
  {COL("0;90"),    COL("0;39"),      COL("0;39"),     COL("0;39"),\
   COL("0;32"),    COL("1;39"),      COL("1;39"),     COL("1;34")};
static const char *const default_colors[] = DEFAULT_COLORS;
static const char *colors[] = DEFAULT_COLORS;
#define COLORS_LEN (sizeof(colors) / sizeof(colors[0]))
#define FIELD_COLOR (colors[7])

static char *colors_buf = NULL;
int jq_set_colors(const char *code_str) {
  if (code_str == NULL)
    return 1;

  // the start of each color code in the env var, and the byte after the end of the last one
  const char *codes[COLORS_LEN + 1];
  size_t num_colors;
  // must be initialized before `goto default_colors`, used later to loop over every color
  size_t ci = 0;

  for (num_colors = 0;; num_colors++) {
    codes[num_colors] = code_str;
    code_str += strspn(code_str, "0123456789;");
    if (code_str[0] == '\0' || num_colors + 1 >= COLORS_LEN) {
      break;
    } else if (code_str[0] != ':') {
      return 0; // invalid character
    }
    code_str++;
  }
  if (codes[num_colors] != code_str) {
    // count the last color and store its end (plus one byte for consistency with starts)
    // an empty last color would be ignored (for cases like "" and "0:")
    num_colors++;
    codes[num_colors] = code_str + 1;
  } else if (num_colors == 0) {
    if (colors_buf != NULL) {
      jv_mem_free(colors_buf);
      colors_buf = NULL;
    }
    goto default_colors;
  }

  colors_buf = jv_mem_realloc(
    colors_buf,
    // add ESC '[' 'm' to each string
    // '\0' is already included in difference of codes
    codes[num_colors] - codes[0] + 3 * num_colors
  );
  char *cb = colors_buf;
  for (; ci < num_colors; ci++) {
    colors[ci] = cb;
    size_t len = codes[ci + 1] - 1 - codes[ci];

    cb[0] = ESC[0];
    cb[1] = '[';
    memcpy(cb + 2, codes[ci], len);
    cb[2 + len] = 'm';
    cb[3 + len] = '\0';

    cb += len + 4;
  }
  default_colors:
  for (; ci < COLORS_LEN; ci++)
    colors[ci] = default_colors[ci];
  return 1;
}

static void put_buf(const char *s, int len, FILE *fout, jv *strout, int is_tty) {
  if (strout) {
    *strout = jv_string_append_buf(*strout, s, len);
  } else {
#ifdef WIN32
  /* See util.h */
  if (is_tty) {
    wchar_t *ws;
    size_t wl;
    if (len == -1)
      len = strlen(s);
    wl = MultiByteToWideChar(CP_UTF8, 0, s, len, NULL, 0);
    ws = jv_mem_calloc(wl + 1, sizeof(*ws));
    if (!ws)
      return;
    wl = MultiByteToWideChar(CP_UTF8, 0, s, len, ws, wl + 1);
    ws[wl] = 0;
    WriteConsoleW((HANDLE)_get_osfhandle(fileno(fout)), ws, wl, NULL, NULL);
    free(ws);
  } else
    fwrite(s, 1, len, fout);
#else
  fwrite(s, 1, len, fout);
#endif
  }
}

static void put_char(char c, FILE* fout, jv* strout, int T) {
  put_buf(&c, 1, fout, strout, T);
}

static void put_str(const char* s, FILE* fout, jv* strout, int T) {
  put_buf(s, strlen(s), fout, strout, T);
}

static void put_indent(int n, int flags, FILE* fout, jv* strout, int T) {
  if (flags & JV_PRINT_TAB) {
    while (n--)
      put_char('\t', fout, strout, T);
  } else {
    n *= ((flags & (JV_PRINT_SPACE0 | JV_PRINT_SPACE1 | JV_PRINT_SPACE2)) >> 8);
    while (n--)
      put_char(' ', fout, strout, T);
  }
}

static void jvp_dump_string(jv str, int ascii_only, FILE* F, jv* S, int T) {
  assert(jv_get_kind(str) == JV_KIND_STRING);
  const char* i = jv_string_value(str);
  const char* end = i + jv_string_length_bytes(jv_copy(str));
  const char* cstart;
  int c = 0;
  char buf[32];
  put_char('"', F, S, T);
  while ((i = jvp_utf8_next((cstart = i), end, &c))) {
    assert(c != -1);
    int unicode_escape = 0;
    if (0x20 <= c && c <= 0x7E) {
      // printable ASCII
      if (c == '"' || c == '\\') {
        put_char('\\', F, S, T);
      }
      put_char(c, F, S, T);
    } else if (c < 0x20 || c == 0x7F) {
      // ASCII control character
      switch (c) {
      case '\b':
        put_char('\\', F, S, T);
        put_char('b', F, S, T);
        break;
      case '\t':
        put_char('\\', F, S, T);
        put_char('t', F, S, T);
        break;
      case '\r':
        put_char('\\', F, S, T);
        put_char('r', F, S, T);
        break;
      case '\n':
        put_char('\\', F, S, T);
        put_char('n', F, S, T);
        break;
      case '\f':
        put_char('\\', F, S, T);
        put_char('f', F, S, T);
        break;
      default:
        unicode_escape = 1;
        break;
      }
    } else {
      if (ascii_only) {
        unicode_escape = 1;
      } else {
        put_buf(cstart, i - cstart, F, S, T);
      }
    }
    if (unicode_escape) {
      if (c <= 0xffff) {
        snprintf(buf, sizeof(buf), "\\u%04x", c);
      } else {
        c -= 0x10000;
        snprintf(buf, sizeof(buf), "\\u%04x\\u%04x",
                0xD800 | ((c & 0xffc00) >> 10),
                0xDC00 | (c & 0x003ff));
      }
      put_str(buf, F, S, T);
    }
  }
  assert(c != -1);
  put_char('"', F, S, T);
}

static void put_refcnt(struct dtoa_context* C, int refcnt, FILE *F, jv* S, int T){
  char buf[JVP_DTOA_FMT_MAX_LEN];
  put_char(' ', F, S, T);
  put_char('(', F, S, T);
  put_str(jvp_dtoa_fmt(C, buf, refcnt), F, S, T);
  put_char(')', F, S, T);
}

static void jv_dump_term(struct dtoa_context* C, jv x, int flags, int indent, FILE* F, jv* S) {
  char buf[JVP_DTOA_FMT_MAX_LEN];
  const char* color = 0;
  double refcnt = (flags & JV_PRINT_REFCOUNT) ? jv_get_refcnt(x) - 1 : -1;
  if ((flags & JV_PRINT_COLOR) && jv_get_kind(x) != JV_KIND_INVALID) {
    color = colors[(int)jv_get_kind(x) - 1];
    put_str(color, F, S, flags & JV_PRINT_ISATTY);
  }
  if (indent > MAX_PRINT_DEPTH) {
    put_str("<skipped: too deep>", F, S, flags & JV_PRINT_ISATTY);
  } else switch (jv_get_kind(x)) {
  default:
  case JV_KIND_INVALID:
    if (flags & JV_PRINT_INVALID) {
      jv msg = jv_invalid_get_msg(jv_copy(x));
      if (jv_get_kind(msg) == JV_KIND_STRING) {
        put_str("<invalid:", F, S, flags & JV_PRINT_ISATTY);
        jvp_dump_string(msg, flags | JV_PRINT_ASCII, F, S, flags & JV_PRINT_ISATTY);
        put_char('>', F, S, flags & JV_PRINT_ISATTY);
      } else {
        put_str("<invalid>", F, S, flags & JV_PRINT_ISATTY);
      }
    } else {
      assert(0 && "Invalid value");
    }
    break;
  case JV_KIND_NULL:
    put_str("null", F, S, flags & JV_PRINT_ISATTY);
    break;
  case JV_KIND_FALSE:
    put_str("false", F, S, flags & JV_PRINT_ISATTY);
    break;
  case JV_KIND_TRUE:
    put_str("true", F, S, flags & JV_PRINT_ISATTY);
    break;
  case JV_KIND_NUMBER: {
    if (jvp_number_is_nan(x)) {
      jv_dump_term(C, jv_null(), flags, indent, F, S);
    } else {
#ifdef USE_DECNUM
      const char * literal_data = jv_number_get_literal(x);
      if (literal_data) {
        put_str(literal_data, F, S, flags & JV_PRINT_ISATTY);
      } else {
#endif
        double d = jv_number_value(x);
        if (d != d) {
          // JSON doesn't have NaN, so we'll render it as "null"
          put_str("null", F, S, flags & JV_PRINT_ISATTY);
        } else {
          // Normalise infinities to something we can print in valid JSON
          if (d > DBL_MAX) d = DBL_MAX;
          if (d < -DBL_MAX) d = -DBL_MAX;
          put_str(jvp_dtoa_fmt(C, buf, d), F, S, flags & JV_PRINT_ISATTY);
        }
      }
#ifdef USE_DECNUM
    }
#endif
    break;
  }
  case JV_KIND_STRING:
    jvp_dump_string(x, flags & JV_PRINT_ASCII, F, S, flags & JV_PRINT_ISATTY);
    if (flags & JV_PRINT_REFCOUNT)
      put_refcnt(C, refcnt, F, S, flags & JV_PRINT_ISATTY);
    break;
  case JV_KIND_ARRAY: {
    if (jv_array_length(jv_copy(x)) == 0) {
      put_str("[]", F, S, flags & JV_PRINT_ISATTY);
      break;
    }
    put_char('[', F, S, flags & JV_PRINT_ISATTY);
    jv_array_foreach(x, i, elem) {
      if (i!=0) {
        if (color) put_str(color, F, S, flags & JV_PRINT_ISATTY);
        put_char(',', F, S, flags & JV_PRINT_ISATTY);
      }
      if (color) put_str(COLRESET, F, S, flags & JV_PRINT_ISATTY);
      if (flags & JV_PRINT_PRETTY) {
        put_char('\n', F, S, flags & JV_PRINT_ISATTY);
        put_indent(indent + 1, flags, F, S, flags & JV_PRINT_ISATTY);
      }
      jv_dump_term(C, elem, flags, indent + 1, F, S);
    }
    if (flags & JV_PRINT_PRETTY) {
      put_char('\n', F, S, flags & JV_PRINT_ISATTY);
      put_indent(indent, flags, F, S, flags & JV_PRINT_ISATTY);
    }
    if (color) put_str(color, F, S, flags & JV_PRINT_ISATTY);
    put_char(']', F, S, flags & JV_PRINT_ISATTY);
    if (flags & JV_PRINT_REFCOUNT)
      put_refcnt(C, refcnt, F, S, flags & JV_PRINT_ISATTY);
    break;
  }
  case JV_KIND_OBJECT: {
    if (jv_object_length(jv_copy(x)) == 0) {
      put_str("{}", F, S, flags & JV_PRINT_ISATTY);
      break;
    }
    put_char('{', F, S, flags & JV_PRINT_ISATTY);
    int first = 1;
    int i = 0;
    jv keyset = jv_null();
    while (1) {
      jv key, value;
      if (flags & JV_PRINT_SORTED) {
        if (first) {
          keyset = jv_keys(jv_copy(x));
          i = 0;
        } else {
          i++;
        }
        if (i >= jv_array_length(jv_copy(keyset))) {
          jv_free(keyset);
          break;
        }
        key = jv_array_get(jv_copy(keyset), i);
        value = jv_object_get(jv_copy(x), jv_copy(key));
      } else {
        if (first) {
          i = jv_object_iter(x);
        } else {
          i = jv_object_iter_next(x, i);
        }
        if (!jv_object_iter_valid(x, i)) break;
        key = jv_object_iter_key(x, i);
        value = jv_object_iter_value(x, i);
      }

      if (!first) {
        if (color) put_str(color, F, S, flags & JV_PRINT_ISATTY);
        put_char(',', F, S, flags & JV_PRINT_ISATTY);
      }
      if (color) put_str(COLRESET, F, S, flags & JV_PRINT_ISATTY);
      if (flags & JV_PRINT_PRETTY) {
        put_char('\n', F, S, flags & JV_PRINT_ISATTY);
        put_indent(indent + 1, flags, F, S, flags & JV_PRINT_ISATTY);
      }

      first = 0;
      if (color) put_str(FIELD_COLOR, F, S, flags & JV_PRINT_ISATTY);
      jvp_dump_string(key, flags & JV_PRINT_ASCII, F, S, flags & JV_PRINT_ISATTY);
      jv_free(key);
      if (color) put_str(COLRESET, F, S, flags & JV_PRINT_ISATTY);

      if (color) put_str(color, F, S, flags & JV_PRINT_ISATTY);
      put_char(':', F, S, flags & JV_PRINT_ISATTY);
      if (color) put_str(COLRESET, F, S, flags & JV_PRINT_ISATTY);
      if (flags & JV_PRINT_PRETTY) {
        put_char(' ', F, S, flags & JV_PRINT_ISATTY);
      }

      jv_dump_term(C, value, flags, indent + 1, F, S);
    }
    if (flags & JV_PRINT_PRETTY) {
      put_char('\n', F, S, flags & JV_PRINT_ISATTY);
      put_indent(indent, flags, F, S, flags & JV_PRINT_ISATTY);
    }
    if (color) put_str(color, F, S, flags & JV_PRINT_ISATTY);
    put_char('}', F, S, flags & JV_PRINT_ISATTY);
    if (flags & JV_PRINT_REFCOUNT)
      put_refcnt(C, refcnt, F, S, flags & JV_PRINT_ISATTY);
  }
  }
  jv_free(x);
  if (color) {
    put_str(COLRESET, F, S, flags & JV_PRINT_ISATTY);
  }
}

void jv_dumpf(jv x, FILE *f, int flags) {
  jv_dump_term(tsd_dtoa_context_get(), x, flags, 0, f, 0);
}

void jv_dump(jv x, int flags) {
  jv_dumpf(x, stdout, flags);
}

/* This one is nice for use in debuggers */
void jv_show(jv x, int flags) {
  if (flags == -1)
    flags = JV_PRINT_PRETTY | JV_PRINT_COLOR | JV_PRINT_INDENT_FLAGS(2);
  jv_dumpf(jv_copy(x), stderr, flags | JV_PRINT_INVALID);
  fflush(stderr);
}

jv jv_dump_string(jv x, int flags) {
  jv s = jv_string("");
  jv_dump_term(tsd_dtoa_context_get(), x, flags, 0, 0, &s);
  return s;
}

char *jv_dump_string_trunc(jv x, char *outbuf, size_t bufsize) {
  x = jv_dump_string(x, 0);
  const char *str = jv_string_value(x);
  const size_t len = strlen(str);
  strncpy(outbuf, str, bufsize);
  if (len > bufsize - 1 && bufsize >= 4) {
    // Indicate truncation with '...' without breaking UTF-8.
    const char *s = jvp_utf8_backtrack(outbuf + bufsize - 4, outbuf, NULL);
    if (s) bufsize = s + 4 - outbuf;
    strcpy(outbuf + bufsize - 4, "...");
  } else {
    outbuf[bufsize - 1] = '\0';
  }
  jv_free(x);
  return outbuf;
}
