#ifndef BUILTIN_H
#define BUILTIN_H

#include "jq.h"
#include "bytecode.h"
#include "compile.h"

int builtins_bind(jq_state *, block*);

#endif
