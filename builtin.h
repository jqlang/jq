#ifndef BUILTIN_H
#define BUILTIN_H

#include "compile.h"

block builtins_bind(block);


typedef void (*cfunction_ptr)(jv input[], jv output[]);

struct cfunction {
  cfunction_ptr fptr;
  const char* name;
  int nargs;
};


#endif
