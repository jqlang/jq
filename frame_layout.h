#ifndef FRAME_LAYOUT_H
#include "newstack.h"
#include "bytecode.h"

struct closure {
  struct bytecode* bc;
  stack_ptr env;
};

struct continuation {
  struct bytecode* bc;
  stack_ptr env;
  stack_ptr retdata;
  uint16_t* retaddr;
};

typedef union frame_elem {
  struct continuation cont;
  struct closure closure;
  jv jsonval;
} *frame_ptr;

/*
 * Frame layout
 * fr[0] - FORKABLE_STACK_HEADER (next pointer)
 * fr[1] - self (used to store return addresses, etc)
 * fr[2...nclosures+2] - closure params
 * fr[nclosures+2..nclosures+nlocals+2] - local variables
 */

static int frame_size(struct bytecode* bc) {
  return sizeof(union frame_elem) * (bc->nclosures + bc->nlocals + 2);
}

static struct continuation* frame_self(frame_ptr fr) {
  return &fr[1].cont;
}

static struct closure* frame_closure_arg(frame_ptr fr, int closure) {
  assert(closure >= 0);
  assert(closure < frame_self(fr)->bc->nclosures);
  return &fr[2+closure].closure;
}

static jv* frame_local_var(frame_ptr fr, int var) {
  assert(var >= 0);
  assert(var < frame_self(fr)->bc->nlocals);
  return &fr[2 + frame_self(fr)->bc->nclosures + var].jsonval;
}


static frame_ptr frame_current(struct stack* stk, stack_ptr idx) {
  frame_ptr fp = stack_block(stk, idx);

  stack_ptr next = *stack_block_next(stk, idx);
  if (next) {
    frame_ptr fpnext = stack_block(stk, next);
    struct bytecode* bc = frame_self(fpnext)->bc;
    assert(frame_self(fp)->retaddr >= bc->code && frame_self(fp)->retaddr < bc->code + bc->codelen);
  } else {
    assert(frame_self(fp)->retaddr == 0);
  }
  return fp;
}

static struct bytecode* frame_current_bytecode(struct stack* stk, stack_ptr curr) {
  return frame_self(frame_current(stk, curr))->bc;
}
static uint16_t** frame_current_retaddr(struct stack* stk, stack_ptr curr) {
  return &frame_self(frame_current(stk, curr))->retaddr;
}

static stack_ptr frame_get_parent(struct stack* stk, stack_ptr fr) {
  return frame_self(stack_block(stk, fr))->env;
}

static stack_ptr frame_get_level(struct stack* stk, stack_ptr fr, int level) {
  for (int i=0; i<level; i++) {
    fr = frame_get_parent(stk, fr);
  }
  return fr;
}

static stack_ptr frame_push(struct stack* stk, stack_ptr curr, struct closure cl, uint16_t* retaddr, stack_ptr datastk) {
  stack_ptr fpidx = stack_push_block(stk, curr, frame_size(cl.bc));
  frame_ptr fp = stack_block(stk, fpidx);
  struct continuation* cc = frame_self(fp);
  cc->bc = cl.bc;
  cc->env = cl.env;
  cc->retdata = datastk;
  cc->retaddr = retaddr;
  for (int i=0; i<cl.bc->nlocals; i++) {
    *frame_local_var(fp, i) = jv_invalid();
  }
  return fpidx;
}

static stack_ptr frame_pop(struct stack* stk, stack_ptr curr) {
  frame_ptr fp = frame_current(stk, curr);
  if (stack_pop_will_free(stk, curr)) {
    int nlocals = frame_self(fp)->bc->nlocals;
    for (int i=0; i<nlocals; i++) {
      jv_free(*frame_local_var(fp, i));
    }
  }
  return stack_pop_block(stk, curr, frame_size(frame_self(fp)->bc));
}

#endif
