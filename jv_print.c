#include "jv.h"
#include <stdio.h>
#include <float.h>
#include <string.h>
#include <assert.h>

#include "jv_dtoa.h"
#include "jv_unicode.h"

#define ESC "\033"
#define COL(c) (ESC "[" c "m")
#define COLRESET (ESC "[0m")

// Colour table. See http://en.wikipedia.org/wiki/ANSI_escape_code#Colors
// for how to choose these.
static const jv_kind colour_kinds[] = 
  {JV_KIND_NULL,   JV_KIND_FALSE, JV_KIND_TRUE, JV_KIND_NUMBER,
   JV_KIND_STRING, JV_KIND_ARRAY, JV_KIND_OBJECT};
static const char* const colours[] =
  {COL("1;30"),    COL("0;39"),      COL("0;39"),     COL("0;39"),
   COL("0;32"),      COL("1;39"),     COL("1;39")};
#define FIELD_COLOUR COL("34;1")

static void put_buf(const char* s, int len, FILE* fout, jv* strout) {
  if (strout) {
    *strout = jv_string_append_buf(*strout, s, len);
  } else {
    fwrite(s, 1, len, fout);
  }
}

static void put_char(char c, FILE* fout, jv* strout) {
  put_buf(&c, 1, fout, strout);
}

static void put_str(const char* s, FILE* fout, jv* strout) {
  put_buf(s, strlen(s), fout, strout);
}

static void put_space(int n, FILE* fout, jv* strout) {
  while (n--) {
    put_char(' ', fout, strout);
  }
}

static void jvp_dump_string(jv str, int ascii_only, FILE* F, jv* S) {
  assert(jv_get_kind(str) == JV_KIND_STRING);
  const char* i = jv_string_value(str);
  const char* end = i + jv_string_length_bytes(jv_copy(str));
  const char* cstart;
  int c = 0;
  char buf[32];
  put_char('"', F, S);
  while ((i = jvp_utf8_next((cstart = i), end, &c))) {
    assert(c != -1);
    int unicode_escape = 0;
    if (0x20 <= c && c <= 0x7E) {
      // printable ASCII
      if (c == '"' || c == '\\') {
        put_char('\\', F, S);
      }
      put_char(c, F, S);
    } else if (c < 0x20 || c == 0x7F) {
      // ASCII control character
      switch (c) {
      case '\b':
        put_char('\\', F, S);
        put_char('b', F, S);
        break;
      case '\t':
        put_char('\\', F, S);
        put_char('t', F, S);
        break;
      case '\r':
        put_char('\\', F, S);
        put_char('r', F, S);
        break;
      case '\n':
        put_char('\\', F, S);
        put_char('n', F, S);
        break;
      case '\f':
        put_char('\\', F, S);
        put_char('f', F, S);
        break;
      default:
        unicode_escape = 1;
        break;
      }
    } else {
      if (ascii_only) {
        unicode_escape = 1;
      } else {
        put_buf(cstart, i - cstart, F, S);
      }
    }
    if (unicode_escape) {
      if (c <= 0xffff) {
        sprintf(buf, "\\u%04x", c);
      } else {
        c -= 0x10000;
        sprintf(buf, "\\u%04x\\u%04x", 
                0xD800 | ((c & 0xffc00) >> 10),
                0xDC00 | (c & 0x003ff));
      }
      put_str(buf, F, S);
    }
  }
  assert(c != -1);
  put_char('"', F, S);
}

enum { INDENT = 2 };

