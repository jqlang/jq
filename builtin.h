#ifndef BUILTIN_H
#define BUILTIN_H

#include "compile.h"

block builtins_bind(block);


typedef void (*cfunction_ptr)(void);

struct cfunction {
  cfunction_ptr fptr;
  const char* name;
  int nargs;
};


jv cfunction_invoke(struct cfunction* function, jv input[]);


#endif
