#ifndef JV_YYJSON_H
#define JV_YYJSON_H

#include <stddef.h>
#include "jv.h"

/*
 * Fast-path JSON reader built on yyjson.
 *
 * Both functions return 1 on success (storing the value in *out) and 0 when
 * the input is not something the fast path accepts.  A 0 return does not mean
 * the input is invalid JSON for jq: callers must fall back to the regular
 * jv_parser, which also produces jq's exact error messages.
 */

/* Parse exactly one complete JSON text spanning all of [s, s+len). */
int jv_yyjson_parse_all(const char *s, size_t len, jv *out);

/*
 * Parse one JSON value from the start of buf (leading whitespace allowed).
 * buf must be writable and followed by at least 4 readable/writable bytes
 * (yyjson padding); its contents are clobbered.  On success *consumed is the
 * number of bytes used up to the end of the value.
 */
int jv_yyjson_parse_next(char *buf, size_t len, size_t *consumed, jv *out);

#endif
