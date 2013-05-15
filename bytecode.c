#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bytecode.h"
#include "opcode.h"
#include "jv_alloc.h"

static int bytecode_operation_length(uint16_t* codeptr) {
  int length = opcode_describe(*codeptr)->length;
  if (*codeptr == CALL_JQ) {
    length += codeptr[1] * 2;
  }
  return length;
}

void dump_disassembly(int indent, struct bytecode* bc) {
  if (bc->nclosures > 0) {
    printf("%*s[params: ", indent, "");
    jv params = jv_object_get(jv_copy(bc->debuginfo), jv_string("params"));
    for (int i=0; i<bc->nclosures; i++) {
      if (i) printf(", ");
      jv name = jv_array_get(jv_copy(params), i);
      printf("%s", jv_string_value(name));
      jv_free(name);
    }
    jv_free(params);
    printf("]\n");
  }
  dump_code(indent, bc);
  for (int i=0; i<bc->nsubfunctions; i++) {
    struct bytecode* subfn = bc->subfunctions[i];
    jv name = jv_object_get(jv_copy(subfn->debuginfo), jv_string("name"));
    printf("%*s%s:%d:\n", indent, "", jv_string_value(name), i);
    jv_free(name);
    dump_disassembly(indent+2, subfn);
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

static struct bytecode* getlevel(struct bytecode* bc, int level) {
  while (level > 0) {
    bc = bc->parent;
    level--;
  }
  return bc;
}

void dump_operation(struct bytecode* bc, uint16_t* codeptr) {
  int pc = codeptr - bc->code;
  printf("%04d ", pc);
  const struct opcode_description* op = opcode_describe(bc->code[pc++]);
  printf("%s", op->name);
  if (op->length > 1) {
    uint16_t imm = bc->code[pc++];
    if (op->op == CALL_JQ) {
      for (int i=0; i<imm+1; i++) {
        uint16_t level = bc->code[pc++];
        uint16_t idx = bc->code[pc++];
        jv name;
        if (idx & ARG_NEWCLOSURE) {
          idx &= ~ARG_NEWCLOSURE;
          name = jv_object_get(jv_copy(getlevel(bc,level)->subfunctions[idx]->debuginfo), 
                               jv_string("name"));
        } else {
          name = jv_array_get(jv_object_get(jv_copy(getlevel(bc,level)->debuginfo), 
                                            jv_string("params")), idx);
        }
        printf(" %s:%d",
               jv_string_value(name),
               idx);
        jv_free(name);
        if (level) {
          printf("^%d", level);
        }
      }
    } else if (op->op == CALL_BUILTIN) {
      int func = bc->code[pc++];
      jv name = jv_array_get(jv_copy(bc->globals->cfunc_names), func);
      printf(" %s", jv_string_value(name));
      jv_free(name);
    } else if (op->flags & OP_HAS_BRANCH) {
      printf(" %04d", pc + imm);
    } else if (op->flags & OP_HAS_CONSTANT) {
      printf(" ");
      jv_dump(jv_array_get(jv_copy(bc->constants), imm), 0);
    } else if (op->flags & OP_HAS_VARIABLE) {
      uint16_t v = bc->code[pc++];
      jv name = jv_array_get(jv_object_get(jv_copy(getlevel(bc,imm)->debuginfo), jv_string("locals")), v);
      printf(" $%s:%d", 
             jv_string_value(name), 
             v);
      jv_free(name);
      if (imm) {
        printf("^%d", imm);
      }
    } else {
      printf(" %d", imm);
    }
  }  
}

void symbol_table_free(struct symbol_table* syms) {
  jv_mem_free(syms->cfunctions);
  jv_free(syms->cfunc_names);
  jv_mem_free(syms);
}

void bytecode_free(struct bytecode* bc) {
  jv_mem_free(bc->code);
  jv_free(bc->constants);
  for (int i=0; i<bc->nsubfunctions; i++)
    bytecode_free(bc->subfunctions[i]);
  if (!bc->parent)
    symbol_table_free(bc->globals);
  jv_mem_free(bc->subfunctions);
  jv_free(bc->debuginfo);
  jv_mem_free(bc);
}