static void jv_dump_term(struct dtoa_context* C, jv x, int flags, int indent, FILE* F, jv* S) {
  char buf[JVP_DTOA_FMT_MAX_LEN];
  const char* colour = 0;
  if (flags & JV_PRINT_COLOUR) {
    for (unsigned i=0; i<sizeof(colour_kinds)/sizeof(colour_kinds[0]); i++) {
      if (jv_get_kind(x) == colour_kinds[i]) {
        colour = colours[i];
        put_str(colour, F, S);
        break;
      }
    }
  }
  switch (jv_get_kind(x)) {
  default:
  case JV_KIND_INVALID:
    if (flags & JV_PRINT_INVALID) {
      jv msg = jv_invalid_get_msg(jv_copy(x));
      if (jv_get_kind(msg) == JV_KIND_STRING) {
        put_str("<invalid:", F, S);
        jvp_dump_string(msg, flags | JV_PRINT_ASCII, F, S);
        put_str(">", F, S);
      } else {
        put_str("<invalid>", F, S);
      }
    } else {
      assert(0 && "Invalid value");
    }
    break;
  case JV_KIND_NULL:
    put_str("null", F, S);
    break;
  case JV_KIND_FALSE:
    put_str("false", F, S);
    break;
  case JV_KIND_TRUE:
    put_str("true", F, S);
    break;
  case JV_KIND_NUMBER: {
    double d = jv_number_value(x);
    if (d != d) {
      // JSON doesn't have NaN, so we'll render it as "null"
      put_str("null", F, S);
    } else {
      // Normalise infinities to something we can print in valid JSON
      if (d > DBL_MAX) d = DBL_MAX;
      if (d < -DBL_MAX) d = -DBL_MAX;
      put_str(jvp_dtoa_fmt(C, buf, d), F, S);
    }
    break;
  }
  case JV_KIND_STRING:
    jvp_dump_string(x, flags & JV_PRINT_ASCII, F, S);
    break;
  case JV_KIND_ARRAY: {
    if (jv_array_length(jv_copy(x)) == 0) {
      put_str("[]", F, S);
      break;
    }
    put_str("[", F, S);
    if (flags & JV_PRINT_PRETTY) {
      put_char('\n', F, S);
      put_space(indent+INDENT, F, S);
    }
    jv_array_foreach(x, i, elem) {
      if (i!=0) {
        if (flags & JV_PRINT_PRETTY) {
          put_str(",\n", F, S);
          put_space(indent+INDENT, F, S);
        } else {
          put_str(",", F, S);
        }
      }
      jv_dump_term(C, elem, flags, indent + INDENT, F, S);
      if (colour) put_str(colour, F, S);
    }
    if (flags & JV_PRINT_PRETTY) {
      put_char('\n', F, S);
      put_space(indent, F, S);
    }
    if (colour) put_str(colour, F, S);
    put_char(']', F, S);
    break;
  }
  case JV_KIND_OBJECT: {
    if (jv_object_length(jv_copy(x)) == 0) {
      put_str("{}", F, S);
      break;
    }
    put_char('{', F, S);
    if (flags & JV_PRINT_PRETTY) {
      put_char('\n', F, S);
      put_space(indent+INDENT, F, S);
    }
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
        if (flags & JV_PRINT_PRETTY){
          put_str(",\n", F, S);
          put_space(indent+INDENT, F, S);
        } else {
          put_str(",", F, S);
        }
      }
      if (colour) put_str(COLRESET, F, S);

      first = 0;
      if (colour) put_str(FIELD_COLOUR, F, S);
      jvp_dump_string(key, flags & JV_PRINT_ASCII, F, S);
      jv_free(key);
      if (colour) put_str(COLRESET, F, S);

      if (colour) put_str(colour, F, S);
      put_str((flags & JV_PRINT_PRETTY) ? ": " : ":", F, S);
      if (colour) put_str(COLRESET, F, S);
      
      jv_dump_term(C, value, flags, indent + INDENT, F, S);
      if (colour) put_str(colour, F, S);
    }
    if (flags & JV_PRINT_PRETTY) {
      put_char('\n', F, S);
      put_space(indent, F, S);
    }
    if (colour) put_str(colour, F, S);
    put_char('}', F, S);
  }
  }
  jv_free(x);
  if (colour) {
    put_str(COLRESET, F, S);
  }
}

void jv_dumpf(jv x, FILE *f, int flags) {
  struct dtoa_context C;
  jvp_dtoa_context_init(&C);
  jv_dump_term(&C, x, flags, 0, f, 0);
  jvp_dtoa_context_free(&C);
}

void jv_dump(jv x, int flags) {
  jv_dumpf(x, stdout, flags);
}

/* This one is nice for use in debuggers */
void jv_show(jv x, int flags) {
  if (flags == -1)
    flags = JV_PRINT_PRETTY | JV_PRINT_COLOUR;
  jv_dumpf(jv_copy(x), stderr, flags | JV_PRINT_INVALID);
  fflush(stderr);
}

jv jv_dump_string(jv x, int flags) {
  struct dtoa_context C;
  jvp_dtoa_context_init(&C);
  jv s = jv_string("");
  jv_dump_term(&C, x, flags, 0, 0, &s);
  jvp_dtoa_context_free(&C);
  return s;
}
