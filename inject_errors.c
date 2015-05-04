
#define _GNU_SOURCE /* for RTLD_NEXT */
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static FILE *fail;
static FILE *fail_read;
static FILE *fail_write;
static FILE *fail_close;
static int error;

static FILE * (*real_fopen)(const char *, const char *);
static int           (*real_fclose)(FILE *);
static int           (*real_ferror)(FILE *);
static void          (*real_clearerr)(FILE *);
static char *        (*real_fgets)(char *, int, FILE *);
static size_t        (*real_fread)(void *, size_t, size_t, FILE *);
static size_t        (*real_fwrite)(const void *, size_t, size_t, FILE *);

#define GET_REAL(sym)                         \
  do {                                        \
    if (real_ ## sym == 0) {                  \
      real_ ## sym = dlsym(RTLD_NEXT, #sym);  \
      assert(real_ ## sym != 0);              \
    }                                         \
  } while (0)

#define dbg_write(msg) (void)write(2, msg, sizeof(msg) - 1)

#define dbg() \
  do {                                      \
    dbg_write("here: ");                    \
    dbg_write(__func__);                    \
    dbg_write("!\n");                       \
  } while (0)

FILE *fopen(const char *path, const char *mode) {
  GET_REAL(fopen);
  fail = fail_read = fail_write = fail_close = 0;
  FILE *f = real_fopen(path, mode);
  error = EIO;
  if (strcmp(path, "fail_read") == 0) {
    fail = fail_read = f;
  } else if (strncmp(path, "fail_write", sizeof("fail_write") - 1) == 0) {
    // Not that jq opens files for write anyways...
    fail = fail_write = f;
    if (strcmp(path, "fail_write_enospc") == 0)
      error = ENOSPC;
  } else if (strncmp(path, "fail_close", sizeof("fail_close") - 1) == 0) {
    fail = fail_close = f;
    if (strcmp(path, "fail_close_enospc") == 0)
      error = ENOSPC;
  }
  return f;
}

int fclose(FILE *f) {
  GET_REAL(fclose);
  int res = real_fclose(f);
  if (fail_close == f) {
    fail = fail_read = fail_write = fail_close = 0;
    return EOF;
  }
  return res;
}

char * fgets(char *buf, int len, FILE *f) {
  GET_REAL(fgets);
  char *res = real_fgets(buf, len, f);
  if (fail_read == f)
    return 0;
  return res;
}

size_t fread(void *buf, size_t sz, size_t nemb, FILE *f) {
  GET_REAL(fread);
  size_t res = real_fread(buf, sz, nemb, f);
  if (fail_read == f)
    return 0;
  return res;
}

size_t fwrite(const void *buf, size_t sz, size_t nemb, FILE *f) {
  GET_REAL(fwrite);
  size_t res = real_fwrite(buf, sz, nemb, f);
  if (fail_write == f)
    return 0;
  return res;
}

int ferror(FILE *f) {
  GET_REAL(ferror);
  int res = real_ferror(f);
  if (fail == f) {
    errno = error;
    return 1;
  }
  return res;
}

void clearerr(FILE *f) {
  GET_REAL(clearerr);
  real_clearerr(f);
  if (fail == f) {
    fail = fail_read = fail_write = fail_close = 0;
    error = 0;
  }
}
