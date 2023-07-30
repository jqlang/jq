/* Common functions for our test programs. */

#include "tests.h"

size_t lineno = 0;

void check(int cond, const char *format, ...)
{
     if (!cond) {
          va_list args;
          fprintf(stderr, "line %zd: ", lineno);
          va_start(args, format);
          vfprintf(stderr, format, args);
          va_end(args);
          fprintf(stderr, "\n");
          exit(1);
     }
}

size_t skipspaces(const unsigned char *buf, size_t i)
{
    while (isspace(buf[i])) ++i;
    return i;
}

/* if buf points to a sequence of codepoints encoded as hexadecimal strings,
   separated by whitespace, and terminated by any character not in
   [0-9a-fA-F] or whitespace, then stores the corresponding utf8 string
   in dest, returning the number of bytes read from buf */
size_t encode(unsigned char *dest, const unsigned char *buf)
{
     size_t i = 0, j;
     utf8proc_ssize_t d = 0;
     for (;;) {
          int c;
          i = skipspaces(buf, i);
          for (j=i; buf[j] && strchr("0123456789abcdef", tolower(buf[j])); ++j)
               ; /* find end of hex input */
          if (j == i) { /* no codepoint found */
               dest[d] = 0; /* NUL-terminate destination string */
               return i + 1;
          }
          check(sscanf((char *) (buf + i), "%x", (unsigned int *)&c) == 1, "invalid hex input %s", buf+i);
          i = j; /* skip to char after hex input */
          d += utf8proc_encode_char(c, (utf8proc_uint8_t *) (dest + d));
     }
}

/* simplistic, portable replacement for getline, sufficient for our tests */
size_t simple_getline(unsigned char buf[8192], FILE *f) {
    size_t i = 0;
    while (i < 8191) {
        int c = getc(f);
        if (c == EOF || c == '\n') break;
        buf[i++] = (unsigned char) c;
    }
    buf[i] = 0;
    return i;
}
