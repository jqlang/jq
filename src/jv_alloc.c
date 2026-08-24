#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jv.h"
#include "jv_alloc.h"

struct nomem_handler {
    jv_nomem_handler_f handler;
    void *data;
};

struct custom_allocator {
    jv_malloc_t malloc_fn;
    jv_free_t free_fn;
    jv_realloc_t realloc_fn;
};

#if !defined(HAVE_PTHREAD_KEY_CREATE) || \
    !defined(HAVE_PTHREAD_ONCE) || \
    !defined(HAVE_ATEXIT)

/* Try thread-local storage? */

#ifdef _MSC_VER
/* Visual C++: yes */
static __declspec(thread) struct nomem_handler nomem_handler;
static __declspec(thread) struct custom_allocator custom_allocator;
#define USE_TLS
#else
#ifdef HAVE___THREAD
/* GCC and friends: yes */
static __thread struct nomem_handler nomem_handler;
static __thread struct custom_allocator custom_allocator;
#define USE_TLS
#endif /* HAVE___THREAD */
#endif /* _MSC_VER */

#endif /* !HAVE_PTHREAD_KEY_CREATE */

#ifdef USE_TLS
void jv_nomem_handler(jv_nomem_handler_f handler, void *data) {
  nomem_handler.handler = handler;
}

static void memory_exhausted(void) {
  if (nomem_handler.handler)
    nomem_handler.handler(nomem_handler.data); // Maybe handler() will longjmp() to safety
  // Or not
  fprintf(stderr, "jq: error: cannot allocate memory\n");
  abort();
}
#else /* USE_TLS */

#ifdef HAVE_PTHREAD_KEY_CREATE
#include <pthread.h>

static pthread_key_t nomem_handler_key;
static pthread_key_t custom_allocator_key;
static pthread_once_t mem_once = PTHREAD_ONCE_INIT;

/* tsd_fini is called on application exit */
/* it clears the nomem_handler and custom_allocator allocated in the main thread */
static void tsd_fini(void) {
  struct nomem_handler *nomem_handler;
  struct custom_allocator *custom_alloc;
  
  nomem_handler = pthread_getspecific(nomem_handler_key);
  if (nomem_handler) {
    (void) pthread_setspecific(nomem_handler_key, NULL);
    free(nomem_handler);
  }
  
  custom_alloc = pthread_getspecific(custom_allocator_key);
  if (custom_alloc) {
    (void) pthread_setspecific(custom_allocator_key, NULL);
    free(custom_alloc);
  }
}

/* The tsd_fini_thread is a destructor set by calling */
/* pthread_key_create(&nomem_handler_key, tsd_fini_thread) */
/* It is called when thread ends */
static void tsd_fini_thread(void *nomem_handler) {
  free(nomem_handler);
}

/* Destructor for custom allocator thread-specific data */
static void tsd_fini_custom_alloc(void *custom_alloc) {
  free(custom_alloc);
}

static void tsd_init(void) {
  if (pthread_key_create(&nomem_handler_key, tsd_fini_thread) != 0) {
    fprintf(stderr, "jq: error: cannot create thread specific key");
    abort();
  }
  if (pthread_key_create(&custom_allocator_key, tsd_fini_custom_alloc) != 0) {
    fprintf(stderr, "jq: error: cannot create thread specific key");
    abort();
  }
  if (atexit(tsd_fini) != 0) {
    fprintf(stderr, "jq: error: cannot set an exit handler");
    abort();
  }
}

static void tsd_init_nomem_handler(void)
{
  if (pthread_getspecific(nomem_handler_key) == NULL) {
    struct nomem_handler *nomem_handler = calloc(1, sizeof(struct nomem_handler));
    if (pthread_setspecific(nomem_handler_key, nomem_handler) != 0) {
      fprintf(stderr, "jq: error: cannot set thread specific data");
      abort();
    }
  }
}

