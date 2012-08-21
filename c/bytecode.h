#ifndef BYTECODE_H
#define BYTECODE_H
#include <jansson.h>
#include <stdint.h>

#include "opcode.h"

typedef void (*cfunction_ptr)(json_t* input[], json_t* output[]);

struct cfunction {
  cfunction_ptr fptr;
  const char* name;
  opcode callop;
};

#define MAX_CFUNCTION_ARGS 10
struct symbol_table {
  struct cfunction* cfunctions;
  int ncfunctions;
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

  json_t* constants;
  struct symbol_table* globals;

  struct bytecode** subfunctions;
  int nsubfunctions;

  struct bytecode* parent;
};

void dump_disassembly(int, struct bytecode* code);
void dump_code(int, struct bytecode* code);
void dump_operation(struct bytecode* bc, uint16_t* op);

#endif
