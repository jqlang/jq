#ifndef JV_ALLOC_H
#define JV_ALLOC_H

#include <stddef.h>

void* jv_mem_alloc(size_t);
void jv_mem_free(void*);
__attribute__((warn_unused_result)) void* jv_mem_realloc(void*, size_t);

#endif
