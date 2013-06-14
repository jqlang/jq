#ifndef FRAME_LAYOUT_H
#include "newstack.h"
#include "bytecode.h"

struct closure {
  struct bytecode* bc;
  stack_ptr env;
};

union frame_entry {
  struct closure closure;
  jv localvar;
};

struct frame {
  struct bytecode* bc;
  stack_ptr env;
  stack_ptr retdata;
  uint16_t* retaddr;
  /* bc->nclosures closures followed by bc->nlocals local variables */
  union frame_entry entries[0]; 
};

static int frame_size(struct bytecode* bc) {
  return sizeof(struct frame) + sizeof(union frame_entry) * (bc->nclosures + bc->nlocals);
}

static struct closure* frame_closure_arg(struct frame* fr, int closure) {
  assert(closure >= 0);
  assert(closure < fr->bc->nclosures);
  return &fr->entries[closure].closure;
}

static jv* frame_local_var(struct frame* fr, int var) {
  assert(var >= 0);
  assert(var < fr->bc->nlocals);
  return &fr->entries[fr->bc->nclosures + var].localvar;
}

static struct frame* frame_current(struct stack* stk, stack_ptr idx) {
  struct frame* fp = stack_block(stk, idx);

  stack_ptr next = *stack_block_next(stk, idx);
  if (next) {
    struct frame* fpnext = stack_block(stk, next);
    struct bytecode* bc = fpnext->bc;
    assert(fp->retaddr >= bc->code && fp->retaddr < bc->code + bc->codelen);
  } else {
    assert(fp->retaddr == 0);
  }
  return fp;
}

static stack_ptr frame_get_level(struct stack* stk, stack_ptr fr, int level) {
  for (int i=0; i<level; i++) {
    struct frame* fp = stack_block(stk, fr);
    fr = fp->env;
  }
  return fr;
}

static stack_ptr frame_push(struct stack* stk, stack_ptr curr, struct closure cl, uint16_t* retaddr, stack_ptr datastk) {
  stack_ptr fpidx = stack_push_block(stk, curr, frame_size(cl.bc));
  struct frame* fp = stack_block(stk, fpidx);
  fp->bc = cl.bc;
  fp->env = cl.env;
  fp->retdata = datastk;
  fp->retaddr = retaddr;
  for (int i=0; i<cl.bc->nlocals; i++) {
    *frame_local_var(fp, i) = jv_invalid();
  }
  return fpidx;
}

static stack_ptr frame_pop(struct stack* stk, stack_ptr curr) {
  struct frame* fp = frame_current(stk, curr);
  if (stack_pop_will_free(stk, curr)) {
    int nlocals = fp->bc->nlocals;
    for (int i=0; i<nlocals; i++) {
      jv_free(*frame_local_var(fp, i));
    }
  }
  return stack_pop_block(stk, curr, frame_size(fp->bc));
}

#endif
