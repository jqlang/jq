#ifndef OPCODE_H
#define OPCODE_H
#include <assert.h>

typedef enum {
#define OP(name, imm, in, out) name,
#include "opcode_list.h"
#undef OP
} opcode;

enum {
  NUM_OPCODES = 
#define OP(name, imm, in, out) +1
#include "opcode_list.h"
#undef OP
};

enum {
  OP_HAS_CONSTANT = 2,
  OP_HAS_VARIABLE = 4,
  OP_HAS_BRANCH = 8,
  OP_HAS_CFUNC = 32,
  OP_HAS_UFUNC = 64,
  OP_IS_CALL_PSEUDO = 128,
  OP_HAS_BINDING = 1024,
};
struct opcode_description {
  opcode op;
  const char* name;

  int flags;

  // length in 16-bit units
  int length;

  int stack_in, stack_out;
};

const struct opcode_description* opcode_describe(opcode op);

#endif
