#ifndef BUILTIN_H
#define BUILTIN_H

#include "jq.h"
#include "bytecode.h"
#include "compile.h"

#ifdef __cplusplus
extern "C" {
#endif

int builtins_bind(jq_state *, block*);

#ifdef __cplusplus
}
#endif

#endif
