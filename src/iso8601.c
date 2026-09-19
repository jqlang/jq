/*
 * A self-contained ISO 8601 date and time parser.
 *
 * The C library is of no help here: strptime() has no format specifier for
 * fractional seconds, and no portable way to parse week dates (glibc
 * consumes %G and %V but then discards them).  The results would also
 * differ between the host strptime() and the copy bundled in util.c.
 * Everything below is plain integer arithmetic instead, which also keeps
 * the computation clear of the range of a 32-bit time_t.
 *
 * The accepted grammar is
 *
 *   datetime := date [ ("T"|"t"|" ") time ]
 *   date     := year "-" MM "-" DD           |  year MMDD
 *             | year "-" DDD                 |  year DDD
 *             | year "-" ("W"|"w") ww "-" d  |  year ("W"|"w") ww d
 *   year     := YYYY | ("+"|"-") YYYYYY
 *   time     := hms [ ("."|",") digits ] [ offset ]
 *   hms      := hh [ ":" mm [ ":" ss ] ]  |  hh [ mm [ ss ] ]
 *   offset   := ("Z"|"z") | ("+"|"-") hh [ [":"] mm ]
 *
 * As in the standard's own notation a run of letters stands for that many
 * decimal digits -- DDD is the three-digit day of the year, d the one-digit
 * day of the week -- while "digits" is one or more of them.
 *
 * The decimal fraction belongs to the lowest component of the hms that is
 * present, so "23,5" is half past 23 and "2320,8" is 23:20:48.
 *
 * The date must always be complete, so "2015-03" is an error even though it
 * is a valid ISO 8601 date.  The time may drop its seconds, or its minutes
 * and seconds, or be omitted altogether, in which case the value is
 * midnight UTC.  The date and the time must agree on whether separators are
 * used, as ISO 8601 requires -- "2026-08-28T093015Z" is not a valid
 * representation.  The offset is exempt from that rule, since "+0900" after
 * an extended time is common in the wild.
 */

#include <stdint.h>

#include "iso8601.h"

/*
 * Days since 1970-01-01 of the civil date y-m-d in the proleptic Gregorian
 * calendar, negative before the epoch, after Howard Hinnant's
 * days_from_civil().  Requires 1 <= m <= 12 and 1 <= d <= days_in_month(y, m);
 * the callers check both.
 */
static int64_t days_from_civil(int64_t y, int m, int d) {
  y -= m <= 2;
  int64_t era = (y >= 0 ? y : y - 399) / 400;
  int64_t yoe = y - era * 400;                                  /* [0, 399] */
  int64_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; /* [0, 365] */
  int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;          /* [0, 146096] */
  return era * 146097 + doe - 719468;
}

