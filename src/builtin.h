#ifndef BUILTIN_H
#define BUILTIN_H

#include "jq.h"
#include "bytecode.h"
#include "compile.h"

int builtins_bind(jq_state *, block*);

#define BINOPS \
  BINOP(plus) \
  BINOP(minus) \
  BINOP(multiply) \
  BINOP(divide) \
  BINOP(mod) \
  BINOP(equal) \
  BINOP(notequal) \
  BINOP(less) \
  BINOP(lesseq) \
  BINOP(greater) \
  BINOP(greatereq) \


#define BINOP(name) jv binop_ ## name(jv, jv);
BINOPS
#undef BINOP

#endif
