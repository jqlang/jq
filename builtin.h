#ifndef BUILTIN_H
#define BUILTIN_H

#include "jq.h"
#include "jv.h"
#include "bytecode.h"
#include "compile.h"

struct jq_stdio_handle {
  char buf[4096];
  FILE *f;
  struct jv_parser *p;
  jv s;
  unsigned int close_it:1;
  unsigned int is_pipe:1;
  unsigned int is_null:1;
};

int builtins_bind(jq_state *, block*);

#endif
