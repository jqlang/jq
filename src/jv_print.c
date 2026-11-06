#include <assert.h>
#include <stdio.h>
#include <float.h>
#include <string.h>
#include <stdint.h>

#include "jv.h"
#include "jv_dtoa.h"
#include "jv_dtoa_tsd.h"
#include "jv_unicode.h"
#include "jv_alloc.h"
#include "jv_private.h"
#include "util.h"

#ifndef MAX_PRINT_DEPTH
#define MAX_PRINT_DEPTH (10000)
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

/*
 * Output is collected in a buffer and handed to stdio (or appended to the
 * output string) in pieces, rather than one call per token: printing emits
 * many tiny pieces (punctuation, indentation, escapes) and a call for each
 * of them costs far more than the bytes do.  The bytes produced are the same.
 */
#define DUMP_BUF_SIZE 4096

struct dump_out {
  FILE *fout;      /* stream to write to; NULL when building a string */
  jv *strout;      /* string to append to; NULL when writing to fout */
  int is_tty;
  size_t len, cap;
  char buf[DUMP_BUF_SIZE]; /* pending output: buf[0 .. len) */
};

static void dump_out_init(struct dump_out *o, FILE *fout, jv *strout, int is_tty) {
  o->fout = fout;
  o->strout = strout;
  o->is_tty = is_tty;
  o->len = 0;
  o->cap = sizeof(o->buf);
}

static void dump_out_flush(struct dump_out *o, const char *s, size_t n) {
  if (o->strout)
    *o->strout = jv_string_append_buf(*o->strout, s, n);
  else
    priv_fwrite(s, n, o->fout, o->is_tty); /* see util.h */
}

static void dump_out_finish(struct dump_out *o) {
  if (o->len)
    dump_out_flush(o, o->buf, o->len);
}

static void out_write_slow(struct dump_out *o, const char *s, size_t n) {
  if (o->fout && o->is_tty) {
    /* A console wants each write to be whole UTF-8 (see priv_fwrite). */
    priv_fwrite(s, n, o->fout, 1);
    return;
  }
  if (o->len + n > o->cap) {
    if (o->len)
      dump_out_flush(o, o->buf, o->len);
    o->len = 0;
    if (n > o->cap) {
      dump_out_flush(o, s, n);
      return;
    }
  }
  memcpy(o->buf + o->len, s, n);
  o->len += n;
}

static inline void out_write(struct dump_out *o, const char *s, size_t n) {
  if (n <= o->cap - o->len && !(o->fout && o->is_tty)) {
    memcpy(o->buf + o->len, s, n);
    o->len += n;
  } else {
    out_write_slow(o, s, n);
  }
}

static inline void put_char(char c, struct dump_out *o) {
  if (o->len < o->cap && !(o->fout && o->is_tty))
    o->buf[o->len++] = c;
  else
    out_write_slow(o, &c, 1);
}

static inline void put_str(const char *s, struct dump_out *o) {
  out_write(o, s, strlen(s));
}

