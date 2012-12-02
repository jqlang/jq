#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bytecode.h"
#include "opcode.h"

static int bytecode_operation_length(uint16_t* codeptr) {
  if (opcode_describe(*codeptr)->flags & OP_HAS_VARIABLE_LENGTH_ARGLIST) {
    return 4 + codeptr[1] * 2;
  } else {
    return opcode_length(*codeptr);
  }
}

void dump_disassembly(int indent, struct bytecode* bc) {
  if (bc->nclosures > 0) {
    printf("%*s[params: %d]\n", indent, "", bc->nclosures);
  }
  dump_code(indent, bc);
  for (int i=0; i<bc->nsubfunctions; i++) {
    printf("%*ssubfn[%d]:\n", indent, "", i);
    dump_disassembly(indent+2, bc->subfunctions[i]);
  }
}

void dump_code(int indent, struct bytecode* bc) {
  int pc = 0;
  while (pc < bc->codelen) {
    printf("%*s", indent, "");
    dump_operation(bc, bc->code + pc);
    printf("\n");
    pc += bytecode_operation_length(bc->code + pc);
  }
}

void dump_operation(struct bytecode* bc, uint16_t* codeptr) {
  int pc = codeptr - bc->code;
  printf("%04d ", pc);
  const struct opcode_description* op = opcode_describe(bc->code[pc++]);
  printf("%s", op->name);
  if (op->length > 1) {
    uint16_t imm = bc->code[pc++];
    if (op->flags & OP_HAS_VARIABLE_LENGTH_ARGLIST) {
      for (int i=0; i<imm+1; i++) {
        uint16_t level = bc->code[pc++];
        uint16_t idx = bc->code[pc++];
        if (idx & ARG_NEWCLOSURE) {
          printf(" subfn[%d]", idx & ~ARG_NEWCLOSURE);
        } else {
          printf(" param[%d]", idx);
        }
        if (level) {
          printf("^%d", level);
        }
      }
    } else if (op->flags & OP_HAS_BRANCH) {
      printf(" %04d", pc + imm);
    } else if (op->flags & OP_HAS_CONSTANT) {
      printf(" ");
      jv_dump(jv_array_get(jv_copy(bc->constants), imm), 0);
    } else if (op->flags & OP_HAS_VARIABLE) {
      uint16_t v = bc->code[pc++];
      printf(" v%d", v);
      if (imm) {
        printf("^%d", imm);
      }
    } else {
      printf(" %d", imm);
    }
  }  
}

void symbol_table_free(struct symbol_table* syms) {
  free(syms->cfunctions);
  free(syms);
}

void bytecode_free(struct bytecode* bc) {
  free(bc->code);
  jv_free(bc->constants);
  for (int i=0; i<bc->nsubfunctions; i++)
    bytecode_free(bc->subfunctions[i]);
  if (!bc->parent)
    symbol_table_free(bc->globals);
  free(bc->subfunctions);
  free(bc);
}
