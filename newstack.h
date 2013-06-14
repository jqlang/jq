#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
struct determine_alignment {
  char x;
  union {
    int i;
    double d;
    uint64_t u64;
    size_t sz;
    void* ptr;
  } u;
};

enum {ALIGNMENT = offsetof(struct determine_alignment, u)};

size_t align_round_up(size_t sz) {
  return ((sz + (ALIGNMENT - 1)) / ALIGNMENT) * ALIGNMENT;
}

typedef int stack_ptr;

struct stack {
  char* mem_end; // one-past-the-end of allocated region
  stack_ptr bound;
  stack_ptr limit; // 0 - stack is empty
};

void stack_init(struct stack* s) {
  s->mem_end = 0;
  s->bound = ALIGNMENT;
  s->limit = 0;
}

void stack_free(struct stack* s) {
  char* mem_start = s->mem_end - ( -s->bound + ALIGNMENT);
  free(mem_start);
  stack_init(s);
}

stack_ptr stack_top(struct stack* s) {
  return s->limit;
}

int stack_pop_will_free(struct stack* s, stack_ptr p) {
  return p == s->limit;
}

void* stack_block(struct stack* s, stack_ptr p) {
  return (void*)(s->mem_end + p);
}

stack_ptr* stack_block_next(struct stack* s, stack_ptr p) {
  return &(((stack_ptr*)stack_block(s, p))[-1]);
}

void stack_reallocate(struct stack* s, size_t sz) {
  int old_mem_length = -(s->bound) + ALIGNMENT;
  char* old_mem_start = s->mem_end - old_mem_length;
  
  int new_mem_length = align_round_up((old_mem_length + sz + 256) * 2);
  char* new_mem_start = realloc(old_mem_start, new_mem_length);
  memmove(new_mem_start + (new_mem_length - old_mem_length),
            new_mem_start, old_mem_length);
  s->mem_end = new_mem_start + new_mem_length;
  s->bound = -(new_mem_length - ALIGNMENT);
}

stack_ptr stack_push_block(struct stack* s, stack_ptr p, size_t sz) {
  int alloc_sz = align_round_up(sz) + ALIGNMENT;
  stack_ptr r = s->limit - alloc_sz;
  if (r < s->bound) {
    stack_reallocate(s, alloc_sz);
  }
  s->limit = r;
  *stack_block_next(s, r) = p;
  return r;
}

stack_ptr stack_pop_block(struct stack* s, stack_ptr p, size_t sz) {
  stack_ptr r = *stack_block_next(s, p);
  if (p == s->limit) {
    int alloc_sz = align_round_up(sz) + ALIGNMENT;
    s->limit += alloc_sz;
  }
  return r;
}
