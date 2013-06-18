#ifndef BUILTIN_H
#define BUILTIN_H

#include "bytecode.h"
#include "compile.h"

int builtins_bind(block*);

jv cfunction_invoke(struct cfunction* function, jv input[]);


#endif
