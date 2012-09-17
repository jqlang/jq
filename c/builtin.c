#include "builtin.h"
#include "compile.h"

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

static void f_length(jv input[], jv output[]) {
  if (jv_get_kind(input[0]) == JV_KIND_ARRAY) {
    output[0] = jv_number(jv_array_length(input[0]));
  } else if (jv_get_kind(input[0]) == JV_KIND_OBJECT) {
    output[0] = jv_number(jv_object_length(input[0]));
  } else if (jv_get_kind(input[0]) == JV_KIND_STRING) {
    output[0] = jv_number(jv_string_length(input[0]));
  } else {
    output[0] = jv_invalid_with_msg(jv_string_fmt("Cannot get the length of %s",
                                                  jv_kind_name(jv_get_kind(input[0]))));
    jv_free(input[0]);
  }
}

static void f_tostring(jv input[], jv output[]) {
  if (jv_get_kind(input[0]) == JV_KIND_STRING) {
    output[0] = input[0];
  } else {
    output[0] = jv_dump_string(input[0], 0);
  }
}

static block j_empty() {
  return gen_op_block_defn(CLOSURE_CREATE, "empty", gen_op_simple(BACKTRACK));
}

static block j_false() {
  return gen_op_block_defn(CLOSURE_CREATE, "false",
                           block_join(gen_op_const(LOADK, jv_false()),
                                      gen_op_simple(RET)));
}

static block j_true() {
  return gen_op_block_defn(CLOSURE_CREATE, "true",
                           block_join(gen_op_const(LOADK, jv_true()),
                                      gen_op_simple(RET)));
}

static block j_null() {
  return gen_op_block_defn(CLOSURE_CREATE, "null",
                           block_join(gen_op_const(LOADK, jv_null()),
                                      gen_op_simple(RET)));
}


static struct cfunction function_list[] = {
  {f_plus, "_plus", CALL_BUILTIN_3_1},
  {f_minus, "_minus", CALL_BUILTIN_3_1},
  {f_multiply, "_multiply", CALL_BUILTIN_3_1},
  {f_divide, "_divide", CALL_BUILTIN_3_1},
  {f_tonumber, "tonumber", CALL_BUILTIN_1_1},
  {f_tostring, "tostring", CALL_BUILTIN_1_1},
  {f_equal, "_equal", CALL_BUILTIN_3_1},
  {f_length, "length", CALL_BUILTIN_1_1},
};

static struct symbol_table builtins = {function_list, sizeof(function_list)/sizeof(function_list[0])};

typedef block (*bytecoded_builtin)();
static bytecoded_builtin bytecoded_builtins[] = {
  j_empty,
  j_false,
  j_true,
  j_null,
};


block builtins_bind(block b) {
  for (int i=0; i<sizeof(bytecoded_builtins)/sizeof(bytecoded_builtins[0]); i++) {
    b = block_bind(bytecoded_builtins[i](), b, OP_IS_CALL_PSEUDO);
  }
  return gen_cbinding(&builtins, b);
}