static int is_leap(int64_t y) {
  return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

static int days_in_month(int64_t y, int m) {
  static const int d[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  return m == 2 && is_leap(y) ? 29 : d[m - 1];
}

/* ISO weekday (1 = Monday .. 7 = Sunday) of a day count since the epoch. */
static int iso_wday(int64_t days) {
  int64_t w = (days + 3) % 7;
  if (w < 0)
    w += 7;
  return (int)w + 1;
}

/*
 * An ISO year has 53 weeks if it starts on a Thursday, or if it is a leap
 * year starting on a Wednesday; otherwise it has 52.
 */
static int iso_weeks_in_year(int64_t y) {
  int jan1 = iso_wday(days_from_civil(y, 1, 1));
  return jan1 == 4 || (jan1 == 3 && is_leap(y)) ? 53 : 52;
}

static int civil_to_days(int64_t y, int mon, int mday, int64_t *days) {
  if (mon < 1 || mon > 12 || mday < 1 || mday > days_in_month(y, mon))
    return 0;
  *days = days_from_civil(y, mon, mday);
  return 1;
}

static int ordinal_to_days(int64_t y, int yday, int64_t *days) {
  if (yday < 1 || yday > (is_leap(y) ? 366 : 365))
    return 0;
  *days = days_from_civil(y, 1, 1) + yday - 1;
  return 1;
}

static int week_to_days(int64_t y, int week, int wday, int64_t *days) {
  if (week < 1 || week > iso_weeks_in_year(y) || wday < 1 || wday > 7)
    return 0;
  int64_t jan4 = days_from_civil(y, 1, 4);
  *days = jan4 - (iso_wday(jan4) - 1) + (int64_t)(week - 1) * 7 + (wday - 1);
  return 1;
}

/*
 * The scanner works over an explicit [p, end) range rather than a C string, so
 * that a NUL byte embedded in the input is a parse error instead of silently
 * truncating the value.
 */
typedef struct {
  const char *p;
  const char *end;
} iso8601_scanner;

static int scan_at_end(iso8601_scanner *s) {
  return s->p == s->end;
}

static int scan_peek(iso8601_scanner *s) {
  return scan_at_end(s) ? -1 : (unsigned char)*s->p;
}

static int scan_eat(iso8601_scanner *s, char c) {
  if (scan_at_end(s) || *s->p != c)
    return 0;
  s->p++;
  return 1;
}

/*
 * W, T and Z are what ISO 8601 calls designators.  They are written in upper
 * case, but the standard notes that "lower case characters may be used when
 * upper case characters are not available", so match either.
 */
static int scan_designator(iso8601_scanner *s, char c) {
  if (scan_at_end(s) || (*s->p != c && *s->p != c + ('a' - 'A')))
    return 0;
  s->p++;
  return 1;
}

static size_t scan_count_digits(iso8601_scanner *s) {
  size_t n = 0;
  while (s->p + n < s->end && s->p[n] >= '0' && s->p[n] <= '9')
    n++;
  return n;
}

static int scan_digits(iso8601_scanner *s, int n, int *out) {
  if (s->end - s->p < n)
    return 0;
  int v = 0;
  for (int i = 0; i < n; i++) {
    unsigned char c = (unsigned char)s->p[i];
    if (c < '0' || c > '9')
      return 0;
    v = v * 10 + (c - '0');
  }
  s->p += n;
  *out = v;
  return 1;
}

/*
 * A plain year is exactly four digits.  An expanded year carries a mandatory
 * sign and exactly six digits, which is what makes it unambiguous in front of
 * a date in the basic form.
 */
static int parse_year(iso8601_scanner *s, int64_t *year) {
  int64_t sign = 1;
  int digits = 4;
  if (scan_peek(s) == '+' || scan_peek(s) == '-') {
    sign = *s->p == '-' ? -1 : 1;
    s->p++;
    digits = 6;
  }
  if (s->end - s->p < digits)
    return 0;
  int64_t v = 0;
  for (int i = 0; i < digits; i++) {
    unsigned char c = (unsigned char)s->p[i];
    if (c < '0' || c > '9')
      return 0;
    v = v * 10 + (c - '0');
  }
  s->p += digits;
  *year = sign * v;
  return 1;
}

/* Sets *extended to whether the date was written with "-" separators. */
static int parse_date(iso8601_scanner *s, int64_t *days, int *extended) {
  int64_t year;
  int week, wday, mon, mday, yday;

  if (!parse_year(s, &year))
    return 0;

  if (scan_eat(s, '-')) {
    *extended = 1;
    if (scan_designator(s, 'W')) {
      return scan_digits(s, 2, &week) && scan_eat(s, '-') &&
             scan_digits(s, 1, &wday) && week_to_days(year, week, wday, days);
    }
    switch (scan_count_digits(s)) {
    case 3:
      return scan_digits(s, 3, &yday) && ordinal_to_days(year, yday, days);
    case 2:
      return scan_digits(s, 2, &mon) && scan_eat(s, '-') &&
             scan_digits(s, 2, &mday) && civil_to_days(year, mon, mday, days);
    default:
      return 0;
    }
  }

  *extended = 0;
  if (scan_designator(s, 'W')) {
    return scan_digits(s, 2, &week) && scan_digits(s, 1, &wday) &&
           week_to_days(year, week, wday, days);
  }
  switch (scan_count_digits(s)) {
  case 4:
    return scan_digits(s, 2, &mon) && scan_digits(s, 2, &mday) &&
           civil_to_days(year, mon, mday, days);
  case 3:
    return scan_digits(s, 3, &yday) && ordinal_to_days(year, yday, days);
  default:
    /* A bare "YYYYMM" is ambiguous with "YYMMDD"; the standard forbids it. */
    return 0;
  }
}

/*
 * Parses the time of day, storing the seconds since midnight in *secs.  A
 * time of "24:00:00" is midnight of the following day, which *carry reports.
 *
 * The minutes and seconds may be omitted, in which case the decimal fraction
 * belongs to the lowest component that is present -- "23,5" is half past 23
 * -- so *unit* records how many seconds that component is worth.
 */
static int parse_time(iso8601_scanner *s, int extended, double *secs,
                      int64_t *carry) {
  int hh, mm = 0, ss = 0, unit = 3600;

  if (!scan_digits(s, 2, &hh))
    return 0;
  if (extended ? scan_eat(s, ':') : scan_count_digits(s) >= 2) {
    if (!scan_digits(s, 2, &mm))
      return 0;
    unit = 60;
    if (extended ? scan_eat(s, ':') : scan_count_digits(s) >= 2) {
      if (!scan_digits(s, 2, &ss))
        return 0;
      unit = 1;
    }
  }

  double frac = 0;
  if (scan_peek(s) == '.' || scan_peek(s) == ',') {
    static const double pow10[] = {
      1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7,
      1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15
    };
    s->p++;
    if (scan_count_digits(s) == 0)
      return 0;
    uint64_t num = 0;
    int digits = 0;
    while (scan_peek(s) >= '0' && scan_peek(s) <= '9') {
      if (digits < 15) {
        num = num * 10 + (uint64_t)(*s->p - '0');
        digits++;
      }
      s->p++;
    }
    frac = (double)num / pow10[digits];
  }

  *carry = 0;
  if (hh == 24) {
    if (mm != 0 || ss != 0 || frac != 0)
      return 0;
    hh = 0;
    *carry = 1;
  } else if (hh > 24 || mm > 59 || ss > 60) {
    return 0;
  } else if (ss == 60) {
    ss = 59;
  }

  *secs = hh * 3600 + mm * 60 + ss + frac * unit;
  return 1;
}

/* An absent offset means UTC, which is what jq has always assumed. */
static int parse_offset(iso8601_scanner *s, int *offset) {
  if (scan_designator(s, 'Z')) {
    *offset = 0;
    return 1;
  }

  int sign = scan_peek(s);
  if (sign != '+' && sign != '-') {
    *offset = 0;
    return 1;
  }

  s->p++;
  int oh, om = 0;
  if (!scan_digits(s, 2, &oh))
    return 0;
  int had_colon = scan_eat(s, ':');
  if (!scan_digits(s, 2, &om) && had_colon)
    return 0;
  if (oh > 23 || om > 59)
    return 0;

  *offset = (sign == '+' ? 1 : -1) * (oh * 3600 + om * 60);
  return 1;
}

int jvp_iso8601_parse(const char *buf, size_t len, double *out) {
  iso8601_scanner s = {buf, buf + len};
  int64_t days, carry = 0;
  int extended, offset = 0, sep;
  double secs = 0;

  if (!parse_date(&s, &days, &extended))
    return 0;

  sep = scan_peek(&s);
  if (sep == 'T' || sep == 't' || sep == ' ') {
    s.p++;
    if (!parse_time(&s, extended, &secs, &carry) || !parse_offset(&s, &offset))
      return 0;
  }
  if (!scan_at_end(&s))
    return 0;

  *out = (double)(days + carry) * 86400.0 + secs - offset;
  return 1;
}
