#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bytecode.h"
#include "jv_alloc.h"

// flags, length
#define NONE 0, 1
#define CONSTANT OP_HAS_CONSTANT, 2
#define VARIABLE (OP_HAS_VARIABLE | OP_HAS_BINDING), 3
#define GLOBAL (OP_HAS_CONSTANT | OP_HAS_VARIABLE | OP_HAS_BINDING | OP_IS_CALL_PSEUDO), 4
#define BRANCH OP_HAS_BRANCH, 2
#define CFUNC (OP_HAS_CFUNC | OP_HAS_BINDING), 3
#define UFUNC (OP_HAS_UFUNC | OP_HAS_BINDING | OP_IS_CALL_PSEUDO), 4
#define DEFINITION (OP_IS_CALL_PSEUDO | OP_HAS_BINDING), 0
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


int bytecode_operation_length(uint16_t* codeptr) {
  int length = opcode_describe(*codeptr)->length;
  if (*codeptr == CALL_JQ || *codeptr == TAIL_CALL_JQ) {
    length += codeptr[1] * 2;
  }
  return length;
}

static void dump_code(int indent, struct bytecode* bc) {
  int pc = 0;
  while (pc < bc->codelen) {
    printf("%*s", indent, "");
    dump_operation(bc, bc->code + pc);
    printf("\n");
    pc += bytecode_operation_length(bc->code + pc);
  }
}

static void symbol_table_free(struct symbol_table* syms) {
  jv_mem_free(syms->cfunctions);
  jv_free(syms->cfunc_names);
  jv_mem_free(syms);
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
    if (op->op == CALL_JQ || op->op == TAIL_CALL_JQ) {
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

void bytecode_free(struct bytecode* bc) {
  if (!bc)
    return;
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
