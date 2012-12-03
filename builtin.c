#include <string.h>
#include "builtin.h"
#include "bytecode.h"
#include "compile.h"
#include "parser.h"
#include "locfile.h"
#include "jv_aux.h"


typedef jv (*func_1)(jv);
typedef jv (*func_2)(jv,jv);
typedef jv (*func_3)(jv,jv,jv);
typedef jv (*func_4)(jv,jv,jv,jv);
typedef jv (*func_5)(jv,jv,jv,jv,jv);
jv cfunction_invoke(struct cfunction* function, jv input[]) {
  switch (function->nargs) {
  case 1: return ((func_1)function->fptr)(input[0]);
  case 2: return ((func_2)function->fptr)(input[0], input[1]);
  case 3: return ((func_3)function->fptr)(input[0], input[1], input[2]);
  case 4: return ((func_4)function->fptr)(input[0], input[1], input[2], input[3]);
  case 5: return ((func_5)function->fptr)(input[0], input[1], input[2], input[3], input[4]);
  default: return jv_invalid_with_msg(jv_string("Function takes too many arguments"));
  }
}

static jv type_error(jv bad, const char* msg) {
  jv err = jv_invalid_with_msg(jv_string_fmt("%s %s",
                                             jv_kind_name(jv_get_kind(bad)),
                                             msg));
  jv_free(bad);
  return err;
}

static jv type_error2(jv bad1, jv bad2, const char* msg) {
  jv err = jv_invalid_with_msg(jv_string_fmt("%s and %s %s",
                                             jv_kind_name(jv_get_kind(bad1)),
                                             jv_kind_name(jv_get_kind(bad2)),
                                             msg));
  jv_free(bad1);
  jv_free(bad2);
  return err;
}


static jv f_plus(jv input, jv a, jv b) {
  jv_free(input);
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    return jv_number(jv_number_value(a) + 
                     jv_number_value(b));
  } else if (jv_get_kind(a) == JV_KIND_STRING && jv_get_kind(b) == JV_KIND_STRING) {
    return jv_string_concat(a, b);
  } else if (jv_get_kind(a) == JV_KIND_ARRAY && jv_get_kind(b) == JV_KIND_ARRAY) {
    return jv_array_concat(a, b);
  } else if (jv_get_kind(a) == JV_KIND_OBJECT && jv_get_kind(b) == JV_KIND_OBJECT) {
    return jv_object_merge(a, b);
  } else {
    return type_error2(a, b, "cannot be added");
  }
}

static jv f_minus(jv input, jv a, jv b) {
  jv_free(input);
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    return jv_number(jv_number_value(a) - jv_number_value(b));
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
    return out;
  } else {
    return type_error2(a, b, "cannot be subtracted");
  }
}

static jv f_multiply(jv input, jv a, jv b) {
  jv_free(input);
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    return jv_number(jv_number_value(a) * jv_number_value(b));
  } else {
    return type_error2(a, b, "cannot be multiplied");
  }  
}

static jv f_divide(jv input, jv a, jv b) {
  jv_free(input);
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    return jv_number(jv_number_value(a) / jv_number_value(b));
  } else {
    return type_error2(a, b, "cannot be divided");
  }  
}

static jv f_add(jv array) {
  if (jv_get_kind(array) != JV_KIND_ARRAY) {
    return type_error(array, "cannot have its elements added");
  } else if (jv_array_length(jv_copy(array)) == 0) {
    jv_free(array);
    return jv_null();
  } else {
    jv sum = jv_array_get(jv_copy(array), 0);
    for (int i = 1; i < jv_array_length(jv_copy(array)); i++) {
      sum = f_plus(jv_null(), sum, jv_array_get(jv_copy(array), i));
    }
    jv_free(array);
    return sum;
  }
}

static jv f_equal(jv input, jv a, jv b) {
  jv_free(input);
  return jv_bool(jv_equal(a, b));
}

static jv f_notequal(jv input, jv a, jv b) {
  jv_free(input);
  return jv_bool(!jv_equal(a, b));
}

enum cmp_op {
  CMP_OP_LESS,
  CMP_OP_GREATER,
  CMP_OP_LESSEQ,
  CMP_OP_GREATEREQ
};

static jv order_cmp(jv input, jv a, jv b, enum cmp_op op) {
  jv_free(input);
  int r = jv_cmp(a, b);
  return jv_bool((op == CMP_OP_LESS && r < 0) ||
                 (op == CMP_OP_LESSEQ && r <= 0) ||
                 (op == CMP_OP_GREATEREQ && r >= 0) ||
                 (op == CMP_OP_GREATER && r > 0));
}

static jv f_less(jv input, jv a, jv b) {
  return order_cmp(input, a, b, CMP_OP_LESS);
}

static jv f_greater(jv input, jv a, jv b) {
  return order_cmp(input, a, b, CMP_OP_GREATER);
}

static jv f_lesseq(jv input, jv a, jv b) {
  return order_cmp(input, a, b, CMP_OP_LESSEQ);
}

static jv f_greatereq(jv input, jv a, jv b) {
  return order_cmp(input, a, b, CMP_OP_GREATEREQ);
}

static jv f_contains(jv a, jv b) {
  jv_kind akind = jv_get_kind(a);

  if (akind == jv_get_kind(b)) {
    return jv_bool(jv_contains(a, b));
  } else {
    return type_error2(a, b, "cannot have their containment checked");
  }
}

