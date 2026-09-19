#ifndef JQ_ISO8601_H
#define JQ_ISO8601_H

#include <stddef.h>

/*
 * Parses [buf, buf + len) as an ISO 8601 date and time of day.
 *
 * On success stores the number of seconds since the Unix epoch in *out and
 * returns 1.  On failure returns 0 and leaves *out untouched.
 */
int jvp_iso8601_parse(const char *buf, size_t len, double *out);

#endif /* JQ_ISO8601_H */
