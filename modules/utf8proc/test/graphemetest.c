#include "tests.h"

/* check one line in the format of GraphemeBreakTest.txt */
void checkline(const char *_buf, bool verbose) {
    size_t bi = 0, si = 0;
    utf8proc_uint8_t src[1024]; /* more than long enough for all of our tests */
    const unsigned char *buf = (const unsigned char *) _buf;

    while (buf[bi]) {
        bi = skipspaces(buf, bi);
        if (buf[bi] == 0xc3 && buf[bi+1] == 0xb7) { /* U+00f7 = grapheme break */
            src[si++] = '/';
            bi += 2;
        }
        else if (buf[bi] == 0xc3 && buf[bi+1] == 0x97) { /* U+00d7 = no break */
            bi += 2;
        }
        else if (buf[bi] == '#') { /* start of comments */
            break;
        }
        else if (buf[bi] == '/') { /* for convenience, also accept / as grapheme break */
            src[si++] = '/';
            bi += 1;
        }
        else { /* hex-encoded codepoint */
            size_t len = encode((unsigned char*) (src + si), buf + bi) - 1;
            while (src[si]) ++si; /* advance to NUL termination */
            bi += len;
        }
    }
    if (si && src[si-1] == '/')
        --si; /* no break after final grapheme */
    src[si] = 0; /* NUL-terminate */

    if (si) { /* test utf8proc_map */
        utf8proc_uint8_t utf8[1024]; /* copy src without 0xff grapheme separators */
        size_t i = 0, j = 0;
        utf8proc_ssize_t glen, k;
        utf8proc_uint8_t *g; /* utf8proc_map grapheme results */
        while (i < si) {
            if (src[i] != '/')
                utf8[j++] = src[i++];
            else
                i++;
        }
        glen = utf8proc_map(utf8, (utf8proc_ssize_t)j, &g, UTF8PROC_CHARBOUND);
        if (glen == UTF8PROC_ERROR_INVALIDUTF8) {
            /* the test file contains surrogate codepoints, which are only for UTF-16 */
            printf("line %zd: ignoring invalid UTF-8 codepoints\n", lineno);
        }
        else {
            check(glen >= 0, "utf8proc_map error = %s",
                utf8proc_errmsg(glen));
            for (k = 0; k <= glen; ++k)
                if (g[k] == 0xff)
                    g[k] = '/'; /* easier-to-read output (/ is not in test strings) */
            check(!strcmp((char*)g, (char*)src),
                "grapheme mismatch: \"%s\" instead of \"%s\"", (char*)g, (char*)src);
        }
        free(g);
    }

    if (si) { /* test manual calls to utf8proc_grapheme_break_stateful */
        utf8proc_int32_t state = 0, prev_codepoint = 0;
        size_t i = 0;
        utf8proc_bool expectbreak = false;
        do {
            utf8proc_int32_t codepoint;
            i += (size_t)utf8proc_iterate(src + i, (utf8proc_ssize_t)(si - i), &codepoint);
            check(codepoint >= 0, "invalid UTF-8 data");
            if (codepoint == 0x002F)
                expectbreak = true;
            else {
                if (prev_codepoint != 0) {
                    check(expectbreak == utf8proc_grapheme_break_stateful(prev_codepoint, codepoint, &state),
                          "grapheme mismatch: between 0x%04x and 0x%04x in \"%s\"", prev_codepoint, codepoint, (char*) src);
                }
                expectbreak = false;
                prev_codepoint = codepoint;
            }
        } while (i < si);
    }

    if (verbose)
        printf("passed grapheme test: \"%s\"\n", (char*) src);
}

int main(int argc, char **argv)
{
    unsigned char buf[8192];
    FILE *f = argc > 1 ? fopen(argv[1], "r") : NULL;

    check(f != NULL, "error opening GraphemeBreakTest.txt");
    while (simple_getline(buf, f) > 0) {
        if ((++lineno) % 100 == 0)
            printf("checking line %zd...\n", lineno);
        if (buf[0] == '#') continue;
        checkline((char *) buf, false);
    }
    fclose(f);
    printf("Passed tests after %zd lines!\n", lineno);

    printf("Performing regression tests...\n");

    /* issue 144 */
    {
        utf8proc_uint8_t input[] = {0xef,0xbf,0xbf,0xef,0xbf,0xbe,0x00}; /* "\uffff\ufffe" */
        utf8proc_uint8_t output[] = {0xff,0xef,0xbf,0xbf,0xff,0xef,0xbf,0xbe,0x00}; /* with 0xff grapheme markers */
        utf8proc_ssize_t glen;
        utf8proc_uint8_t *g;
        glen = utf8proc_map(input, 6, &g, UTF8PROC_CHARBOUND);
        check(!strcmp((char*)g, (char*)output), "mishandled u+ffff and u+fffe grapheme breaks");
        check(glen != 6, "mishandled u+ffff and u+fffe grapheme breaks");
        free(g);
    };

    /* https://github.com/JuliaLang/julia/issues/37680 */
    checkline("/ 1f1f8 1f1ea / 1f1f8 1f1ea /", true); /* Two swedish flags after each other */
    checkline("/ 1f926 1f3fc 200d 2642 fe0f /", true); /* facepalm + pale skin + zwj + male sign + FE0F */
    checkline("/ 1f468 1f3fb 200d 1f91d 200d 1f468 1f3fd /", true); /* man face + pale skin + zwj + hand holding + zwj + man face + dark skin */

    check(utf8proc_grapheme_break(0x03b1, 0x03b2), "failed 03b1 / 03b2 test");
    check(!utf8proc_grapheme_break(0x03b1, 0x0302), "failed 03b1 0302 test");

    printf("Passed regression tests!\n");

    return 0;
}