static void tsd_init_custom_allocator(void)
{
  if (pthread_getspecific(custom_allocator_key) == NULL) {
    struct custom_allocator *custom_alloc = calloc(1, sizeof(struct custom_allocator));
    if (pthread_setspecific(custom_allocator_key, custom_alloc) != 0) {
      fprintf(stderr, "jq: error: cannot set thread specific data");
      abort();
    }
  }
}

void jv_nomem_handler(jv_nomem_handler_f handler, void *data) {
  pthread_once(&mem_once, tsd_init); // cannot fail
  tsd_init_nomem_handler();

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

static void memory_exhausted(void) {
  struct nomem_handler *nomem_handler;

  pthread_once(&mem_once, tsd_init);
  tsd_init_nomem_handler();

  nomem_handler = pthread_getspecific(nomem_handler_key);
  if (nomem_handler && nomem_handler->handler)
    nomem_handler->handler(nomem_handler->data); // Maybe handler() will longjmp() to safety
  // Or not
  fprintf(stderr, "jq: error: cannot allocate memory\n");
  abort();
}

#else

/* No thread-local storage of any kind that we know how to handle */

static struct nomem_handler nomem_handler;
static struct custom_allocator custom_allocator;

void jv_nomem_handler(jv_nomem_handler_f handler, void *data) {
  nomem_handler.handler = handler;
  nomem_handler.data = data;
}

static void memory_exhausted(void) {
  fprintf(stderr, "jq: error: cannot allocate memory\n");
  abort();
}

#endif /* HAVE_PTHREAD_KEY_CREATE */
#endif /* USE_TLS */


// Helper functions to get custom allocator based on threading model
#ifdef USE_TLS
static struct custom_allocator* get_custom_allocator(void) {
  return &custom_allocator;
}
#else
#ifdef HAVE_PTHREAD_KEY_CREATE
static struct custom_allocator* get_custom_allocator(void) {
  pthread_once(&mem_once, tsd_init);
  tsd_init_custom_allocator();
  return pthread_getspecific(custom_allocator_key);
}
#else
static struct custom_allocator* get_custom_allocator(void) {
  return &custom_allocator;
}
#endif
#endif

void* jv_mem_alloc(size_t sz) {
  struct custom_allocator* custom_alloc = get_custom_allocator();
  void* p;
  
  if (custom_alloc && custom_alloc->malloc_fn) {
    p = custom_alloc->malloc_fn(sz);
  } else {
    p = malloc(sz);
  }
  
  if (!p) {
    memory_exhausted();
  }
  return p;
}

void* jv_mem_alloc_unguarded(size_t sz) {
  struct custom_allocator* custom_alloc = get_custom_allocator();
  
  if (custom_alloc && custom_alloc->malloc_fn) {
    return custom_alloc->malloc_fn(sz);
  } else {
    return malloc(sz);
  }
}

void* jv_mem_calloc(size_t nemb, size_t sz) {
  assert(nemb > 0 && sz > 0);
  struct custom_allocator* custom_alloc = get_custom_allocator();
  void* p;
  
  if (custom_alloc && custom_alloc->malloc_fn) {
    p = custom_alloc->malloc_fn(nemb * sz);
    if (p) {
      memset(p, 0, nemb * sz);
    }
  } else {
    p = calloc(nemb, sz);
  }
  
  if (!p) {
    memory_exhausted();
  }
  return p;
}

void* jv_mem_calloc_unguarded(size_t nemb, size_t sz) {
  assert(nemb > 0 && sz > 0);
  struct custom_allocator* custom_alloc = get_custom_allocator();
  
  if (custom_alloc && custom_alloc->malloc_fn) {
    void* p = custom_alloc->malloc_fn(nemb * sz);
    if (p) {
      memset(p, 0, nemb * sz);
    }
    return p;
  } else {
    return calloc(nemb, sz);
  }
}

char* jv_mem_strdup(const char *s) {
  struct custom_allocator* custom_alloc = get_custom_allocator();
  char *p;
  
  if (custom_alloc && custom_alloc->malloc_fn) {
    size_t len = strlen(s) + 1;
    p = custom_alloc->malloc_fn(len);
    if (p) {
      memcpy(p, s, len);
    }
  } else {
    p = strdup(s);
  }
  
  if (!p) {
    memory_exhausted();
  }
  return p;
}

char* jv_mem_strdup_unguarded(const char *s) {
  struct custom_allocator* custom_alloc = get_custom_allocator();
  
  if (custom_alloc && custom_alloc->malloc_fn) {
    size_t len = strlen(s) + 1;
    char *p = custom_alloc->malloc_fn(len);
    if (p) {
      memcpy(p, s, len);
    }
    return p;
  } else {
    return strdup(s);
  }
}

void jv_mem_free(void* p) {
  struct custom_allocator* custom_alloc = get_custom_allocator();
  
  if (custom_alloc && custom_alloc->free_fn) {
    custom_alloc->free_fn(p);
  } else {
    free(p);
  }
}

void* jv_mem_realloc(void* p, size_t sz) {
  struct custom_allocator* custom_alloc = get_custom_allocator();
  
  if (custom_alloc && custom_alloc->realloc_fn) {
    p = custom_alloc->realloc_fn(p, sz);
  } else {
    p = realloc(p, sz);
  }
  
  if (!p) {
    memory_exhausted();
  }
  return p;
}

#ifdef USE_TLS
// Thread-local storage implementation
void jv_set_alloc_funcs(jv_malloc_t malloc_fn, jv_free_t free_fn) {
  custom_allocator.malloc_fn = malloc_fn;
  custom_allocator.free_fn = free_fn;
  custom_allocator.realloc_fn = NULL;
}

void jv_set_alloc_funcs_ex(jv_malloc_t malloc_fn, jv_free_t free_fn, jv_realloc_t realloc_fn) {
  custom_allocator.malloc_fn = malloc_fn;
  custom_allocator.free_fn = free_fn;
  custom_allocator.realloc_fn = realloc_fn;
}

#else

#ifdef HAVE_PTHREAD_KEY_CREATE
// Pthread-based implementation
void jv_set_alloc_funcs(jv_malloc_t malloc_fn, jv_free_t free_fn) {
  pthread_once(&mem_once, tsd_init);
  tsd_init_custom_allocator();
  
  struct custom_allocator *custom_alloc = pthread_getspecific(custom_allocator_key);
  if (custom_alloc) {
    custom_alloc->malloc_fn = malloc_fn;
    custom_alloc->free_fn = free_fn;
    custom_alloc->realloc_fn = NULL;
  }
}

void jv_set_alloc_funcs_ex(jv_malloc_t malloc_fn, jv_free_t free_fn, jv_realloc_t realloc_fn) {
  pthread_once(&mem_once, tsd_init);
  tsd_init_custom_allocator();
  
  struct custom_allocator *custom_alloc = pthread_getspecific(custom_allocator_key);
  if (custom_alloc) {
    custom_alloc->malloc_fn = malloc_fn;
    custom_alloc->free_fn = free_fn;
    custom_alloc->realloc_fn = realloc_fn;
  }
}

#else
// Global variable implementation
void jv_set_alloc_funcs(jv_malloc_t malloc_fn, jv_free_t free_fn) {
  custom_allocator.malloc_fn = malloc_fn;
  custom_allocator.free_fn = free_fn;
  custom_allocator.realloc_fn = NULL;
}

void jv_set_alloc_funcs_ex(jv_malloc_t malloc_fn, jv_free_t free_fn, jv_realloc_t realloc_fn) {
  custom_allocator.malloc_fn = malloc_fn;
  custom_allocator.free_fn = free_fn;
  custom_allocator.realloc_fn = realloc_fn;
}

#endif /* HAVE_PTHREAD_KEY_CREATE */
#endif /* USE_TLS */
