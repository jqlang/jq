#include "builtin.h"

#include <jansson.h>


static void f_false(json_t* input[], json_t* output[]) {
  output[0] = json_false();
}

static void f_true(json_t* input[], json_t* output[]) {
  output[0] = json_true();
}


static void f_plus(json_t* input[], json_t* output[]) {
  json_t* a = input[2];
  json_t* b = input[1];
  if (json_is_number(a) && json_is_number(b)) {
    output[0] = json_real(json_number_value(a) + 
                          json_number_value(b));
  } else if (json_is_array(a) && json_is_array(b)) {
    output[0] = json_copy(a);
    json_array_extend(output[0], b);
  } else {
    output[0] = json_string("wtf gaize");
  }
}

struct cfunction function_list[] = {
  {f_true, "true", CALL_BUILTIN_1_1},
  {f_false, "false", CALL_BUILTIN_1_1},
  {f_plus, "_plus", CALL_BUILTIN_3_1},
};
struct symbol_table builtins = {function_list, sizeof(function_list)/sizeof(function_list[0])};
