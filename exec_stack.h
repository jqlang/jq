#ifndef EXEC_STACK_H
#define EXEC_STACK_H
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "jv_alloc.h"

/*
 * The stack is a directed forest of variably sized blocks. Each block has a
 * "next" block which is at a higher memory address, or 0 if the block has no
 * "next" block. More than one block may have no "next" block. A block may be
 * the "next" block of more than one other block. Pushed blocks are added at
 * the low-address end of the stack.
 *
 * Stack pointers are negative integers that are offsets relative to "mem_end",
 * the end of the allocated region. The stack "bound" is the stack pointer of
 * the last block that would be able to fit in the currently allocated region.
 * The stack "limit" is the stack pointer of the last block currently in the
 * stack. The stack pointer of the "next" block is stored directly below each
 * block.
 *
 *                      <- mem_end = 0x100
 * 0xF8  +------------+
 * 0xF0  |            |
 * 0xE8  +------------+ <- stack_ptr1 = -0x18
 * 0xE0  next = 0
 * 0xD8  +------------+
 * 0xD0  |            |
 * 0xC8  |            |
 * 0xC0  +------------+ <- stack_ptr2 = limit = -0x40
 * 0xB8  next = -0x18
 * 0xB0
 * 0xA8                 <- bound = -0x58
 * 0xA0
 */

struct determine_alignment {
  char x;
  union { int i; double d; uint64_t u64; size_t sz; void* ptr; } u;
};
enum {ALIGNMENT = offsetof(struct determine_alignment, u)};

static size_t align_round_up(size_t sz) {
  return ((sz + (ALIGNMENT - 1)) / ALIGNMENT) * ALIGNMENT;
}

typedef int stack_ptr;

struct stack {
  char* mem_end; // one-past-the-end of allocated region
  stack_ptr bound;
  stack_ptr limit; // 0 - stack is empty
};

static void stack_init(struct stack* s) {
  s->mem_end = 0;
  s->bound = ALIGNMENT;
  s->limit = 0;
}

static void stack_reset(struct stack* s) {
  assert(s->limit == 0 && "stack freed while not empty");
  char* mem_start = s->mem_end - ( -s->bound + ALIGNMENT);
  free(mem_start);
  stack_init(s);
}

static int stack_pop_will_free(struct stack* s, stack_ptr p) {
  return p == s->limit;
}

static void* stack_block(struct stack* s, stack_ptr p) {
  return (void*)(s->mem_end + p);
}

static stack_ptr* stack_block_next(struct stack* s, stack_ptr p) {
  return &((stack_ptr*)stack_block(s, p))[-1];
}

static void stack_reallocate(struct stack* s, size_t sz) {
  int old_mem_length = -(s->bound) + ALIGNMENT;
  char* old_mem_start = s->mem_end - old_mem_length;

  int new_mem_length = align_round_up((old_mem_length + sz + 256) * 2);
  char* new_mem_start = jv_mem_realloc(old_mem_start, new_mem_length);
  memmove(new_mem_start + (new_mem_length - old_mem_length),
            new_mem_start, old_mem_length);
  s->mem_end = new_mem_start + new_mem_length;
  s->bound = -(new_mem_length - ALIGNMENT);
}

static stack_ptr stack_push_block(struct stack* s, stack_ptr p, size_t sz) {
  int alloc_sz = align_round_up(sz) + ALIGNMENT;
  stack_ptr r = s->limit - alloc_sz;
  if (r < s->bound) {
    stack_reallocate(s, alloc_sz);
  }
  s->limit = r;
  *stack_block_next(s, r) = p;
  return r;
}

static stack_ptr stack_pop_block(struct stack* s, stack_ptr p, size_t sz) {
  stack_ptr r = *stack_block_next(s, p);
  if (p == s->limit) {
    int alloc_sz = align_round_up(sz) + ALIGNMENT;
    s->limit += alloc_sz;
  }
  return r;
}
#endif