static void put_indent(int n, int flags, struct dump_out *o) {
  static const char spaces[] = "                ";
  static const char tabs[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
  size_t total;
  const char *pad;
  if (flags & JV_PRINT_TAB) {
    total = n;
    pad = tabs;
  } else {
    total = (size_t)n * ((flags & (JV_PRINT_SPACE0 | JV_PRINT_SPACE1 | JV_PRINT_SPACE2)) >> 8);
    pad = spaces;
  }
  while (total > 0) {
    size_t k = total < sizeof(spaces) - 1 ? total : sizeof(spaces) - 1;
    out_write(o, pad, k);
    total -= k;
  }
}

static void put_unicode_escape(unsigned c, struct dump_out *o) {
  static const char hex[] = "0123456789abcdef";
  char buf[6] = {'\\', 'u', hex[(c >> 12) & 0xf], hex[(c >> 8) & 0xf],
                 hex[(c >> 4) & 0xf], hex[c & 0xf]};
  out_write(o, buf, sizeof(buf));
}

/* True if some byte of x needs escaping: below 0x20, '"', '\\', DEL, or
 * (with ascii_only) at least 0x80.  Exact for "some byte", so a word that
 * is reported clean has no such byte; a word reported dirty is looked at
 * byte by byte. */
static int word_needs_escape(uint64_t x, int ascii_only) {
  const uint64_t ones = UINT64_C(0x0101010101010101);
  const uint64_t highs = UINT64_C(0x8080808080808080);
  uint64_t q = x ^ (ones * '"');
  uint64_t b = x ^ (ones * '\\');
  uint64_t d = x ^ (ones * 0x7F);
  uint64_t t = (x - ones * 0x20) & ~x;
  t |= ((q - ones) & ~q) | ((b - ones) & ~b) | ((d - ones) & ~d);
  if (ascii_only)
    t |= x;
  return (t & highs) != 0;
}

static void jvp_dump_string(jv str, int ascii_only, struct dump_out *o) {
  assert(jv_get_kind(str) == JV_KIND_STRING);
  const char* start = jv_string_value(str);
  const char* end = start + jv_string_length_bytes(jv_copy(str));
  const char* i = start;
  const char* run = start; /* bytes before run are already written */
  put_char('"', o);
  while (i < end) {
    if (end - i >= 8) {
      uint64_t word;
      memcpy(&word, i, 8);
      if (!word_needs_escape(word, ascii_only)) {
        i += 8;
        continue;
      }
    }
    /* up to eight bytes one at a time before looking at words again */
    const char* lim = end - i >= 8 ? i + 8 : end;
    while (i < lim) {
      unsigned char b = (unsigned char)*i;
      if ((b >= 0x20 && b < 0x7F && b != '"' && b != '\\') || (b >= 0x80 && !ascii_only))
        i++; /* written as part of the run */
      else
        break;
    }
    if (i == lim)
      continue;
    unsigned char c = (unsigned char)*i;
    if (i > run)
      out_write(o, run, i - run);
    if (c >= 0x80) {
      /* Strings are valid UTF-8, so the sequence length follows from the lead byte. */
      const unsigned char* p = (const unsigned char*)i;
      int cp;
      if (c < 0xE0) {
        cp = ((c & 0x1F) << 6) | (p[1] & 0x3F);
        i += 2;
      } else if (c < 0xF0) {
        cp = ((c & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
        i += 3;
      } else {
        cp = ((c & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
        i += 4;
      }
      if (cp <= 0xffff) {
        put_unicode_escape(cp, o);
      } else {
        cp -= 0x10000;
        put_unicode_escape(0xD800 | ((cp & 0xffc00) >> 10), o);
        put_unicode_escape(0xDC00 | (cp & 0x003ff), o);
      }
    } else {
      i++;
      switch (c) {
      case '"':
      case '\\':
        put_char('\\', o);
        put_char((char)c, o);
        break;
      case '\b':
        out_write(o, "\\b", 2);
        break;
      case '\t':
        out_write(o, "\\t", 2);
        break;
      case '\r':
        out_write(o, "\\r", 2);
        break;
      case '\n':
        out_write(o, "\\n", 2);
        break;
      case '\f':
        out_write(o, "\\f", 2);
        break;
      default:
        /* ASCII control character or DEL */
        put_unicode_escape(c, o);
        break;
      }
    }
    run = i;
  }
  if (i > run)
    out_write(o, run, i - run);
  put_char('"', o);
}

static void put_refcnt(struct dtoa_context* C, int refcnt, struct dump_out *o) {
  char buf[JVP_DTOA_FMT_MAX_LEN];
  put_char(' ', o);
  put_char('(', o);
  put_str(jvp_dtoa_fmt(C, buf, refcnt), o);
  put_char(')', o);
}

static void jv_dump_term(struct dtoa_context* C, jv x, int flags, int indent, struct dump_out *o) {
  char buf[JVP_DTOA_FMT_MAX_LEN];
  const char* color = 0;
  double refcnt = (flags & JV_PRINT_REFCOUNT) ? jv_get_refcnt(x) - 1 : -1;
  if ((flags & JV_PRINT_COLOR) && jv_get_kind(x) != JV_KIND_INVALID) {
    color = colors[(int)jv_get_kind(x) - 1];
    put_str(color, o);
  }
  if (indent > MAX_PRINT_DEPTH) {
    put_str("<skipped: too deep>", o);
  } else switch (jv_get_kind(x)) {
  default:
  case JV_KIND_INVALID:
    if (flags & JV_PRINT_INVALID) {
      jv msg = jv_invalid_get_msg(jv_copy(x));
      if (jv_get_kind(msg) == JV_KIND_STRING) {
        put_str("<invalid:", o);
        jvp_dump_string(msg, flags | JV_PRINT_ASCII, o);
        put_char('>', o);
      } else {
        put_str("<invalid>", o);
      }
    } else {
      assert(0 && "Invalid value");
    }
    break;
  case JV_KIND_NULL:
    put_str("null", o);
    break;
  case JV_KIND_FALSE:
    put_str("false", o);
    break;
  case JV_KIND_TRUE:
    put_str("true", o);
    break;
  case JV_KIND_NUMBER: {
    if (jvp_number_is_nan(x)) {
      jv_dump_term(C, jv_null(), flags, indent, o);
    } else {
#ifdef USE_DECNUM
      const char * literal_data = jv_number_get_literal(x);
      if (literal_data) {
        put_str(literal_data, o);
      } else {
#endif
        double d = jv_number_value(x);
        if (d != d) {
          // JSON doesn't have NaN, so we'll render it as "null"
          put_str("null", o);
        } else {
          // Normalise infinities to something we can print in valid JSON
          if (d > DBL_MAX) d = DBL_MAX;
          if (d < -DBL_MAX) d = -DBL_MAX;
          put_str(jvp_dtoa_fmt(C, buf, d), o);
        }
      }
#ifdef USE_DECNUM
    }
#endif
    break;
  }
  case JV_KIND_STRING:
    jvp_dump_string(x, flags & JV_PRINT_ASCII, o);
    if (flags & JV_PRINT_REFCOUNT)
      put_refcnt(C, refcnt, o);
    break;
  case JV_KIND_ARRAY: {
    if (jv_array_length(jv_copy(x)) == 0) {
      put_str("[]", o);
      break;
    }
    put_char('[', o);
    jv_array_foreach(x, i, elem) {
      if (i!=0) {
        if (color) put_str(color, o);
        put_char(',', o);
      }
      if (color) put_str(COLRESET, o);
      if (flags & JV_PRINT_PRETTY) {
        put_char('\n', o);
        put_indent(indent + 1, flags, o);
      }
      jv_dump_term(C, elem, flags, indent + 1, o);
    }
    if (flags & JV_PRINT_PRETTY) {
      put_char('\n', o);
      put_indent(indent, flags, o);
    }
    if (color) put_str(color, o);
    put_char(']', o);
    if (flags & JV_PRINT_REFCOUNT)
      put_refcnt(C, refcnt, o);
    break;
  }
  case JV_KIND_OBJECT: {
    if (jv_object_length(jv_copy(x)) == 0) {
      put_str("{}", o);
      break;
    }
    put_char('{', o);
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
        if (color) put_str(color, o);
        put_char(',', o);
      }
      if (color) put_str(COLRESET, o);
      if (flags & JV_PRINT_PRETTY) {
        put_char('\n', o);
        put_indent(indent + 1, flags, o);
      }

      first = 0;
      if (color) put_str(FIELD_COLOR, o);
      jvp_dump_string(key, flags & JV_PRINT_ASCII, o);
      jv_free(key);
      if (color) put_str(COLRESET, o);

      if (color) put_str(color, o);
      put_char(':', o);
      if (color) put_str(COLRESET, o);
      if (flags & JV_PRINT_PRETTY) {
        put_char(' ', o);
      }

      jv_dump_term(C, value, flags, indent + 1, o);
    }
    if (flags & JV_PRINT_PRETTY) {
      put_char('\n', o);
      put_indent(indent, flags, o);
    }
    if (color) put_str(color, o);
    put_char('}', o);
    if (flags & JV_PRINT_REFCOUNT)
      put_refcnt(C, refcnt, o);
  }
  }
  jv_free(x);
  if (color) {
    put_str(COLRESET, o);
  }
}

void jv_dumpf(jv x, FILE *f, int flags) {
  struct dump_out o;
  dump_out_init(&o, f, NULL, flags & JV_PRINT_ISATTY);
  jv_dump_term(tsd_dtoa_context_get(), x, flags, 0, &o);
  dump_out_finish(&o);
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
  struct dump_out o;
  dump_out_init(&o, NULL, &s, 0);
  jv_dump_term(tsd_dtoa_context_get(), x, flags, 0, &o);
  dump_out_finish(&o);
  return s;
}

char *jv_dump_string_trunc(jv x, char *outbuf, size_t bufsize) {
  assert(bufsize > 0);
  if (jv_get_kind(x) == JV_KIND_STRING &&
      (size_t)jv_string_length_bytes(jv_copy(x)) > bufsize) {
    x = jv_string_slice(x, 0, bufsize);
  }
  x = jv_dump_string(x, 0);
  const char *str = jv_string_value(x);
  const size_t len = strlen(str);
  if (len > bufsize - 1 && bufsize >= 8) {
    char delim = 0;
    switch (str[0]) {
    case '"': delim = '"'; break;
    case '[': delim = ']'; break;
    case '{': delim = '}'; break;
    }
    size_t l = bufsize - (delim ? 5 : 4);  // "...", delim (if any), '\0'
    const char *s = jvp_utf8_backtrack(str + l, str, NULL);
    if (s) l = s - str;
    memcpy(outbuf, str, l);
    outbuf[l++] = '.';
    outbuf[l++] = '.';
    outbuf[l++] = '.';
    if (delim) outbuf[l++] = delim;
    outbuf[l] = '\0';
  } else {
    size_t l = MIN(len, bufsize - 1);
    memcpy(outbuf, str, l);
    outbuf[l] = '\0';
  }
  jv_free(x);
  return outbuf;
}
