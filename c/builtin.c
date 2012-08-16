#include "builtin.h"

#include <jansson.h>


void f_false(json_t* input[], json_t* output[]) {
  output[0] = json_false();
}

void f_true(json_t* input[], json_t* output[]) {
  output[0] = json_true();
}

struct cfunction function_list[] = {
  {f_true, "true", CALL_BUILTIN_1_1},
  {f_false, "false", CALL_BUILTIN_1_1},
};
struct symbol_table builtins = {function_list, sizeof(function_list)/sizeof(function_list[0])};
