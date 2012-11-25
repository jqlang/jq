#include "opcode.h"

// flags, length
#define NONE 0, 1
#define CONSTANT OP_HAS_CONSTANT, 2
#define VARIABLE (OP_HAS_VARIABLE | OP_HAS_BINDING), 3
#define BRANCH OP_HAS_BRANCH, 2
#define CFUNC (OP_HAS_SYMBOL | OP_HAS_CFUNC | OP_HAS_VARIABLE_LENGTH_ARGLIST), 2
#define UFUNC (OP_HAS_UFUNC | OP_HAS_VARIABLE_LENGTH_ARGLIST), 2
#define CALLSEQ_END_IMM (OP_IS_CALL_PSEUDO), 0
#define CLOSURE_PARAM_IMM (OP_IS_CALL_PSEUDO | OP_HAS_BINDING), 0
#define CLOSURE_CREATE_IMM (OP_HAS_BLOCK | OP_IS_CALL_PSEUDO | OP_HAS_BINDING), 0
#define CLOSURE_CREATE_C_IMM (OP_IS_CALL_PSEUDO | OP_HAS_BINDING), 0
#define CLOSURE_REF_IMM (OP_IS_CALL_PSEUDO | OP_HAS_BINDING), 2

#define OP(name, imm, in, out) \
  {name, #name, imm, in, out},

static const struct opcode_description opcode_descriptions[] = {
#include "opcode_list.h"
};

static const struct opcode_description invalid_opcode_description = {
  -1, "#INVALID", 0, 0, 0, 0
};


const struct opcode_description* opcode_describe(opcode op) {
  if ((int)op >= 0 && (int)op < NUM_OPCODES) {
    return &opcode_descriptions[op];
  } else {
    return &invalid_opcode_description;
  }
}
