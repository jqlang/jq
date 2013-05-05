#ifndef JV_ALLOC_H
#define JV_ALLOC_H

#include <stddef.h>

#ifndef NDEBUG
extern volatile char jv_mem_uninitialised;
#endif

static void jv_mem_invalidate(void* mem, size_t n) {
#ifndef NDEBUG
  char* m = mem;
  while (n--) *m++ ^= jv_mem_uninitialised ^ jv_mem_uninitialised;
#endif
}

void* jv_mem_alloc(size_t);
void jv_mem_free(void*);
__attribute__((warn_unused_result)) void* jv_mem_realloc(void*, size_t);

#endif
