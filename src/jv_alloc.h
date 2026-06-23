#ifndef JV_ALLOC_H
#define JV_ALLOC_H

#include <stddef.h>

typedef void* (*jv_malloc_t)(size_t size);
typedef void (*jv_free_t)(void* ptr);
typedef void* (*jv_realloc_t)(void* ptr, size_t size);

void* jv_mem_alloc(size_t);
void* jv_mem_alloc_unguarded(size_t);
void* jv_mem_calloc(size_t, size_t);
void* jv_mem_calloc_unguarded(size_t, size_t);
char* jv_mem_strdup(const char *);
char* jv_mem_strdup_unguarded(const char *);
void jv_mem_free(void*);
__attribute__((warn_unused_result)) void* jv_mem_realloc(void*, size_t);

void jv_set_alloc_funcs(jv_malloc_t malloc_fn, jv_free_t free_fn);
void jv_set_alloc_funcs_ex(jv_malloc_t malloc_fn, jv_free_t free_fn, jv_realloc_t realloc_fn);

#endif
