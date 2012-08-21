#include <assert.h>
#include <string.h>
#include "opcode.h"
#include "compile.h"


struct inst {
  struct inst* next;
  struct inst* prev;

  opcode op;
  
  union {
    uint16_t intval;
    struct inst* target;
    json_t* constant;
    char* symbol;
  } imm;

  struct inst* var_binding;
  int var_frame_idx;

  int bytecode_pos; // position just after this insn
};

static inst* inst_new(opcode op) {
  inst* i = malloc(sizeof(inst));
  i->next = i->prev = 0;
  i->op = op;
  i->bytecode_pos = -1;
  i->var_binding = 0;
  i->var_frame_idx = 0;
  return i;
}

static void inst_free(struct inst* i) {
  if (opcode_describe(i->op)->flags &
      (OP_HAS_SYMBOL | OP_HAS_VARIABLE)) {
    free(i->imm.symbol);
  }
  free(i);
}

static block inst_block(inst* i) {
  block b = {i,i};
  return b;
}

block gen_noop() {
  block b = {0,0};
  return b;
}

block gen_op_simple(opcode op) {
  assert(!(opcode_describe(op)->flags & OP_HAS_IMMEDIATE));
  return inst_block(inst_new(op));
}


block gen_op_const(opcode op, json_t* constant) {
  assert(opcode_describe(op)->flags & OP_HAS_CONSTANT);
  inst* i = inst_new(op);
  i->imm.constant = constant;
  return inst_block(i);
}

block gen_op_target(opcode op, block target) {
  assert(opcode_describe(op)->flags & OP_HAS_BRANCH);
  assert(target.last);
  inst* i = inst_new(op);
  i->imm.target = target.last;
  return inst_block(i);
}

block gen_op_targetlater(opcode op) {
  assert(opcode_describe(op)->flags & OP_HAS_BRANCH);
  inst* i = inst_new(op);
  i->imm.target = 0;
  return inst_block(i);
}
void inst_set_target(block b, block target) {
  assert(b.first);
  assert(b.first == b.last);
  assert(opcode_describe(b.first->op)->flags & OP_HAS_BRANCH);
  assert(target.last);
  b.first->imm.target = target.last;
}

block gen_op_var_unbound(opcode op, const char* name) {
  assert(opcode_describe(op)->flags & OP_HAS_VARIABLE);
  inst* i = inst_new(op);
  i->imm.symbol = strdup(name);
  return inst_block(i);
}

block gen_op_var_bound(opcode op, block binder) {
  assert(binder.first);
  assert(binder.first == binder.last);
  block b = gen_op_var_unbound(op, binder.first->imm.symbol);
  b.first->var_binding = binder.first;
  return b;
}

block gen_op_symbol(opcode op, const char* sym) {
  assert(opcode_describe(op)->flags & OP_HAS_SYMBOL);
  inst* i = inst_new(op);
  i->imm.symbol = strdup(sym);
  return inst_block(i);
}

static void inst_join(inst* a, inst* b) {
  assert(a && b);
  assert(!a->next);
  assert(!b->prev);
  a->next = b;
  b->prev = a;
}

void block_append(block* b, block b2) {
  if (b2.first) {
    if (b->last) {
      inst_join(b->last, b2.first);
    } else {
      b->first = b2.first;
    }
    b->last = b2.last;
  }
}

block block_join(block a, block b) {
  block c = a;
  block_append(&c, b);
  return c;
}

block block_bind(block binder, block body) {
  assert(binder.first);
  assert(binder.first == binder.last);
  assert(opcode_describe(binder.first->op)->flags & OP_HAS_VARIABLE);
  assert(binder.first->imm.symbol);
  assert(binder.first->var_binding == 0);

  binder.first->var_binding = binder.first;
  for (inst* i = body.first; i; i = i->next) {
    if (opcode_describe(i->op)->flags & OP_HAS_VARIABLE &&
        i->var_binding == 0 &&
        !strcmp(i->imm.symbol, binder.first->imm.symbol)) {
      // bind this variable
      i->var_binding = binder.first;
    }
  }
  return block_join(binder, body);
}


block gen_subexp(block a) {
  block c = gen_noop();
  block_append(&c, gen_op_simple(DUP));
  block_append(&c, a);
  block_append(&c, gen_op_simple(SWAP));
  return c;
}

block gen_both(block a, block b) {
  block c = gen_noop();
  block jump = gen_op_targetlater(JUMP);
  block fork = gen_op_targetlater(FORK);
  block_append(&c, fork);
  block_append(&c, a);
  block_append(&c, jump);
  inst_set_target(fork, c);
  block_append(&c, b);
  inst_set_target(jump, c);
  return c;
}