static jv f_tonumber(jv input) {
  if (jv_get_kind(input) == JV_KIND_NUMBER) {
    return input;
  }
  if (jv_get_kind(input) == JV_KIND_STRING) {
    jv parsed = jv_parse(jv_string_value(input));
    if (!jv_is_valid(parsed) || jv_get_kind(parsed) == JV_KIND_NUMBER) {
      jv_free(input);
      return parsed;
    }
  }
  return type_error(input, "cannot be parsed as a number");
}

static jv f_length(jv input) {
  if (jv_get_kind(input) == JV_KIND_ARRAY) {
    return jv_number(jv_array_length(input));
  } else if (jv_get_kind(input) == JV_KIND_OBJECT) {
    return jv_number(jv_object_length(input));
  } else if (jv_get_kind(input) == JV_KIND_STRING) {
    return jv_number(jv_string_length(input));
  } else {
    return type_error(input, "has no length");
  }
}

static jv f_tostring(jv input) {
  if (jv_get_kind(input) == JV_KIND_STRING) {
    return input;
  } else {
    return jv_dump_string(input, 0);
  }
}

static jv f_keys(jv input) {
  if (jv_get_kind(input) == JV_KIND_OBJECT || jv_get_kind(input) == JV_KIND_ARRAY) {
    return jv_keys(input);
  } else {
    return type_error(input, "has no keys");
  }
}

static jv f_sort(jv input){
  if (jv_get_kind(input) == JV_KIND_ARRAY) {
    return jv_sort(input, jv_copy(input));
  } else {
    return type_error(input, "cannot be sorted, as it is not an array");
  }
}

static jv f_sort_by_impl(jv input, jv keys) {
  if (jv_get_kind(input) == JV_KIND_ARRAY && 
      jv_get_kind(keys) == JV_KIND_ARRAY &&
      jv_array_length(jv_copy(input)) == jv_array_length(jv_copy(keys))) {
    return jv_sort(input, keys);
  } else {
    return type_error2(input, keys, "cannot be sorted, as they are not both arrays");
  }
}

static jv f_group_by_impl(jv input, jv keys) {
  if (jv_get_kind(input) == JV_KIND_ARRAY && 
      jv_get_kind(keys) == JV_KIND_ARRAY &&
      jv_array_length(jv_copy(input)) == jv_array_length(jv_copy(keys))) {
    return jv_group(input, keys);
  } else {
    return type_error2(input, keys, "cannot be sorted, as they are not both arrays");
  }
}

static jv f_type(jv input) {
  jv out = jv_string(jv_kind_name(jv_get_kind(input)));
  jv_free(input);
  return out;
}

static struct cfunction function_list[] = {
  {(cfunction_ptr)f_plus, "_plus", 3},
  {(cfunction_ptr)f_minus, "_minus", 3},
  {(cfunction_ptr)f_multiply, "_multiply", 3},
  {(cfunction_ptr)f_divide, "_divide", 3},
  {(cfunction_ptr)f_tonumber, "tonumber", 1},
  {(cfunction_ptr)f_tostring, "tostring", 1},
  {(cfunction_ptr)f_keys, "keys", 1},
  {(cfunction_ptr)f_equal, "_equal", 3},
  {(cfunction_ptr)f_notequal, "_notequal", 3},
  {(cfunction_ptr)f_less, "_less", 3},
  {(cfunction_ptr)f_greater, "_greater", 3},
  {(cfunction_ptr)f_lesseq, "_lesseq", 3},
  {(cfunction_ptr)f_greatereq, "_greatereq", 3},
  {(cfunction_ptr)f_contains, "contains", 2},
  {(cfunction_ptr)f_length, "length", 1},
  {(cfunction_ptr)f_type, "type", 1},
  {(cfunction_ptr)f_add, "add", 1},
  {(cfunction_ptr)f_sort, "sort", 1},
  {(cfunction_ptr)f_sort_by_impl, "_sort_by_impl", 2},
  {(cfunction_ptr)f_group_by_impl, "_group_by_impl", 2},
};

static struct symbol_table cbuiltins = {function_list, sizeof(function_list)/sizeof(function_list[0])};

typedef block (*bytecoded_builtin)();
struct bytecoded_builtin { const char* name; block code; };
static block bind_bytecoded_builtins(block b) {
  struct bytecoded_builtin builtin_defs[] = {
    {"empty", gen_op_simple(BACKTRACK)},
    {"false", gen_const(jv_false())},
    {"true", gen_const(jv_true())},
    {"null", gen_const(jv_null())},
    {"not", gen_condbranch(gen_const(jv_false()),
                           gen_const(jv_true()))}
  };
  block builtins = gen_noop();
  for (unsigned i=0; i<sizeof(builtin_defs)/sizeof(builtin_defs[0]); i++) {
    builtins = BLOCK(builtins, gen_function(builtin_defs[i].name, builtin_defs[i].code));
  }
  return block_bind(builtins, b, OP_IS_CALL_PSEUDO);
}

static const char* jq_builtins[] = {
  "def map(f): [.[] | f];",
  "def select(f): if f then . else empty end;",
  "def sort_by(f): _sort_by_impl(map([f]));",
  "def group_by(f): _group_by_impl(map([f]));",
  "def unique: group_by(.) | map(.[0]);",
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
  b = bind_bytecoded_builtins(b);
  return gen_cbinding(&cbuiltins, b);
}
