#include "tests.h"
#include <ctype.h>
#include <wchar.h>

static int my_unassigned(int c) {
    int cat = utf8proc_get_property(c)->category;
    return (cat == UTF8PROC_CATEGORY_CN) || (cat == UTF8PROC_CATEGORY_CO);
}

static int my_isprint(int c) {
    int cat = utf8proc_get_property(c)->category;
    return (UTF8PROC_CATEGORY_LU <= cat && cat <= UTF8PROC_CATEGORY_ZS) ||
           (c == 0x0601 || c == 0x0602 || c == 0x0603 || c == 0x06dd || c == 0x00ad) ||
           (cat == UTF8PROC_CATEGORY_CN) || (cat == UTF8PROC_CATEGORY_CO);
}

int main(int argc, char **argv)
{
    int c, error = 0, updates = 0;

    (void) argc; /* unused */
    (void) argv; /* unused */

    /* some simple sanity tests of the character widths */
    for (c = 0; c <= 0x110000; ++c) {
        int cat = utf8proc_get_property(c)->category;
        int w = utf8proc_charwidth(c);
        if ((cat == UTF8PROC_CATEGORY_MN || cat == UTF8PROC_CATEGORY_ME) && w > 0) {
            fprintf(stderr, "nonzero width %d for combining char %x\n", w, c);
            error += 1;
        }
        if (w == 0 &&
            ((cat >= UTF8PROC_CATEGORY_LU && cat <= UTF8PROC_CATEGORY_LO) ||
             (cat >= UTF8PROC_CATEGORY_ND && cat <= UTF8PROC_CATEGORY_SC) ||
             (cat >= UTF8PROC_CATEGORY_SO && cat <= UTF8PROC_CATEGORY_ZS))) {
            fprintf(stderr, "zero width for symbol-like char %x\n", c);
            error += 1;
        }
        if (c <= 127 && ((!isprint(c) && w > 0) || (isprint(c) && wcwidth(c) != w))) {
            fprintf(stderr, "wcwidth %d mismatch %d for %s ASCII %x\n",
            wcwidth(c), w,
            isprint(c) ? "printable" : "non-printable", c);
            error += 1;
        }
        if (!my_isprint(c) && w > 0) {
            fprintf(stderr, "non-printing %x had width %d\n", c, w);
            error += 1;
        }
        if (my_unassigned(c) && w != 1) {
            fprintf(stderr, "unexpected width %d for unassigned char %x\n", w, c);
            error += 1;
        }
    }
    check(!error, "utf8proc_charwidth FAILED %d tests.", error);

    check(utf8proc_charwidth(0x00ad) == 1, "incorrect width for U+00AD (soft hyphen)");
    check(utf8proc_charwidth(0xe000) == 1, "incorrect width for U+e000 (PUA)");

    /* print some other information by compariing with system wcwidth */
    printf("Mismatches with system wcwidth (not necessarily errors):\n");
    for (c = 0; c <= 0x110000; ++c) {
        int w = utf8proc_charwidth(c);
        int wc = wcwidth(c);
        if (sizeof(wchar_t) == 2 && c >= (1<<16)) continue;
        /* lots of these errors for out-of-date system unicode tables */
        if (wc == -1 && my_isprint(c) && !my_unassigned(c) && w > 0)
            updates += 1;
        if (wc == -1 && !my_isprint(c) && w > 0)
            printf("  wcwidth(%x) = -1 for non-printable width-%d char\n", c, w);
        if (wc >= 0 && wc != w)
            printf("  wcwidth(%x) = %d != charwidth %d\n", c, wc, w);
    }
    printf("   ... (positive widths for %d chars unknown to wcwidth) ...\n", updates);
    printf("Character-width tests SUCCEEDED.\n");

    return 0;
}
