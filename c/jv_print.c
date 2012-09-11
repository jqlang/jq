#include "jv.h"
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "jv_dtoa.h"
#include "jv_unicode.h"

static void jv_dump_string(jv str, int ascii_only) {
  assert(jv_get_kind(str) == JV_KIND_STRING);
  const char* i = jv_string_value(str);
  const char* end = i + jv_string_length(jv_copy(str));
  int c = 0;
  while ((i = jvp_utf8_next(i, end, &c))) {
    assert(c != -1);
    int unicode_escape = 0;
    if (0x20 <= c && c <= 0x7E) {
      // printable ASCII
      if (c == '"' || c == '\\') {
        putchar('\\');
      }
      putchar(c);
    } else if (c < 0x20 || c == 0x7F) {
      // ASCII control character
      switch (c) {
      case '\b':
        putchar('\\');
        putchar('b');
        break;
      case '\t':
        putchar('\\');
        putchar('t');
        break;
      case '\r':
        putchar('\\');
        putchar('r');
        break;
      case '\n':
        putchar('\\');
        putchar('n');
        break;
      case '\f':
        putchar('\\');
        putchar('f');
        break;
      default:
        unicode_escape = 1;
        break;
      }
    } else {
      unicode_escape = 1;
    }
    if (unicode_escape) {
      if (c <= 0xffff) {
        printf("\\u%04x", c);
      } else {
        c -= 0x10000;
        printf("\\u%04x\\u%04x", 
               0xD800 | ((c & 0xffc00) >> 10),
               0xDC00 | (c & 0x003ff));
      }
    }
  }
  assert(c != -1);
}

enum { INDENT = 2 };

static void jv_dump_term(struct dtoa_context* C, jv x, int flags, int indent) {
  char buf[JVP_DTOA_FMT_MAX_LEN];
  switch (jv_get_kind(x)) {
  case JV_KIND_INVALID:
    assert(0 && "Invalid value");
    break;
  case JV_KIND_NULL:
    printf("null");
    break;
  case JV_KIND_FALSE:
    printf("false");
    break;
  case JV_KIND_TRUE:
    printf("true");
    break;
  case JV_KIND_NUMBER: {
    double d = jv_number_value(x);
    if (d != d) {
      // JSON doesn't have NaN, so we'll render it as "null"
      printf("null");
    } else {
      // Normalise infinities to something we can print in valid JSON
      if (d > DBL_MAX) d = DBL_MAX;
      if (d < -DBL_MAX) d = -DBL_MAX;
      printf("%s", jvp_dtoa_fmt(C, buf, d));
    }
    break;
  }
  case JV_KIND_STRING:
    // FIXME: all sorts of broken
    putchar('"');
    jv_dump_string(x, 0);
    putchar('"');
    break;
  case JV_KIND_ARRAY: {
    if (jv_array_length(jv_copy(x)) == 0) {
      printf("[]");
      break;
    }
    printf("[");
    if (flags & JV_PRINT_PRETTY) printf("\n%*s", indent+INDENT, "");
    for (int i=0; i<jv_array_length(jv_copy(x)); i++) {
      if (i!=0) {
        if (flags & JV_PRINT_PRETTY) printf(",\n%*s", indent+INDENT, "");
        else printf(", ");
      }
      jv_dump_term(C, jv_array_get(jv_copy(x), i), flags, indent + INDENT);
    }
    if (flags & JV_PRINT_PRETTY) printf("\n%*s", indent, "");
    printf("]");
    break;
  }
  case JV_KIND_OBJECT: {
    if (jv_object_length(jv_copy(x)) == 0) {
      printf("{}");
      break;
    }
    printf("{");
    if (flags & JV_PRINT_PRETTY) printf("\n%*s", indent+INDENT, "");
    int first = 1;
    jv_object_foreach(i, x) {
      if (!first) {
        if (flags & JV_PRINT_PRETTY) printf(",\n%*s", indent+INDENT, "");
        else printf(", ");
      }
      first = 0;
      jv_dump_term(C, jv_object_iter_key(x, i), flags, indent + INDENT);
      printf(": ");
      jv_dump_term(C, jv_object_iter_value(x, i), flags, indent + INDENT);
    }
    if (flags & JV_PRINT_PRETTY) printf("\n%*s", indent, "");
    printf("}");
  }
  }
  jv_free(x);
}

void jv_dump(jv x, int flags) {
  struct dtoa_context C;
  jvp_dtoa_context_init(&C);
  jv_dump_term(&C, x, flags, 0);
  jvp_dtoa_context_free(&C);
}
