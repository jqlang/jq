#include "jv.h"
#include <stdio.h>

#include "jv_dtoa.h"

static void jv_dump_string(jv str, int ascii_only) {
  assert(jv_get_kind(str) == JV_KIND_STRING);
  const char* i = jv_string_value(str);
  const char* end = i + jv_string_length(str);
  while (i < end) {
    int unicode_escape;
    int c = (unsigned char)*i++;
    if (0x20 <= c && c <= 0x7E) {
      // printable ASCII
      if (c == '"' || c == '\\') {
        putchar('\\');
      }
      putchar(c);
      unicode_escape = 0;
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
    }
  }
}

static void jv_dump_term(struct dtoa_context* C, jv x) {
  char buf[JVP_DTOA_FMT_MAX_LEN];
  switch (jv_get_kind(x)) {
  case JV_KIND_NULL:
    printf("null");
    break;
  case JV_KIND_FALSE:
    printf("false");
    break;
  case JV_KIND_TRUE:
    printf("true");
    break;
  case JV_KIND_NUMBER:
    printf("%s", jvp_dtoa_fmt(C, buf, jv_number_value(x)));
    break;
  case JV_KIND_STRING:
    // FIXME: all sorts of broken
    printf("\"%s\"", jv_string_value(x));
    break;
  case JV_KIND_ARRAY: {
    printf("[");
    for (int i=0; i<jv_array_length(jv_copy(x)); i++) {
      if (i!=0) printf(", ");
      jv_dump(jv_array_get(jv_copy(x), i));
    }
    printf("]");
    break;
  }
  case JV_KIND_OBJECT: {
    printf("{");
    int first = 1;
    for (int i = jv_object_iter(x); jv_object_iter_valid(x,i); i = jv_object_iter_next(x,i)) {
      if (!first) printf(", ");
      first = 0;
      jv_dump(jv_object_iter_key(x, i));
      printf(": ");
      jv_dump(jv_object_iter_value(x, i));
    }
    printf("}");
  }
  }
  jv_free(x);
}

void jv_dump(jv x) {
  struct dtoa_context C;
  jvp_dtoa_context_init(&C);
  jv_dump_term(&C, x);
  jvp_dtoa_context_free(&C);
}
