#include "jv.h"
#include <stdio.h>

void jv_dump(jv x) {
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
    printf("%f", jv_number_value(x));
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
