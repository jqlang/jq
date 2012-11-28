#include <string.h>
#include "builtin.h"
#include "compile.h"
#include "jq_parser.h"
#include "locfile.h"

enum {
  CMP_OP_LESS,
  CMP_OP_GREATER,
  CMP_OP_LESSEQ,
  CMP_OP_GREATEREQ
} _cmp_op;

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

static void f_add(jv input[], jv output[]) {
  jv array = input[0];
  if (jv_get_kind(array) != JV_KIND_ARRAY) {
    output[0] = jv_invalid_with_msg(jv_string_fmt("Cannot add elements of an %s",
                                                  jv_kind_name(jv_get_kind(array))));
  } else if (jv_array_length(jv_copy(array)) == 0) {
    output[0] = jv_null();
  } else {
    jv sum = jv_array_get(jv_copy(array), 0);
    for (int i = 1; i < jv_array_length(jv_copy(array)); i++) {
      jv x = jv_array_get(jv_copy(array), i);
      jv add_args[] = {jv_null(), x, sum};
      f_plus(add_args, &sum);
    }
    output[0] = sum;
  }
  jv_free(array);
}

static void f_equal(jv input[], jv output[]) {
  jv_free(input[0]);
  output[0] = jv_bool(jv_equal(input[2], input[1]));
}

static void f_notequal(jv input[], jv output[]) {
  jv_free(input[0]);
  output[0] = jv_bool(jv_equal(input[2], input[1]) == 0);
}

static void order_cmp(jv input[], jv output[], int op) {
  jv_free(input[0]);
  jv a = input[2];
  jv b = input[1];
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    double da = jv_number_value(a);
    double db = jv_number_value(b);
    output[0] = jv_bool((op == CMP_OP_LESS && da < db) ||
                        (op == CMP_OP_LESSEQ && da <= db) ||
                        (op == CMP_OP_GREATEREQ && da >= db) ||
                        (op == CMP_OP_GREATER && da > db));
  } else {
    output[0] = jv_invalid_with_msg(jv_string_fmt("Attempted to compare order of %s wrt %s",
                                                  jv_kind_name(jv_get_kind(a)),
                                                  jv_kind_name(jv_get_kind(b))));
    jv_free(a);
    jv_free(b);
  }
}

static void f_less(jv input[], jv output[]) {
  order_cmp(input, output, CMP_OP_LESS);
}

static void f_greater(jv input[], jv output[]) {
  order_cmp(input, output, CMP_OP_GREATER);
}

static void f_lesseq(jv input[], jv output[]) {
  order_cmp(input, output, CMP_OP_LESSEQ);
}

static void f_greatereq(jv input[], jv output[]) {
  order_cmp(input, output, CMP_OP_GREATEREQ);
}

