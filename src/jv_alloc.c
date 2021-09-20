#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "jv_alloc.h"

struct nomem_handler {
    jv_nomem_handler_f handler;
    void *data;
};

#if !defined(HAVE_PTHREAD_KEY_CREATE) || \
    !defined(HAVE_PTHREAD_ONCE) || \
    !defined(HAVE_ATEXIT)

/* Try thread-local storage? */

#ifdef _MSC_VER
/* Visual C++: yes */
static __declspec(thread) struct nomem_handler nomem_handler;
#define USE_TLS
#else
#ifdef HAVE___THREAD
/* GCC and friends: yes */
static __thread struct nomem_handler nomem_handler;
#define USE_TLS
#endif /* HAVE___THREAD */
#endif /* _MSC_VER */

#endif /* !HAVE_PTHREAD_KEY_CREATE */

#ifdef USE_TLS
void jv_nomem_handler(jv_nomem_handler_f handler, void *data) {
  nomem_handler.handler = handler;
}

static void memory_exhausted() {
  if (nomem_handler.handler)
    nomem_handler.handler(nomem_handler.data); // Maybe handler() will longjmp() to safety
  // Or not
  fprintf(stderr, "jq: error: cannot allocate memory\n");
  abort();
}
#else /* USE_TLS */

#ifdef HAVE_PTHREAD_KEY_CREATE
#include <pthread.h>

pthread_key_t nomem_handler_key;
pthread_once_t mem_once = PTHREAD_ONCE_INIT;

static void tsd_fini(void) {
  struct nomem_handler *nomem_handler;
  nomem_handler = pthread_getspecific(nomem_handler_key);
  if (nomem_handler) {
    (void) pthread_setspecific(nomem_handler_key, NULL);
    free(nomem_handler);
  }
}

static void tsd_init(void) {
  if (pthread_key_create(&nomem_handler_key, NULL) != 0) {
    fprintf(stderr, "jq: error: cannot create thread specific key");
    abort();
  }
  if (atexit(tsd_fini) != 0) {
    fprintf(stderr, "jq: error: cannot set an exit handler");
    abort();
  }
  struct nomem_handler *nomem_handler = calloc(1, sizeof(struct nomem_handler));
  if (pthread_setspecific(nomem_handler_key, nomem_handler) != 0) {
    fprintf(stderr, "jq: error: cannot set thread specific data");
    abort();
  }
}

void jv_nomem_handler(jv_nomem_handler_f handler, void *data) {
  pthread_once(&mem_once, tsd_init); // cannot fail
  struct nomem_handler *nomem_handler;

  nomem_handler = pthread_getspecific(nomem_handler_key);
  if (nomem_handler == NULL) {
    handler(data);
    fprintf(stderr, "jq: error: cannot allocate memory\n");
    abort();
  }
  nomem_handler->handler = handler;
  nomem_handler->data = data;
}

static void memory_exhausted() {
  struct nomem_handler *nomem_handler;

  pthread_once(&mem_once, tsd_init);
  nomem_handler = pthread_getspecific(nomem_handler_key);
  if (nomem_handler)
    nomem_handler->handler(nomem_handler->data); // Maybe handler() will longjmp() to safety
  // Or not
  fprintf(stderr, "jq: error: cannot allocate memory\n");
  abort();
}

#else

/* No thread-local storage of any kind that we know how to handle */

static struct nomem_handler nomem_handler;
void jv_nomem_handler(jv_nomem_handler_f handler, void *data) {
  nomem_handler.handler = handler;
  nomem_handler.data = data;
}

static void memory_exhausted() {
  fprintf(stderr, "jq: error: cannot allocate memory\n");
  abort();
}

#endif /* HAVE_PTHREAD_KEY_CREATE */
#endif /* USE_TLS */


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

void* jv_mem_calloc(size_t nemb, size_t sz) {
  void* p = calloc(nemb, sz);
  if (!p) {
    memory_exhausted();
  }
  return p;
}

void* jv_mem_calloc_unguarded(size_t nemb, size_t sz) {
  return calloc(nemb, sz);
}

char* jv_mem_strdup(const char *s) {
  char *p = strdup(s);
  if (!p) {
    memory_exhausted();
  }
  return p;
}

char* jv_mem_strdup_unguarded(const char *s) {
  return strdup(s);
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
  // ignore warning that this reads uninitialized memory - that's the point!
#ifndef __clang_analyzer__
  char* p = malloc(1);
  jv_mem_uninitialised = *p;
  free(p);
#endif
}
#endif
