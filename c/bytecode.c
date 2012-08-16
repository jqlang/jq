#include <stdio.h>
#include <stdint.h>
#include <jansson.h>
#include "bytecode.h"
#include "opcode.h"

void dump_disassembly(struct bytecode* bc) {
  int pc = 0;
  while (pc < bc->codelen) {
    dump_operation(bc, bc->code + pc);
    printf("\n");
    pc += opcode_length(bc->code[pc]);
  }
}

void dump_operation(struct bytecode* bc, uint16_t* codeptr) {
  int pc = codeptr - bc->code;
  printf("%04d ", pc);
  const struct opcode_description* op = opcode_describe(bc->code[pc++]);
  printf("%s", op->name);
  if (op->flags & OP_HAS_IMMEDIATE) {
    uint16_t imm = bc->code[pc++];
    printf(" ");
    if (op->flags & OP_HAS_BRANCH) {
      printf("%04d", pc + imm);
    } else if (op->flags & OP_HAS_CONSTANT) {
      json_dumpf(json_array_get(bc->constants, imm),
                 stdout, JSON_ENCODE_ANY);
    } else if (op->flags & OP_HAS_VARIABLE) {
      printf("v%d", imm);
    } else {
      printf("%d", imm);
    }
  }  
}
