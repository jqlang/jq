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
  } else if (jv_get_kind(a) == JV_KIND_STRING && jv_get_kind(b) == JV_KIND_STRING) {
    output[0] = jv_string_concat(a, b);
  } else if (jv_get_kind(a) == JV_KIND_ARRAY && jv_get_kind(b) == JV_KIND_ARRAY) {
    output[0] = jv_array_concat(a, b);
  } else if (jv_get_kind(a) == JV_KIND_OBJECT && jv_get_kind(b) == JV_KIND_OBJECT) {
    output[0] = jv_object_merge(a, b);
  } else {
    output[0] = jv_invalid_with_msg(jv_string_fmt("Attempted to add %s and %s",
                                                  jv_kind_name(jv_get_kind(a)),
                                                  jv_kind_name(jv_get_kind(b))));
    jv_free(a);
    jv_free(b);
  }
}

static void f_minus(jv input[], jv output[]) {
  jv_free(input[0]);
  jv a = input[2];
  jv b = input[1];
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    output[0] = jv_number(jv_number_value(a) - jv_number_value(b));
  } else if (jv_get_kind(a) == JV_KIND_ARRAY && jv_get_kind(b) == JV_KIND_ARRAY) {
    jv out = jv_array();
    for (int i=0; i<jv_array_length(jv_copy(a)); i++) {
      jv x = jv_array_get(jv_copy(a), i);
      int include = 1;
      for (int j=0; j<jv_array_length(jv_copy(b)); j++) {
        if (jv_equal(jv_copy(x), jv_array_get(jv_copy(b), j))) {
          include = 0;
          break;
        }
      }
      if (include)
        out = jv_array_append(out, jv_copy(x));
      jv_free(x);
    }
    jv_free(a);
    jv_free(b);
    output[0] = out;
  } else {
    output[0] = jv_invalid_with_msg(jv_string_fmt("Attempted to subtract %s and %s",
                                                  jv_kind_name(jv_get_kind(a)),
                                                  jv_kind_name(jv_get_kind(b))));
    jv_free(a);
    jv_free(b);
  }
}

static void f_multiply(jv input[], jv output[]) {
  jv_free(input[0]);
  jv a = input[2];
  jv b = input[1];
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    output[0] = jv_number(jv_number_value(a) * jv_number_value(b));
  } else {
    output[0] = jv_invalid_with_msg(jv_string_fmt("Attempted to multiply %s and %s",
                                                  jv_kind_name(jv_get_kind(a)),
                                                  jv_kind_name(jv_get_kind(b))));
    jv_free(a);
    jv_free(b);
  }  
}

static void f_divide(jv input[], jv output[]) {
  jv_free(input[0]);
  jv a = input[2];
  jv b = input[1];
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    output[0] = jv_number(jv_number_value(a) / jv_number_value(b));
  } else {
    output[0] = jv_invalid_with_msg(jv_string_fmt("Attempted to divide %s by %s",
                                                  jv_kind_name(jv_get_kind(a)),
                                                  jv_kind_name(jv_get_kind(b))));
    jv_free(a);
    jv_free(b);
  }  
}

static void f_equal(jv input[], jv output[]) {
  jv_free(input[0]);
  output[0] = jv_bool(jv_equal(input[2], input[1]));
}

static void f_tonumber(jv input[], jv output[]) {
  if (jv_get_kind(input[0]) == JV_KIND_NUMBER) {
    output[0] = input[0];
  } else if (jv_get_kind(input[0]) == JV_KIND_STRING) {
    jv parsed = jv_parse(jv_string_value(input[0]));
    if (!jv_is_valid(parsed)) {
      jv_free(input[0]);
      output[0] = parsed;
    } else if (jv_get_kind(parsed) != JV_KIND_NUMBER) {
      output[0] = jv_invalid_with_msg(jv_string_fmt("'%s' is not a number",
                                                    jv_string_value(input[0])));
      jv_free(input[0]);
    } else {
      jv_free(input[0]);
      output[0] = parsed;
    }
  } else {
    output[0] = jv_invalid_with_msg(jv_string_fmt("Cannot parse %s as a number",
                                                  jv_kind_name(jv_get_kind(input[0]))));
    jv_free(input[0]);
    return;
  }
}

struct cfunction function_list[] = {
  {f_true, "true", CALL_BUILTIN_1_1},
  {f_false, "false", CALL_BUILTIN_1_1},
  {f_null, "null", CALL_BUILTIN_1_1},
  {f_plus, "_plus", CALL_BUILTIN_3_1},
  {f_minus, "_minus", CALL_BUILTIN_3_1},
  {f_multiply, "_multiply", CALL_BUILTIN_3_1},
  {f_divide, "_divide", CALL_BUILTIN_3_1},
  {f_tonumber, "tonumber", CALL_BUILTIN_1_1},
  {f_equal, "_equal", CALL_BUILTIN_3_1},
};
struct symbol_table builtins = {function_list, sizeof(function_list)/sizeof(function_list[0])};
