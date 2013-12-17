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
  int close_it;
};

int builtins_bind(jq_state *, block*);

#endif
