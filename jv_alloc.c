#include <stdlib.h>
#include "jv_alloc.h"


void* jv_mem_alloc(size_t sz) {
  return malloc(sz);
}

void jv_mem_free(void* p) {
  free(p);
}

void* jv_mem_realloc(void* p, size_t sz) {
  return realloc(p, sz);
}