block gen_collect(block expr) {
  block c = gen_noop();
  block_append(&c, gen_op_simple(DUP));
  block_append(&c, gen_op_const(LOADK, json_array()));
  block array_var = block_bind(gen_op_var_unbound(STOREV, "collect"),
                               gen_noop());
  block_append(&c, array_var);

  block tail = {0};
  block_append(&tail, gen_op_simple(DUP));
  block_append(&tail, gen_op_var_bound(LOADV, array_var));
  block_append(&tail, gen_op_simple(SWAP));
  block_append(&tail, gen_op_simple(APPEND));
  block_append(&tail, gen_op_var_bound(STOREV, array_var));
  block_append(&tail, gen_op_simple(BACKTRACK));

  block_append(&c, gen_op_target(FORK, tail));
  block_append(&c, expr);
  block_append(&c, tail);

  block_append(&c, gen_op_var_bound(LOADV, array_var));

  return c;
}

block gen_else(block a, block b) {
  assert(0);
}


struct bytecode* block_compile(struct symbol_table* syms, block b) {
  inst* curr = b.first;
  int pos = 0;
  int var_frame_idx = 0;
  for (; curr; curr = curr->next) {
    if (!curr->next) assert(curr == b.last);
    pos += opcode_length(curr->op);
    curr->bytecode_pos = pos;
    if (opcode_describe(curr->op)->flags & OP_HAS_VARIABLE) {
      assert(curr->var_binding && "unbound variable");
      if (curr->var_binding == curr) {
        curr->var_frame_idx = var_frame_idx++;
      }
    }
  }
  struct bytecode* bc = malloc(sizeof(struct bytecode));
  bc->codelen = pos;
  uint16_t* code = malloc(sizeof(uint16_t) * bc->codelen);
  bc->code = code;
  int* stack_height = malloc(sizeof(int) * (bc->codelen + 1));
  for (int i = 0; i<bc->codelen + 1; i++) stack_height[i] = -1;
  pos = 0;
  json_t* constant_pool = json_array();
  int maxvar = -1;
  int curr_stack_height = 1;
  for (curr = b.first; curr; curr = curr->next) {
    const struct opcode_description* op = opcode_describe(curr->op);
    if (curr_stack_height < op->stack_in) {
      printf("Stack underflow at %04d\n", curr->bytecode_pos);
    }
    if (stack_height[curr->bytecode_pos] != -1 &&
        stack_height[curr->bytecode_pos] != curr_stack_height) {
      // FIXME: not sure this is right at all :(
      printf("Inconsistent stack heights at %04d %s\n", curr->bytecode_pos, op->name);
    }
    curr_stack_height -= op->stack_in;
    curr_stack_height += op->stack_out;
    code[pos++] = curr->op;
    int opflags = op->flags;
    if (opflags & OP_HAS_CONSTANT) {
      code[pos++] = json_array_size(constant_pool);
      json_array_append(constant_pool, curr->imm.constant);
    } else if (opflags & OP_HAS_VARIABLE) {
      uint16_t var = (uint16_t)curr->var_binding->var_frame_idx;
      code[pos++] = var;
      if (var > maxvar) maxvar = var;
    } else if (opflags & OP_HAS_BRANCH) {
      assert(curr->imm.target->bytecode_pos != -1);
      assert(curr->imm.target->bytecode_pos > pos); // only forward branches
      code[pos] = curr->imm.target->bytecode_pos - (pos + 1);
      stack_height[curr->imm.target->bytecode_pos] = curr_stack_height;
      pos++;
    } else if (opflags & OP_HAS_CFUNC) {
      assert(curr->imm.symbol);
      int found = 0;
      for (int i=0; i<syms->ncfunctions; i++) {
        if (!strcmp(curr->imm.symbol, syms->cfunctions[i].name)) {
          code[pos++] = i;
          found = 1;
          break;
        }
      }
      assert(found);
    } else if (opflags & OP_HAS_IMMEDIATE) {
      code[pos++] = curr->imm.intval;
    }
  }
  free(stack_height);
  bc->constants = constant_pool;
  bc->nlocals = maxvar + 2; // FIXME: frames of size zero?
  bc->nclosures = 0;
  bc->globals = syms;
  return bc;
}

void block_free(block b) {
  struct inst* next;
  for (struct inst* curr = b.first; curr; curr = next) {
    next = curr->next;
    inst_free(curr);
  }
}
