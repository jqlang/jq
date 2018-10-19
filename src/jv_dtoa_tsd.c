#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "jv_dtoa_tsd.h"
#include "jv_dtoa.h"
#include "jv_alloc.h"


static pthread_key_t dtoa_ctx_key;
static pthread_once_t dtoa_ctx_once = PTHREAD_ONCE_INIT;

static void tsd_dtoa_ctx_dtor(struct dtoa_context *ctx) {
  if (ctx) {
    jvp_dtoa_context_free(ctx);
    jv_mem_free(ctx);
  }
}

static void tsd_dtoa_ctx_fini() {
  struct dtoa_context *ctx = pthread_getspecific(dtoa_ctx_key);
  tsd_dtoa_ctx_dtor(ctx);
  pthread_setspecific(dtoa_ctx_key, NULL);
}

static void tsd_dtoa_ctx_init() {
  if (pthread_key_create(&dtoa_ctx_key, tsd_dtoa_ctx_dtor) != 0) {
    fprintf(stderr, "error: cannot create thread specific key");
    abort();
  }
  atexit(tsd_dtoa_ctx_fini);
}

inline struct dtoa_context *tsd_dtoa_context_get() {
  pthread_once(&dtoa_ctx_once, tsd_dtoa_ctx_init); // cannot fail
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