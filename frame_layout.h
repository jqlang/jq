#ifndef FRAME_LAYOUT_H
#include "forkable_stack.h"
#include "bytecode.h"

struct closure {
  struct bytecode* bc;
  stack_idx env;
};

struct continuation {
  struct bytecode* bc;
  stack_idx env;
  uint16_t* retaddr;
};

typedef union frame_elem {
  FORKABLE_STACK_HEADER;
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


static frame_ptr frame_current(struct forkable_stack* stk) {
  frame_ptr fp = forkable_stack_peek(stk);
  if (forkable_stack_peek_next(stk, fp)) {
    struct bytecode* bc = frame_self(forkable_stack_peek_next(stk, fp))->bc;
    assert(frame_self(fp)->retaddr >= bc->code && frame_self(fp)->retaddr < bc->code + bc->codelen);
  } else {
    assert(frame_self(fp)->retaddr == 0);
  }
  return fp;
}

static struct bytecode* frame_current_bytecode(struct forkable_stack* stk) {
  return frame_self(frame_current(stk))->bc;
}
static uint16_t** frame_current_retaddr(struct forkable_stack* stk) {
  return &frame_self(frame_current(stk))->retaddr;
}

static frame_ptr frame_get_parent(struct forkable_stack* stk, frame_ptr fr) {
  return forkable_stack_from_idx(stk, frame_self(fr)->env);
}

static frame_ptr frame_get_level(struct forkable_stack* stk, frame_ptr fr, int level) {
  for (int i=0; i<level; i++) {
    fr = frame_get_parent(stk, fr);
  }
  return fr;
}

static frame_ptr frame_push(struct forkable_stack* stk, struct closure cl, uint16_t* retaddr) {
  frame_ptr fp = forkable_stack_push(stk, frame_size(cl.bc));
  struct continuation* cc = frame_self(fp);
  cc->bc = cl.bc;
  cc->env = cl.env;
  cc->retaddr = retaddr;
  for (int i=0; i<cl.bc->nlocals; i++) {
    *frame_local_var(fp, i) = jv_invalid();
  }
  return fp;
}

static void frame_pop(struct forkable_stack* stk) {
  frame_ptr fp = frame_current(stk);
  if (forkable_stack_pop_will_free(stk)) {
    int nlocals = frame_self(fp)->bc->nlocals;
    for (int i=0; i<nlocals; i++) {
      jv_free(*frame_local_var(fp, i));
    }
  }
  forkable_stack_pop(stk);
}

#endif
