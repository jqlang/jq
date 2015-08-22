#ifndef BYTECODE_H
#define BYTECODE_H
#include <stdint.h>

#include "jv.h"

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
  // NOTE: Not actually part of any op -- a pseudo-op flag for special
  //       handling of `break`.
  OP_BIND_WILDCARD = 2048,
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


#define MAX_CFUNCTION_ARGS 10
typedef void (*cfunction_ptr)();
struct cfunction {
  cfunction_ptr fptr;
  const char* name;
  int nargs;
};

struct symbol_table {
  struct cfunction* cfunctions;
  int ncfunctions;
  jv cfunc_names;
};

// The bytecode format matters in:
//   execute.c  - interpreter
//   compile.c  - compiler
//   bytecode.c - disassembler

#define ARG_NEWCLOSURE 0x1000

struct bytecode {
  uint16_t* code;
  int codelen;

  int nlocals;
  int nclosures;

  jv constants; // JSON array of constants
  struct symbol_table* globals;

  struct bytecode** subfunctions;
  int nsubfunctions;

  struct bytecode* parent;

  jv debuginfo;
};

void dump_disassembly(int, struct bytecode* code);
void dump_operation(struct bytecode* bc, uint16_t* op);

int bytecode_operation_length(uint16_t* codeptr);
void bytecode_free(struct bytecode* bc);

#endif
