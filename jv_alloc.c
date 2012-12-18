#include <stdlib.h>
#include <stdio.h>
#include "jv_alloc.h"

static void memory_exhausted() {
  fprintf(stderr, "error: cannot allocate memory\n");
  abort();
}

void* jv_mem_alloc(size_t sz) {
  void* p = malloc(sz);
  if (!p) {
    memory_exhausted();
  }
  return p;
}

void jv_mem_free(void* p) {
  free(p);
}

void* jv_mem_realloc(void* p, size_t sz) {
  p = realloc(p, sz);
  if (!p) {
    memory_exhausted();
  }
  return p;
}
