#include "opcode.h"

#define NONE 0
#define CONSTANT (OP_HAS_IMMEDIATE | OP_HAS_CONSTANT)
#define VARIABLE (OP_HAS_IMMEDIATE | OP_HAS_VARIABLE)
#define BRANCH (OP_HAS_IMMEDIATE | OP_HAS_BRANCH)
#define CFUNC (OP_HAS_IMMEDIATE | OP_HAS_SYMBOL | OP_HAS_CFUNC)

#define OP(name, imm, in, out) \
  {name, #name, imm, in, out},

static const struct opcode_description opcode_descriptions[] = {
#include "opcode_list.h"
};

static const struct opcode_description invalid_opcode_description = {
  -1, "#INVALID", 0, 0, 0
};


const struct opcode_description* opcode_describe(opcode op) {
  if ((int)op >= 0 && (int)op < NUM_OPCODES) {
    return &opcode_descriptions[op];
  } else {
    return &invalid_opcode_description;
  }
}
