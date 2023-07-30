#include "tests.h"
#include <ctype.h>
#include <wchar.h>

int main(int argc, char **argv)
{
     int c, error = 0;

     (void) argc; /* unused */
     (void) argv; /* unused */

     /* some simple sanity tests of  */
     for (c = 0; c < 0xd800; c++) {
        if (!utf8proc_codepoint_valid(c)) {
           fprintf(stderr, "Failed: codepoint_valid(%04x) -> false\n", c);
           error++;
        }
     }
     for (;c < 0xe000; c++) {
        if (utf8proc_codepoint_valid(c)) {
           fprintf(stderr, "Failed: codepoint_valid(%04x) -> true\n", c);
           error++;
        }
     }
     for (;c < 0x110000; c++) {
        if (!utf8proc_codepoint_valid(c)) {
           fprintf(stderr, "Failed: codepoint_valid(%06x) -> false\n", c);
           error++;
        }
     }
     for (;c < 0x110010; c++) {
        if (utf8proc_codepoint_valid(c)) {
           fprintf(stderr, "Failed: codepoint_valid(%06x) -> true\n", c);
           error++;
        }
     }
     check(!error, "utf8proc_codepoint_valid FAILED %d tests.", error);
     printf("Validity tests SUCCEEDED.\n");

     return 0;
}