static void f_contains(jv input[], jv output[]) {
  jv_free(input[0]);
  jv a = input[2];
  jv b = input[1];
  jv_kind akind = jv_get_kind(a);

  if (akind == jv_get_kind(b)) {
    output[0] = jv_bool(jv_contains(a, b));
  } else {  
    output[0] = jv_invalid_with_msg(jv_string_fmt("Can only check containment of values of the same type."));
    jv_free(a);
    jv_free(b);
  }
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

static int string_cmp(const void* pa, const void* pb){
  const jv* a = pa;
  const jv* b = pb;
  int lena = jv_string_length(jv_copy(*a));
  int lenb = jv_string_length(jv_copy(*b));
  int minlen = lena < lenb ? lena : lenb;
  int r = memcmp(jv_string_value(*a), jv_string_value(*b), minlen);
  if (r == 0) r = lena - lenb;
  return r;
}

static void f_keys(jv input[], jv output[]) {
  if (jv_get_kind(input[0]) == JV_KIND_OBJECT) {
    int nkeys = jv_object_length(jv_copy(input[0]));
    jv* keys = malloc(sizeof(jv) * nkeys);
    int kidx = 0;
    jv_object_foreach(i, input[0]) {
      keys[kidx++] = jv_object_iter_key(input[0], i);
    }
    qsort(keys, nkeys, sizeof(jv), string_cmp);
    output[0] = jv_array_sized(nkeys);
    for (int i = 0; i<nkeys; i++) {
      output[0] = jv_array_append(output[0], keys[i]);
    }
    free(keys);
    jv_free(input[0]);
  } else if (jv_get_kind(input[0]) == JV_KIND_ARRAY) {
    int n = jv_array_length(input[0]);
    output[0] = jv_array();
    for (int i=0; i<n; i++){
      output[0] = jv_array_set(output[0], i, jv_number(i));
    }
  } else {
    output[0] = jv_invalid_with_msg(jv_string_fmt("'keys' only supports object, not %s",
                                                  jv_kind_name(jv_get_kind(input[0]))));
    jv_free(input[0]);
  }
}

static void f_type(jv input[], jv output[]) {
  output[0] = jv_string(jv_kind_name(jv_get_kind(input[0])));
  jv_free(input[0]);
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

static block j_not() {
  return gen_op_block_defn(CLOSURE_CREATE, "not",
                           block_join(gen_condbranch(gen_op_const(LOADK, jv_false()),
                                                     gen_op_const(LOADK, jv_true())),
                                      gen_op_simple(RET)));
}

static struct cfunction function_list[] = {
  {f_plus, "_plus", CALL_BUILTIN_3_1},
  {f_minus, "_minus", CALL_BUILTIN_3_1},
  {f_multiply, "_multiply", CALL_BUILTIN_3_1},
  {f_divide, "_divide", CALL_BUILTIN_3_1},
  {f_tonumber, "tonumber", CALL_BUILTIN_1_1},
  {f_tostring, "tostring", CALL_BUILTIN_1_1},
  {f_keys, "keys", CALL_BUILTIN_1_1},
  {f_equal, "_equal", CALL_BUILTIN_3_1},
  {f_notequal, "_notequal", CALL_BUILTIN_3_1},
  {f_less, "_less", CALL_BUILTIN_3_1},
  {f_greater, "_greater", CALL_BUILTIN_3_1},
  {f_lesseq, "_lesseq", CALL_BUILTIN_3_1},
  {f_greatereq, "_greatereq", CALL_BUILTIN_3_1},
  {f_contains, "_contains", CALL_BUILTIN_3_1},
  {f_length, "length", CALL_BUILTIN_1_1},
  {f_type, "type", CALL_BUILTIN_1_1},
  {f_add, "add", CALL_BUILTIN_1_1},
};

static struct symbol_table cbuiltins = {function_list, sizeof(function_list)/sizeof(function_list[0])};

typedef block (*bytecoded_builtin)();
static bytecoded_builtin bytecoded_builtins[] = {
  j_empty,
  j_false,
  j_true,
  j_null,
  j_not,
};

static const char* jq_builtins[] = {
  "def map(f): [.[] | f];",
  "def select(f): if f then . else empty end;",
};


block builtins_bind(block b) {
  for (int i=(int)(sizeof(jq_builtins)/sizeof(jq_builtins[0]))-1; i>=0; i--) {
    struct locfile src;
    locfile_init(&src, jq_builtins[i], strlen(jq_builtins[i]));
    block funcs;
    int nerrors = jq_parse_library(&src, &funcs);
    assert(!nerrors);
    b = block_bind(funcs, b, OP_IS_CALL_PSEUDO);
    locfile_free(&src);
  }
  block builtins = gen_noop();
  for (unsigned i=0; i<sizeof(bytecoded_builtins)/sizeof(bytecoded_builtins[0]); i++) {
    block_append(&builtins, bytecoded_builtins[i]());
  }
  b = block_bind(builtins, b, OP_IS_CALL_PSEUDO);
  return gen_cbinding(&cbuiltins, b);
}
