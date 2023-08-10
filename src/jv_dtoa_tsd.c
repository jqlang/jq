#include <stdlib.h>
#include <stdio.h>

#include "jv_thread.h"
#include "jv_dtoa_tsd.h"
#include "jv_dtoa.h"
#include "jv_alloc.h"

static pthread_once_t dtoa_ctx_once = PTHREAD_ONCE_INIT;

static pthread_key_t dtoa_ctx_key;
static void tsd_dtoa_ctx_dtor(void *ctx) {
  if (ctx) {
    jvp_dtoa_context_free((struct dtoa_context *)ctx);
    jv_mem_free(ctx);
  }
}

#ifndef WIN32
static
#endif
void jv_tsd_dtoa_ctx_fini() {
  struct dtoa_context *ctx = pthread_getspecific(dtoa_ctx_key);
  tsd_dtoa_ctx_dtor(ctx);
  pthread_setspecific(dtoa_ctx_key, NULL);
}

#ifndef WIN32
static
#endif
void jv_tsd_dtoa_ctx_init() {
  if (pthread_key_create(&dtoa_ctx_key, tsd_dtoa_ctx_dtor) != 0) {
    fprintf(stderr, "error: cannot create thread specific key");
    abort();
  }
  atexit(jv_tsd_dtoa_ctx_fini);
}

inline struct dtoa_context *tsd_dtoa_context_get() {
  pthread_once(&dtoa_ctx_once, jv_tsd_dtoa_ctx_init); // cannot fail
  struct dtoa_context *ctx = (struct dtoa_context*)pthread_getspecific(dtoa_ctx_key);
  if (!ctx) {
    ctx = malloc(sizeof(struct dtoa_context));
    jvp_dtoa_context_init(ctx);
    if (pthread_setspecific(dtoa_ctx_key, ctx) != 0) {
      fprintf(stderr, "error: cannot set thread specific data");
      abort();
    }
  }
  return ctx;
}
