#ifndef UTIL_H
#define UTIL_H 1

#include <inttypes.h>
#include <sys/time.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *readfile(const char *filename, size_t *len);

typedef struct timeval mytime;
mytime gettime(void);
double elapsed(mytime t1, mytime t0);

#ifdef __cplusplus
}
#endif

#endif /* UTIL_H */
