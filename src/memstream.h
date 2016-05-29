#ifdef __APPLE__

#ifndef MEMSTREAM_H
#define MEMSTREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#if _POSIX_C_SOURCE < 200809L

FILE *open_memstream(char **ptr, size_t *sizeloc);

#endif /* _POSIX_C_SOURCE < 200809L */

#ifdef __cplusplus
}
#endif

#endif // MEMSTREAM_H

#endif // __APPLE__
