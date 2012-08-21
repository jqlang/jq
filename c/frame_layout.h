#ifndef FRAME_LAYOUT_H
#include "forkable_stack.h"
#include "bytecode.h"
#include <jansson.h>

struct closure {
  struct bytecode* bc;
  uint16_t* pc;
};

typedef union frame_elem {
  FORKABLE_STACK_HEADER;
  struct closure closure;
  json_t* jsonval;
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

static struct closure* frame_self(frame_ptr fr) {
  return &fr[1].closure;
}

static struct closure* frame_closure_arg(frame_ptr fr, int closure) {
  assert(closure >= 0);
  assert(closure < frame_self(fr)->bc->nclosures);
  return &fr[2+closure].closure;
}

static json_t** frame_local_var(frame_ptr fr, int var) {
  assert(var >= 0);
  assert(var < frame_self(fr)->bc->nlocals);
  return &fr[2 + frame_self(fr)->bc->nclosures + var].jsonval;
}


static frame_ptr frame_current(struct forkable_stack* stk) {
  frame_ptr fp = forkable_stack_peek(stk);
  struct bytecode* bc = frame_self(fp)->bc;
  assert(frame_self(fp)->pc >= bc->code && frame_self(fp)->pc < bc->code + bc->codelen);
  return fp;
}

static struct bytecode* frame_current_bytecode(struct forkable_stack* stk) {
  return frame_self(frame_current(stk))->bc;
}
static uint16_t** frame_current_pc(struct forkable_stack* stk) {
  return &frame_self(frame_current(stk))->pc;
}




static void frame_push(struct forkable_stack* stk, struct bytecode* bc) {
  frame_ptr fp = forkable_stack_push(stk, frame_size(bc));
  frame_self(fp)->bc = bc;
  frame_self(fp)->pc = bc->code;
}

static void frame_push_backtrack(struct forkable_stack* stk, 
                                      struct bytecode* bc, uint16_t* pc) {
  frame_ptr fp = forkable_stack_push(stk, sizeof(union frame_elem) * 2);
  frame_self(fp)->bc = bc;
  frame_self(fp)->pc = pc;
}


static void frame_pop(struct forkable_stack* stk) {
  forkable_stack_pop(stk);
}

#endif
