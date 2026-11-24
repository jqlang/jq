#ifndef JQ_ISO8601_H
#define JQ_ISO8601_H

#include <stddef.h>
#include <stdint.h>

/*
 * Parses [buf, buf + len) as an ISO 8601 date and time of day.
 *
 * On success stores the number of seconds since the Unix epoch in *out and
 * returns 1.  On failure returns 0 and leaves *out untouched.
 */
int iso8601_parse(const char *buf, size_t len, double *out);

/*
 * Days since 1970-01-01 of the proleptic Gregorian date y-m-d, negative
 * before the epoch. Requires 1 <= m <= 12 and 1 <= d <= days in that month.
 */
int64_t days_from_civil(int64_t y, int m, int d);

#endif /* JQ_ISO8601_H */
