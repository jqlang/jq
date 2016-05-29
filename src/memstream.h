/*
 * memstream for OSX, courtesy of http://piumarta.com/software/memstream/
 */
 
#ifdef __APPLE__

#ifndef MEMSTREAM_H
#define MEMSTREAM_H

#include <stdio.h>

#if _POSIX_C_SOURCE < 200809L

#ifdef __cplusplus
extern "C" {
#endif

FILE *open_memstream(char **ptr, size_t *sizeloc);

#ifdef __cplusplus
}
#endif

#endif /* _POSIX_C_SOURCE < 200809L */

#endif /* MEMSTREAM_H */

#endif /* __APPLE__ */
