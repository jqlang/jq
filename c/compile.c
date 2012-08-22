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
  } imm;

  // Binding
  // An instruction requiring binding (for parameters/variables)
  // is in one of three states:
  //   bound_by = NULL  - Unbound free variable
  //   bound_by = self  - This instruction binds a variable
  //   bound_by = other - Uses variable bound by other instruction
  // The immediate field is generally not meaningful until instructions
  // are bound, and even then only for instructions which bind.
  struct inst* bound_by;
  char* symbol;
  block subfn;

  // This instruction is compiled as part of which function?
  // (only used during block_compile)
  struct bytecode* compiled;

  int bytecode_pos; // position just after this insn
};

static inst* inst_new(opcode op) {
  inst* i = malloc(sizeof(inst));
  i->next = i->prev = 0;
  i->op = op;
  i->bytecode_pos = -1;
  i->bound_by = 0;
  i->symbol = 0;
  i->subfn = gen_noop();
  return i;
}

static void inst_free(struct inst* i) {
  free(i->symbol);
  if (opcode_describe(i->op)->flags & OP_HAS_BLOCK) {
    block_free(i->subfn);
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
  assert(opcode_describe(op)->length == 1);
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
  i->symbol = strdup(name);
  return inst_block(i);
}

block gen_op_var_bound(opcode op, block binder) {
  assert(opcode_describe(op)->flags & OP_HAS_VARIABLE);
  assert(binder.first);
  assert(binder.first == binder.last);
  block b = gen_op_var_unbound(op, binder.first->symbol);
  b.first->bound_by = binder.first;
  return b;
}

block gen_op_symbol(opcode op, const char* sym) {
  assert(opcode_describe(op)->flags & OP_HAS_SYMBOL);
  inst* i = inst_new(op);
  i->symbol = strdup(sym);
  return inst_block(i);
}

block gen_op_block_defn(opcode op, const char* name, block block) {
  assert(opcode_describe(op)->flags & OP_IS_CALL_PSEUDO);
  assert(opcode_describe(op)->flags & OP_HAS_BLOCK);
  inst* i = inst_new(op);
  i->subfn = block;
  i->symbol = strdup(name);
  return inst_block(i);
}

block gen_op_block_unbound(opcode op, const char* name) {
  assert(opcode_describe(op)->flags & OP_IS_CALL_PSEUDO);
  inst* i = inst_new(op);
  i->symbol = strdup(name);
  return inst_block(i);
}


block gen_op_call(opcode op, block arglist) {
  assert(opcode_describe(op)->flags & OP_HAS_VARIABLE_LENGTH_ARGLIST);
  inst* i = inst_new(op);
  int nargs = 0;
  for (inst* curr = arglist.first; curr; curr = curr->next) {
    assert(opcode_describe(curr->op)->flags & OP_IS_CALL_PSEUDO);
    nargs++;
  }
  assert(nargs < 100); //FIXME
  i->imm.intval = nargs;
  return block_join(inst_block(i), arglist);
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

static void block_bind_subblock(block binder, block body, int bindflags) {
  assert(binder.first);
  assert(binder.first == binder.last);
  assert((opcode_describe(binder.first->op)->flags & bindflags) == bindflags);
  assert(binder.first->symbol);
  assert(binder.first->bound_by == 0 || binder.first->bound_by == binder.first);

  binder.first->bound_by = binder.first;
  for (inst* i = body.first; i; i = i->next) {
    int flags = opcode_describe(i->op)->flags;
    if ((flags & bindflags) == bindflags &&
        i->bound_by == 0 &&
        !strcmp(i->symbol, binder.first->symbol)) {
      // bind this instruction
      i->bound_by = binder.first;
    }
    if (flags & OP_HAS_BLOCK) {
      block_bind_subblock(binder, i->subfn, bindflags);
    }
  }
}

block block_bind(block binder, block body, int bindflags) {
  bindflags |= OP_HAS_BINDING;
  block_bind_subblock(binder, body, bindflags);
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
                               gen_noop(), OP_HAS_VARIABLE);
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

static uint16_t nesting_level(struct bytecode* bc, inst* target) {
  uint16_t level = 0;
  assert(bc && target->compiled);
  while (bc && target->compiled != bc) {
    level++;
    bc = bc->parent;
  }
  assert(bc && bc == target->compiled);
  return level;
}

static void compile(struct bytecode* bc, block b) {
  int pos = 0;
  int var_frame_idx = 0;
  bc->nsubfunctions = 0;
  for (inst* curr = b.first; curr; curr = curr->next) {
    if (!curr->next) assert(curr == b.last);
    pos += opcode_length(curr->op);
    curr->bytecode_pos = pos;
    curr->compiled = bc;

    int opflags = opcode_describe(curr->op)->flags;
    if (opflags & OP_HAS_BINDING) {
      assert(curr->bound_by && "unbound term");
    }
    if ((opflags & OP_HAS_VARIABLE) &&
        curr->bound_by == curr) {
      curr->imm.intval = var_frame_idx++;
    }
    if (opflags & OP_HAS_BLOCK) {
      assert(curr->bound_by == curr);
      curr->imm.intval = bc->nsubfunctions++;
    }
  }
  if (bc->nsubfunctions) {
    bc->subfunctions = malloc(sizeof(struct bytecode*) * bc->nsubfunctions);
    for (inst* curr = b.first; curr; curr = curr->next) {
      if (!(opcode_describe(curr->op)->flags & OP_HAS_BLOCK))
        continue;
      struct bytecode* subfn = malloc(sizeof(struct bytecode));
      bc->subfunctions[curr->imm.intval] = subfn;
      subfn->globals = bc->globals;
      subfn->parent = bc;
      compile(subfn, curr->subfn);
    }
  } else {
    bc->subfunctions = 0;
  }
  bc->codelen = pos;
  uint16_t* code = malloc(sizeof(uint16_t) * bc->codelen);
  bc->code = code;
  pos = 0;
  json_t* constant_pool = json_array();
  int maxvar = -1;
  for (inst* curr = b.first; curr; curr = curr->next) {
    if (curr->op == CLOSURE_CREATE) {
      // CLOSURE_CREATE opcodes define closures for use later in the
      // codestream. They generate no code.

      // FIXME: make the above true :)
      code[pos++] = DUP;
      code[pos++] = POP;
      continue;
    }
    const struct opcode_description* op = opcode_describe(curr->op);
    code[pos++] = curr->op;
    int opflags = op->flags;
    assert(!(op->flags & OP_IS_CALL_PSEUDO));
    if (opflags & OP_HAS_VARIABLE_LENGTH_ARGLIST) {
      int nargs = curr->imm.intval;
      assert(nargs > 0);
      code[pos++] = (uint16_t)nargs;
      for (int i=0; i<nargs; i++) {
        curr = curr->next;
        assert(curr && opcode_describe(curr->op)->flags & OP_IS_CALL_PSEUDO);
        code[pos++] = nesting_level(bc, curr->bound_by);
        switch (curr->bound_by->op) {
        default: assert(0 && "Unknown type of argument");
        case CLOSURE_CREATE:
          code[pos++] = curr->bound_by->imm.intval | ARG_NEWCLOSURE;
          break;
        }
      }
    } else if (opflags & OP_HAS_CONSTANT) {
      code[pos++] = json_array_size(constant_pool);
      json_array_append(constant_pool, curr->imm.constant);
    } else if (opflags & OP_HAS_VARIABLE) {
      code[pos++] = nesting_level(bc, curr->bound_by);
      uint16_t var = (uint16_t)curr->bound_by->imm.intval;
      code[pos++] = var;
      if (var > maxvar) maxvar = var;
    } else if (opflags & OP_HAS_BRANCH) {
      assert(curr->imm.target->bytecode_pos != -1);
      assert(curr->imm.target->bytecode_pos > pos); // only forward branches
      code[pos] = curr->imm.target->bytecode_pos - (pos + 1);
      pos++;
    } else if (opflags & OP_HAS_CFUNC) {
      assert(curr->symbol);
      int found = 0;
      for (int i=0; i<bc->globals->ncfunctions; i++) {
        if (!strcmp(curr->symbol, bc->globals->cfunctions[i].name)) {
          code[pos++] = i;
          found = 1;
          break;
        }
      }
      assert(found);
    } else if (op->length > 1) {
      assert(0 && "codegen not implemented for this operation");
    }
  }
  bc->constants = constant_pool;
  bc->nlocals = maxvar + 2; // FIXME: frames of size zero?
  bc->nclosures = 0;
}

struct bytecode* block_compile(struct symbol_table* syms, block b) {
  struct bytecode* bc = malloc(sizeof(struct bytecode));
  bc->parent = 0;
  bc->globals = syms;
  compile(bc, b);
  return bc;
}

void block_free(block b) {
  struct inst* next;
  for (struct inst* curr = b.first; curr; curr = next) {
    next = curr->next;
    inst_free(curr);
  }
}
