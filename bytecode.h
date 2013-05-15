#ifndef BYTECODE_H
#define BYTECODE_H
#include <stdint.h>

#include "jv.h"
#include "opcode.h"
#include "builtin.h"

#define MAX_CFUNCTION_ARGS 10
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
void dump_code(int, struct bytecode* code);
void dump_operation(struct bytecode* bc, uint16_t* op);

void symbol_table_free(struct symbol_table* syms);
void bytecode_free(struct bytecode* bc);

#endif
