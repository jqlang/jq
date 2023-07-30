#include "tests.h"

int read_range(FILE *f, utf8proc_int32_t *start, utf8proc_int32_t *end)
{
     unsigned char buf[8192];
     size_t len = simple_getline(buf, f);
     size_t pos = skipspaces(buf, 0);
     unsigned char s[16];
     if (pos == len || buf[pos] == '#') return 0;
     pos += encode(s, buf + pos) - 1;
     check(s[0], "invalid line %s in data", buf);
     utf8proc_iterate((utf8proc_uint8_t*) s, -1, start);
     if (buf[pos] == '.' && buf[pos+1] == '.') {
          encode(s, buf + pos + 2);
          check(s[0], "invalid line %s in data", buf);
          utf8proc_iterate((utf8proc_uint8_t*) s, -1, end);
     }
     else
          *end = *start;
     return 1;
}

int test_iscase(const char *fname, int (*iscase)(utf8proc_int32_t),
                utf8proc_int32_t (*thatcase)(utf8proc_int32_t))
{
     FILE *f = fopen(fname, "r");
     int lines = 0, tests = 0, success = 1;
     utf8proc_int32_t c = 0;

     check(f != NULL, "error opening data file \"%s\"\n", fname);

     while (success && !feof(f)) {
          utf8proc_int32_t start, end;
          if (read_range(f, &start, &end)) {
               for (; c < start; ++c) {
                    check(!iscase(c), "failed !iscase(%04x) in %s\n", c, fname);
               }
               for (; c <= end; ++c) {
                    check(iscase(c), "failed iscase(%04x) in %s\n", c, fname);
                    check(thatcase(c) == c, "inconsistent thatcase(%04x) in %s\n", c, fname);
                    ++tests;
               }
          }
          ++lines;
     }
     for (; c <= 0x110000; ++c) {
          check(!iscase(c), "failed !iscase(%04x) in %s\n", c, fname);
     }

     printf("Checked %d characters from %d lines of %s\n", tests, lines, fname);
     fclose(f);
     return success;
}

int main(int argc, char **argv)
{
     check(argc == 3, "Expected Lowercase.txt and Uppercase.txt as arguments");
     check(test_iscase(argv[1], utf8proc_islower, utf8proc_tolower), "Lowercase tests failed");
     check(test_iscase(argv[2], utf8proc_isupper, utf8proc_toupper), "Uppercase tests failed");
     printf("utf8proc iscase tests SUCCEEDED.\n");
     return 0;
}
