#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "opcode.h"
#include "compile.h"
#include "bytecode.h"
#include "locfile.h"

struct inst {
  struct inst* next;
  struct inst* prev;

  opcode op;
  
  union {
    uint16_t intval;
    struct inst* target;
    jv constant;
    struct cfunction* cfunc;
  } imm;

  location source;

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
  block arglist;

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
  i->arglist = gen_noop();
  i->source = UNKNOWN_LOCATION;
  return i;
}

static void inst_free(struct inst* i) {
  free(i->symbol);
  block_free(i->subfn);
  block_free(i->arglist);
  if (opcode_describe(i->op)->flags & OP_HAS_CONSTANT) {
    jv_free(i->imm.constant);
  }
  free(i);
}

static block inst_block(inst* i) {
  block b = {i,i};
  return b;
}

static int block_is_single(block b) {
  return b.first && b.first == b.last;
}

static inst* block_take(block* b) {
  if (b->first == 0) return 0;
  inst* i = b->first;
  if (i->next) {
    i->next->prev = 0;
    b->first = i->next;
    i->next = 0;
  } else {
    b->first = 0;
    b->last = 0;
  }
  return i;
}

block gen_location(location loc, block b) {
  for (inst* i = b.first; i; i = i->next) {
    if (i->source.start == UNKNOWN_LOCATION.start &&
        i->source.end == UNKNOWN_LOCATION.end) {
      i->source = loc;
    }
  }
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


block gen_op_const(opcode op, jv constant) {
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
  assert(block_is_single(b));
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
  assert(block_is_single(binder));
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

block gen_op_block_unbound(opcode op, const char* name) {
  assert(opcode_describe(op)->flags & OP_IS_CALL_PSEUDO);
  inst* i = inst_new(op);
  i->symbol = strdup(name);
  return inst_block(i);
}

block gen_op_block_bound(opcode op, block binder) {
  assert(block_is_single(binder));
  block b = gen_op_block_unbound(op, binder.first->symbol);
  b.first->bound_by = binder.first;
  return b;
}

static void inst_join(inst* a, inst* b) {
  assert(a && b);
  assert(!a->next);
  assert(!b->prev);
  a->next = b;
  b->prev = a;
}

static void block_insert_after(inst* i, block b) {
  if (b.first) {
    assert(b.last);
    if (i->next) {
      inst* j = i->next;
      i->next = 0;
      j->prev = 0;
      inst_join(b.last, j);
    }
    inst_join(i, b.first);
  }
}

static void block_insert_before(inst* i, block b) {
  if (b.first) {
    assert(b.last);
    if (i->prev) {
      inst* j = i->prev;
      i->prev = 0;
      j->next = 0;
      inst_join(j, b.first);
    }
    inst_join(b.last, i);
  }
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

int block_has_only_binders(block binders, int bindflags) {
  bindflags |= OP_HAS_BINDING;
  for (inst* curr = binders.first; curr; curr = curr->next) {
    if ((opcode_describe(curr->op)->flags & bindflags) != bindflags) {
      return 0;
    }
  }
  return 1;
}

static void block_bind_subblock(block binder, block body, int bindflags) {
  assert(block_is_single(binder));
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
      // binding recurses into closures
      block_bind_subblock(binder, i->subfn, bindflags);
    }
    if (flags & OP_HAS_VARIABLE_LENGTH_ARGLIST) {
      // binding recurses into argument list
      block_bind_subblock(binder, i->arglist, bindflags);
    }
  }
}

block block_bind(block binder, block body, int bindflags) {
  assert(block_has_only_binders(binder, bindflags));
  bindflags |= OP_HAS_BINDING;
  for (inst* curr = binder.first; curr; curr = curr->next) {
    block_bind_subblock(inst_block(curr), body, bindflags);
  }
  return block_join(binder, body);
}


block gen_function(const char* name, block body) {
  inst* i = inst_new(CLOSURE_CREATE);
  i->subfn = body;
  i->symbol = strdup(name);
  block b = inst_block(i);
  block_bind_subblock(b, b, OP_IS_CALL_PSEUDO | OP_HAS_BINDING);
  return b;
}

block gen_lambda(block body) {
  return gen_function("@lambda", body);
}

block gen_call(const char* name, block args) {
  inst* i = inst_new(CALL_JQ);
  i->arglist = BLOCK(gen_op_block_unbound(CLOSURE_REF, name), args);
  return BLOCK(inst_block(i), inst_block(inst_new(CALLSEQ_END)));
}



block gen_subexp(block a) {
  return BLOCK(gen_op_simple(DUP), a, gen_op_simple(SWAP));
}

block gen_both(block a, block b) {
  block jump = gen_op_targetlater(JUMP);
  block fork = gen_op_target(FORK, jump);
  block c = BLOCK(fork, a, jump, b);
  inst_set_target(jump, c);
  return c;
}


block gen_collect(block expr) {
  block array_var = block_bind(gen_op_var_unbound(STOREV, "collect"),
                               gen_noop(), OP_HAS_VARIABLE);
  block c = BLOCK(gen_op_simple(DUP), gen_op_const(LOADK, jv_array()), array_var);

  block tail = BLOCK(gen_op_simple(DUP),
                     gen_op_var_bound(LOADV, array_var),
                     gen_op_simple(SWAP),
                     gen_op_simple(APPEND),
                     gen_op_var_bound(STOREV, array_var),
                     gen_op_simple(BACKTRACK));

  return BLOCK(c,
               gen_op_target(FORK, tail),
               expr, 
               tail,
               gen_op_var_bound(LOADV, array_var));
}

block gen_assign(block expr) {
  block result_var = block_bind(gen_op_var_unbound(STOREV, "result"),
                                gen_noop(), OP_HAS_VARIABLE);

  block loop = BLOCK(gen_op_simple(DUP),
                     expr,
                     gen_op_var_bound(ASSIGN, result_var),
                     gen_op_simple(BACKTRACK));

  return BLOCK(gen_op_simple(DUP),
               result_var,
               gen_op_target(FORK, loop),
               loop,
               gen_op_var_bound(LOADV, result_var));
}

block gen_definedor(block a, block b) {
  // var found := false
  block found_var = block_bind(gen_op_var_unbound(STOREV, "found"),
                               gen_noop(), OP_HAS_VARIABLE);
  block init = BLOCK(gen_op_simple(DUP), gen_op_const(LOADK, jv_false()), found_var);

  // if found, backtrack. Otherwise execute b
  block backtrack = gen_op_simple(BACKTRACK);
  block tail = BLOCK(gen_op_simple(DUP),
                     gen_op_var_bound(LOADV, found_var),
                     gen_op_target(JUMP_F, backtrack),
                     backtrack,
                     gen_op_simple(POP),
                     b);

  // try again
  block if_notfound = gen_op_simple(BACKTRACK);

  // found := true, produce result
  block if_found = BLOCK(gen_op_simple(DUP),
                         gen_op_const(LOADK, jv_true()),
                         gen_op_var_bound(STOREV, found_var),
                         gen_op_target(JUMP, tail));

  return BLOCK(init,
               gen_op_target(FORK, if_notfound),
               a,
               gen_op_target(JUMP_F, if_found),
               if_found,
               if_notfound,
               tail);
}

block gen_condbranch(block iftrue, block iffalse) {
  iftrue = BLOCK(iftrue, gen_op_target(JUMP, iffalse));
  return BLOCK(gen_op_target(JUMP_F, iftrue), iftrue, iffalse);
}

block gen_and(block a, block b) {
  // a and b = if a then (if b then true else false) else false
  return BLOCK(gen_op_simple(DUP), a, 
               gen_condbranch(BLOCK(gen_op_simple(POP),
                                    b,
                                    gen_condbranch(gen_op_const(LOADK, jv_true()),
                                                   gen_op_const(LOADK, jv_false()))),
                              BLOCK(gen_op_simple(POP), gen_op_const(LOADK, jv_false()))));
}

block gen_or(block a, block b) {
  // a or b = if a then true else (if b then true else false)
  return BLOCK(gen_op_simple(DUP), a,
               gen_condbranch(BLOCK(gen_op_simple(POP), gen_op_const(LOADK, jv_true())),
                              BLOCK(gen_op_simple(POP),
                                    b,
                                    gen_condbranch(gen_op_const(LOADK, jv_true()),
                                                   gen_op_const(LOADK, jv_false())))));
}

block gen_cond(block cond, block iftrue, block iffalse) {
  return BLOCK(gen_op_simple(DUP), cond, 
               gen_condbranch(BLOCK(gen_op_simple(POP), iftrue),
                              BLOCK(gen_op_simple(POP), iffalse)));
}

block gen_cbinding(struct symbol_table* t, block code) {
  for (int cfunc=0; cfunc<t->ncfunctions; cfunc++) {
    inst* i = inst_new(CLOSURE_CREATE_C);
    i->imm.cfunc = &t->cfunctions[cfunc];
    i->symbol = strdup(i->imm.cfunc->name);
    code = block_bind(inst_block(i), code, OP_IS_CALL_PSEUDO);
  }
  return code;
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

static int count_cfunctions(block b) {
  int n = 0;
  for (inst* i = b.first; i; i = i->next) {
    if (i->op == CLOSURE_CREATE_C) n++;
    if (opcode_describe(i->op)->flags & OP_HAS_BLOCK)
      n += count_cfunctions(i->subfn);
  }
  return n;
}


// Expands call instructions into a calling sequence
// Checking for argument count compatibility happens later
static block expand_call_arglist(block b) {
  block ret = gen_noop();
  for (inst* curr; (curr = block_take(&b));) {
    if (opcode_describe(curr->op)->flags & OP_HAS_VARIABLE_LENGTH_ARGLIST) {
      assert(curr->op == CALL_JQ);
      inst* seq_end = block_take(&b);
      assert(seq_end && seq_end->op == CALLSEQ_END);
      // We expand the argument list as a series of instructions
      block arglist = curr->arglist;
      curr->arglist = gen_noop();
      assert(arglist.first && "zeroth argument (function to call) must be present");
      inst* function = arglist.first->bound_by;
      assert(function); // FIXME better errors

      switch (function->op) {
      default: assert(0 && "Unknown parameter type"); break;
      case CLOSURE_CREATE: 
      case CLOSURE_PARAM: {
        block prelude = gen_noop();
        block callargs = gen_noop();
        int nargs = 0;
        for (inst* i; (i = block_take(&arglist));) {
          assert(opcode_describe(i->op)->flags & OP_IS_CALL_PSEUDO);
          block b = inst_block(i);
          switch (i->op) {
          default: assert(0 && "Unknown type of parameter"); break;
          case CLOSURE_REF:
            block_append(&callargs, b);
            break;
          case CLOSURE_CREATE:
            block_append(&prelude, b);
            block_append(&callargs, gen_op_block_bound(CLOSURE_REF, b));
            break;
          }
          nargs++;
        }

        assert(!arglist.first);
        assert(nargs > 0);
        curr->imm.intval = nargs - 1;
        ret = BLOCK(ret, prelude, inst_block(curr), callargs, inst_block(seq_end));
        break;
      }

      case CLOSURE_CREATE_C: {
        // Arguments to C functions not yet supported
        inst* cfunction_ref = block_take(&arglist);
        block prelude = gen_noop();
        int nargs = 1;
        for (inst* i; (i = block_take(&arglist)); ) {
          assert(i->op == CLOSURE_CREATE); // FIXME
          block body = i->subfn;
          i->subfn = gen_noop();
          inst_free(i);
          // arguments should be pushed in reverse order, prepend them to prelude
          prelude = BLOCK(gen_subexp(expand_call_arglist(body)), prelude);
          nargs++;
        }
        assert(curr->op == CALL_JQ);
        curr->op = CALL_BUILTIN;
        curr->imm.intval = nargs;
        assert(!curr->arglist.first);
        curr->arglist = inst_block(cfunction_ref);
        ret = BLOCK(ret, prelude, inst_block(curr), inst_block(seq_end));
        break;
      }
      }
    } else {
      ret = BLOCK(ret, inst_block(curr));
    }
  }
  return ret;
}

static int compile(struct locfile* locations, struct bytecode* bc, block b) {
  int errors = 0;
  int pos = 0;
  int var_frame_idx = 0;
  bc->nsubfunctions = 0;
  bc->nclosures = 0;
  b = expand_call_arglist(b);
  if (bc->parent) {
    // functions should end in a return
    b = BLOCK(b, gen_op_simple(RET));
  } else {
    // the toplevel should YIELD;BACKTRACK; when it finds an answer
    b = BLOCK(b, gen_op_simple(YIELD), gen_op_simple(BACKTRACK));
  }
  for (inst* curr = b.first; curr; curr = curr->next) {
    if (!curr->next) assert(curr == b.last);
    pos += opcode_length(curr->op);
    curr->bytecode_pos = pos;
    curr->compiled = bc;

    int opflags = opcode_describe(curr->op)->flags;
    if (opflags & OP_HAS_BINDING) {
      if (!curr->bound_by) {
        locfile_locate(locations, curr->source, "error: %s is not defined", curr->symbol);
        errors++;
      }
    }
    if ((opflags & OP_HAS_VARIABLE) &&
        curr->bound_by == curr) {
      curr->imm.intval = var_frame_idx++;
    }
    if (opflags & OP_HAS_BLOCK) {
      assert(curr->bound_by == curr);
      curr->imm.intval = bc->nsubfunctions++;
    }

    if (curr->op == CLOSURE_PARAM) {
      assert(curr->bound_by == curr);
      curr->imm.intval = bc->nclosures++;
    }
    if (curr->op == CLOSURE_CREATE_C) {
      assert(curr->bound_by == curr);
      int idx = bc->globals->ncfunctions++;
      bc->globals->cfunctions[idx] = *curr->imm.cfunc;
      curr->imm.intval = idx;
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
      errors += compile(locations, subfn, curr->subfn);
      curr->subfn = gen_noop();
    }
  } else {
    bc->subfunctions = 0;
  }
  bc->codelen = pos;
  uint16_t* code = malloc(sizeof(uint16_t) * bc->codelen);
  bc->code = code;
  pos = 0;
  jv constant_pool = jv_array();
  int maxvar = -1;
  if (!errors) for (inst* curr = b.first; curr; curr = curr->next) {
    const struct opcode_description* op = opcode_describe(curr->op);
    if (op->length == 0)
      continue;
    code[pos++] = curr->op;
    int opflags = op->flags;
    assert(!(op->flags & OP_IS_CALL_PSEUDO));
    if (curr->op == CALL_BUILTIN) {
      int nargs = curr->imm.intval;
      code[pos++] = (uint16_t)nargs;
      assert(block_is_single(curr->arglist));
      inst* cfunc = curr->arglist.first;
      assert(cfunc && cfunc->bound_by->op == CLOSURE_CREATE_C);
      //*opcode_rewrite = bc->globals->cfunctions[cfunc->bound_by->imm.intval].callop;
      code[pos++] = cfunc->bound_by->imm.intval;
      // FIXME arg errors
      assert(nargs == bc->globals->cfunctions[cfunc->bound_by->imm.intval].nargs);
    } else if (opflags & OP_HAS_VARIABLE_LENGTH_ARGLIST) {
      assert(curr->op == CALL_JQ);
      int nargs = curr->imm.intval;
      assert(nargs >= 0 && nargs < 100); //FIXME
      code[pos++] = (uint16_t)nargs;
      curr = curr->next;
      assert(curr && opcode_describe(curr->op)->flags & OP_IS_CALL_PSEUDO);
      assert(curr->bound_by->op == CLOSURE_CREATE ||
             curr->bound_by->op == CLOSURE_PARAM);
      code[pos++] = nesting_level(bc, curr->bound_by);
      if (curr->bound_by->op == CLOSURE_CREATE) {
        code[pos++] = curr->bound_by->imm.intval | ARG_NEWCLOSURE;
        inst* i = curr->bound_by;
        // FIXME arg errors
        assert(nargs == i->compiled->subfunctions[i->imm.intval]->nclosures);
      } else {
        code[pos++] = curr->bound_by->imm.intval;
        // FIXME arg errors
        assert(nargs == 0);
      }
      for (int i=0; i<nargs; i++) {
        curr = curr->next;
        assert(curr && opcode_describe(curr->op)->flags & OP_IS_CALL_PSEUDO);
        assert(curr->op == CLOSURE_REF);
        code[pos++] = nesting_level(bc, curr->bound_by);
        assert(curr->bound_by->op == CLOSURE_CREATE);
        code[pos++] = curr->bound_by->imm.intval | ARG_NEWCLOSURE;
      }
    } else if (opflags & OP_HAS_CONSTANT) {
      code[pos++] = jv_array_length(jv_copy(constant_pool));
      constant_pool = jv_array_append(constant_pool, jv_copy(curr->imm.constant));
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
    } else if (op->length > 1) {
      assert(0 && "codegen not implemented for this operation");
    }
  }
  bc->constants = constant_pool;
  bc->nlocals = maxvar + 2; // FIXME: frames of size zero?
  block_free(b);
  return errors;
}

int block_compile(block b, struct locfile* locations, struct bytecode** out) {
  struct bytecode* bc = malloc(sizeof(struct bytecode));
  bc->parent = 0;
  bc->globals = malloc(sizeof(struct symbol_table));
  int ncfunc = count_cfunctions(b);
  bc->globals->ncfunctions = 0;
  bc->globals->cfunctions = malloc(sizeof(struct cfunction) * ncfunc);
  int nerrors = compile(locations, bc, b);
  assert(bc->globals->ncfunctions == ncfunc);
  if (nerrors > 0) {
    bytecode_free(bc);
    *out = 0;
  } else {
    *out = bc;
  }
  return nerrors;
}

void block_free(block b) {
  struct inst* next;
  for (struct inst* curr = b.first; curr; curr = next) {
    next = curr->next;
    inst_free(curr);
  }
}
