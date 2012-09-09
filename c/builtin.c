#include "builtin.h"

static void f_false(jv input[], jv output[]) {
  jv_free(input[0]);
  output[0] = jv_false();
}

static void f_true(jv input[], jv output[]) {
  jv_free(input[0]);
  output[0] = jv_true();
}

static void f_null(jv input[], jv output[]) {
  jv_free(input[0]);
  output[0] = jv_null();
}

static void f_plus(jv input[], jv output[]) {
  jv_free(input[0]);
  jv a = input[2];
  jv b = input[1];
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    output[0] = jv_number(jv_number_value(a) + 
                          jv_number_value(b));
  } else if (jv_get_kind(a) == JV_KIND_ARRAY && jv_get_kind(b) == JV_KIND_ARRAY) {
    output[0] = jv_array_concat(a, b);
  } else if (jv_get_kind(a) == JV_KIND_OBJECT && jv_get_kind(b) == JV_KIND_OBJECT) {
    output[0] = jv_object_merge(a, b);
  } else {
    output[0] = jv_string("wtf gaize");
    jv_free(a);
    jv_free(b);
  }
}

struct cfunction function_list[] = {
  {f_true, "true", CALL_BUILTIN_1_1},
  {f_false, "false", CALL_BUILTIN_1_1},
  {f_null, "null", CALL_BUILTIN_1_1},
  {f_plus, "_plus", CALL_BUILTIN_3_1},
};
struct symbol_table builtins = {function_list, sizeof(function_list)/sizeof(function_list[0])};
