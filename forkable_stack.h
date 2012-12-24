#ifndef FORKABLE_STACK_H
#define FORKABLE_STACK_H
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "jv_alloc.h"

struct forkable_stack_header {
  /* distance in bytes between this header and the header
     of the next object on the stack */
  int next_delta;
};

#define FORKABLE_STACK_HEADER struct forkable_stack_header fk_header_

struct forkable_stack {
  // Stack grows down from stk+length
  char* stk;

  // stk+length is just past end of allocated area
  // stk = jv_mem_alloc(length)
  int length;

  // stk+pos is header of top object, or stk+length if empty
  int pos;

  // everything after-or-including stk+savedlimit must be preserved
  int savedlimit;
};

static void forkable_stack_check(struct forkable_stack* s) {
  assert(s->stk);
  assert(s->length > 0);
  assert(s->pos >= 0 && s->pos <= s->length);
  assert(s->savedlimit >= 0 && s->savedlimit <= s->length);
}

static int forkable_stack_empty(struct forkable_stack* s) {
  return s->pos == s->length;
}

static void forkable_stack_init(struct forkable_stack* s, size_t sz) {
  s->stk = jv_mem_alloc(sz);
  s->length = sz;
  s->pos = s->length;
  s->savedlimit = s->length;
  forkable_stack_check(s);
}

static void forkable_stack_free(struct forkable_stack* s) {
  assert(forkable_stack_empty(s));
  assert(s->savedlimit == s->length);
  jv_mem_free(s->stk);
  s->stk = 0;
}

static void* forkable_stack_push(struct forkable_stack* s, size_t sz_size) {
  assert(sz_size > 0 && sz_size % sizeof(struct forkable_stack_header) == 0);
  int size = (int)sz_size;
  forkable_stack_check(s);
  int curr = s->pos < s->savedlimit ? s->pos : s->savedlimit;
  if (curr - size < 0) {
    int oldlen = s->length;
    s->length = (size + oldlen + 1024) * 2;
    s->stk = jv_mem_realloc(s->stk, s->length);
    int shift = s->length - oldlen;
    memmove(s->stk + shift, s->stk, oldlen);
    s->pos += shift;
    s->savedlimit += shift;
    curr += shift;
  }
  int newpos = curr - size;
  assert(newpos < s->pos);
  void* ret = (void*)(s->stk + newpos);
  ((struct forkable_stack_header*)ret)->next_delta = s->pos - newpos;
  s->pos = newpos;
  forkable_stack_check(s);
  return ret;
}

static void* forkable_stack_peek(struct forkable_stack* s) {
  assert(!forkable_stack_empty(s));
  forkable_stack_check(s);
  return (void*)(s->stk + s->pos);
}

static void* forkable_stack_peek_next(struct forkable_stack* s, void* top) {
  forkable_stack_check(s);
  struct forkable_stack_header* elem = top;
  int elempos = (char*)top - s->stk;
  assert(elempos >= 0 && elempos < s->length);
  assert(elempos + elem->next_delta <= s->length);
  if (elempos + elem->next_delta < s->length) {
    return (void*)(s->stk + elempos + elem->next_delta);
  } else {
    return 0;
  }
}

// Returns 1 if the next forkable_stack_pop will permanently remove an
// object from the stack (i.e. the top object was not saved with a fork)
static int forkable_stack_pop_will_free(struct forkable_stack* s) {
  return s->pos < s->savedlimit;
}

static void forkable_stack_pop(struct forkable_stack* s) {
  forkable_stack_check(s);
  struct forkable_stack_header* elem = forkable_stack_peek(s);
  int reclaim_upto = s->pos + elem->next_delta < s->savedlimit ? 
    s->pos + elem->next_delta : s->savedlimit;
  assert(reclaim_upto <= s->length);
  int reclaimed = reclaim_upto > s->pos ? reclaim_upto - s->pos : 0;
  assert(0 <= reclaimed && s->pos + reclaimed <= s->length);
  // s->pos <= i < reclaim_upto means that position i is to be freed
  assert((reclaimed > 0) == forkable_stack_pop_will_free(s));
  int new_pos = s->pos + elem->next_delta;
  jv_mem_invalidate(elem, reclaimed);
  s->pos = new_pos;
}



struct forkable_stack_state {
  // We save the previous pos, savedlimit as
  // length-pos, length-savedlimit since these
  // values are stable across stack reallocations.
  int prev_pos_delta, prev_limit_delta;
};

static void forkable_stack_save(struct forkable_stack* s, struct forkable_stack_state* state) {
  forkable_stack_check(s);
  state->prev_pos_delta = s->length - s->pos;
  state->prev_limit_delta = s->length - s->savedlimit;
  if (s->pos < s->savedlimit) s->savedlimit = s->pos;
}

static void forkable_stack_switch(struct forkable_stack* s, struct forkable_stack_state* state) {
  forkable_stack_check(s);
  int curr_pos = s->pos;
  s->pos = s->length - state->prev_pos_delta;
  state->prev_pos_delta = s->length - curr_pos;

  int curr_limit = s->savedlimit;
  if (curr_pos < curr_limit) s->savedlimit = curr_pos;
  //state->prevlimit = curr_limit; FIXME clean up
  forkable_stack_check(s);
}

static void forkable_stack_restore(struct forkable_stack* s, struct forkable_stack_state* state) {
  forkable_stack_check(s);
  assert(s->savedlimit <= s->length - state->prev_pos_delta);
  assert(s->savedlimit <= s->length - state->prev_limit_delta);
  s->pos = s->length - state->prev_pos_delta;
  s->savedlimit = s->length - state->prev_limit_delta;
  forkable_stack_check(s);
}

typedef int stack_idx;

static stack_idx forkable_stack_to_idx(struct forkable_stack* s, void* ptr) {
  char* item = ptr;
  int pos = item - s->stk;
  assert(pos >= 0 && pos < s->length);
  return s->length - pos;
}

static void* forkable_stack_from_idx(struct forkable_stack* s, stack_idx idx) {
  assert(idx >= 1 && idx <= s->length);
  return &s->stk[s->length - idx];
}

#endif
