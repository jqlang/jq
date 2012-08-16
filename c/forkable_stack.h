#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

struct forkable_stack_header {
  int next;
};

#define FORKABLE_STACK_HEADER struct forkable_stack_header fk_header_

struct forkable_stack {
  char* stk;

  // stk+length is just past end of allocated area
  int length;

  // stk+pos is just past top-of-stack item
  int pos;

  // everything before stk+savedlimit must be preserved
  int savedlimit;
};

static void forkable_stack_check(struct forkable_stack* s) {
  assert(s->stk);
  assert(s->length > 0);
  assert(s->pos >= 0 && s->pos <= s->length);
  assert(s->savedlimit >= 0 && s->savedlimit <= s->length);
}

static int forkable_stack_empty(struct forkable_stack* s) {
  return s->pos == 0;
}

static void forkable_stack_init(struct forkable_stack* s, size_t sz) {
  s->stk = malloc(sz);
  s->length = sz;
  s->pos = 0;
  s->savedlimit = 0;
  forkable_stack_check(s);
}

static void* forkable_stack_push(struct forkable_stack* s, size_t size) {
  forkable_stack_check(s);
  int curr = s->pos > s->savedlimit ? s->pos : s->savedlimit;
  if (curr + size > s->length) {
    s->length = (size + s->length + 1024) * 2;
    s->stk = realloc(s->stk, s->length);
  }
  void* ret = (void*)(s->stk + curr);
  ((struct forkable_stack_header*)ret)->next = s->pos;
  s->pos = curr + size;
  return ret;
}

static void* forkable_stack_peek(struct forkable_stack* s, size_t size) {
  assert(!forkable_stack_empty(s));
  return (void*)(s->stk + s->pos - size);
}

static void* forkable_stack_peek_next(struct forkable_stack* s, void* top, size_t size) {
  struct forkable_stack_header* elem = top;
  return (void*)(s->stk + elem->next - size);
}

static void forkable_stack_pop(struct forkable_stack* s, size_t size) {
  struct forkable_stack_header* elem = forkable_stack_peek(s, size);
  s->pos = elem->next;
}

struct forkable_stack_state {
  int prevpos, prevlimit;
};

static void forkable_stack_save(struct forkable_stack* s, struct forkable_stack_state* state) {
  state->prevpos = s->pos;
  state->prevlimit = s->savedlimit;
  if (s->pos > s->savedlimit) s->savedlimit = s->pos;
}

static void forkable_stack_switch(struct forkable_stack* s, struct forkable_stack_state* state) {
  int curr_pos = s->pos;
  s->pos = state->prevpos;
  state->prevpos = curr_pos;

  int curr_limit = s->savedlimit;
  if (curr_pos > curr_limit) s->savedlimit = curr_pos;
  state->prevlimit = curr_limit;
}

static void forkable_stack_restore(struct forkable_stack* s, struct forkable_stack_state* state) {
  s->pos = state->prevpos;
  s->savedlimit = state->prevlimit;
}
