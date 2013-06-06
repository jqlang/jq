#include <stdlib.h>
#include <stdio.h>
#include "jv_alloc.h"

struct nomem_handler {
    jv_nomem_handler_f handler;
    void *data;
};

#ifndef USE_PTHREAD_KEY
#ifdef _MSC_VER
static __declspec(thread) struct nomem_handler nomem_handler;
#else
static __thread struct nomem_handler nomem_handler;
#endif

void jv_nomem_handler(jv_nomem_handler_f handler, void *data) {
  nomem_handler.handler = handler;
}

static void memory_exhausted() {
  if (nomem_handler.handler)
    nomem_handler.handler(nomem_handler.data); // Maybe handler() will longjmp() to safety
  // Or not
  fprintf(stderr, "error: cannot allocate memory\n");
  abort();
}
#else
#ifdef HAVE_PTHREAD_KEY_CREATE
#include <pthread.h>

pthread_key_t nomem_handler_key;
pthread_once_t mem_once = PTHREAD_ONCE_INIT;

static void tsd_init(void) {
  if (pthread_key_create(&nomem_handler_key, NULL) != 0) {
    fprintf(stderr, "error: cannot create thread specific key");
    abort();
  }
}

void jv_nomem_handler(jv_nomem_handler_f handler, void *data) {
  pthread_once(&mem_once, tsd_init); // cannot fail
  struct nomem_handler *nomem_handler = calloc(1, sizeof (nomem_handler));
  if (nomem_handler == NULL) {
    handler(data);
    fprintf(stderr, "error: cannot allocate memory\n");
    abort();
  }
  nomem_handler.handler = handler;
  nomem_handler.data = data;
  if (pthread_setspecific(nomem_handler_key, nomem_handler) != 0) {
    handler(data);
    fprintf(stderr, "error: cannot set thread specific data");
    abort();
  }
}

static void memory_exhausted() {
  jv_nomem_handler_f handler;

  pthread_once(&mem_once, tsd_init);
  handler = pthread_getspecific(nomem_handler_key);
  if (handler)
    handler(); // Maybe handler() will longjmp() to safety
  // Or not
  fprintf(stderr, "error: cannot allocate memory\n");
  abort();
}

#else

static struct nomem_handler nomem_handler;
void jv_nomem_handler(jv_nomem_handler_f handler, void *data) {
  nomem_handler.handler = handler;
  nomem_handler.data = data;
}

static void memory_exhausted() {
  fprintf(stderr, "error: cannot allocate memory\n");
  abort();
}

#endif /* HAVE_PTHREAD_KEY_CREATE */
#endif /* !USE_PTHREAD_KEY */


void* jv_mem_alloc(size_t sz) {
  void* p = malloc(sz);
  if (!p) {
    memory_exhausted();
  }
  return p;
}

void* jv_mem_alloc_unguarded(size_t sz) {
  return malloc(sz);
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

#ifndef NDEBUG
volatile char jv_mem_uninitialised;
__attribute__((constructor)) void jv_mem_uninit_setup(){
  char* p = malloc(1);
  jv_mem_uninitialised = *p;
  free(p);
}
#endif
