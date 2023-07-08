#include "tests.h"
#include <ctype.h>
#include <wchar.h>

static  int     tests;
static  int     error;

#define CHECKVALID(pos, val, len) buf[pos] = val; testbytes(buf,len,len,__LINE__)
#define CHECKINVALID(pos, val, len) buf[pos] = val; testbytes(buf,len,UTF8PROC_ERROR_INVALIDUTF8,__LINE__)

static void testbytes(utf8proc_uint8_t *buf, utf8proc_ssize_t len, utf8proc_ssize_t retval, int line)
{
    utf8proc_int32_t out[16];
    utf8proc_ssize_t ret;

    /* Make a copy to ensure that memory is left uninitialized after "len"
     * bytes. This way, Valgrind can detect overreads.
     */
    utf8proc_uint8_t tmp[16];
    memcpy(tmp, buf, (unsigned long int)len);

    tests++;
    if ((ret = utf8proc_iterate(tmp, len, out)) != retval) {
        fprintf(stderr, "Failed (%d):", line);
        for (utf8proc_ssize_t i = 0; i < len ; i++) {
            fprintf(stderr, " 0x%02x", tmp[i]);
        }
        fprintf(stderr, " -> %zd\n", ret);
        error++;
    }
}

int main(int argc, char **argv)
{
    utf8proc_int32_t byt;
    utf8proc_uint8_t buf[16];

    (void) argc; (void) argv; /* unused */

    tests = error = 0;

    // Check valid sequences that were considered valid erroneously before
    buf[0] = 0xef;
    buf[1] = 0xb7;
    for (byt = 0x90; byt < 0xa0; byt++) {
        CHECKVALID(2, byt, 3);
    }
    // Check 0xfffe and 0xffff
    buf[1] = 0xbf;
    CHECKVALID(2, 0xbe, 3);
    CHECKVALID(2, 0xbf, 3);
    // Check 0x??fffe & 0x??ffff
    for (byt = 0x1fffe; byt < 0x110000; byt += 0x10000) {
        buf[0] = 0xf0 | (byt >> 18);
        buf[1] = 0x80 | ((byt >> 12) & 0x3f);
        CHECKVALID(3, 0xbe, 4);
        CHECKVALID(3, 0xbf, 4);
    }

    // Continuation byte not after lead
    for (byt = 0x80; byt < 0xc0; byt++) {
        CHECKINVALID(0, byt, 1);
    }

    // Continuation byte not after lead
    for (byt = 0x80; byt < 0xc0; byt++) {
        CHECKINVALID(0, byt, 1);
    }

    // Test lead bytes
    for (byt = 0xc0; byt <= 0xff; byt++) {
	// Single lead byte at end of string
        CHECKINVALID(0, byt, 1);
        // Lead followed by non-continuation character < 0x80
        CHECKINVALID(1, 65, 2);
	// Lead followed by non-continuation character > 0xbf
        CHECKINVALID(1, 0xc0, 2);
    }

    // Test overlong 2-byte
    buf[0] = 0xc0;
    for (byt = 0x81; byt <= 0xbf; byt++) {
        CHECKINVALID(1, byt, 2);
    }
    buf[0] = 0xc1;
    for (byt = 0x80; byt <= 0xbf; byt++) {
        CHECKINVALID(1, byt, 2);
    }

    // Test overlong 3-byte
    buf[0] = 0xe0;
    buf[2] = 0x80;
    for (byt = 0x80; byt <= 0x9f; byt++) {
        CHECKINVALID(1, byt, 3);
    }

    // Test overlong 4-byte
    buf[0] = 0xf0;
    buf[2] = 0x80;
    buf[3] = 0x80;
    for (byt = 0x80; byt <= 0x8f; byt++) {
        CHECKINVALID(1, byt, 4);
    }

    // Test 4-byte > 0x10ffff
    buf[0] = 0xf4;
    buf[2] = 0x80;
    buf[3] = 0x80;
    for (byt = 0x90; byt <= 0xbf; byt++) {
        CHECKINVALID(1, byt, 4);
    }
    buf[1] = 0x80;
    for (byt = 0xf5; byt <= 0xf7; byt++) {
        CHECKINVALID(0, byt, 4);
    }

    // Test 5-byte
    buf[4] = 0x80;
    for (byt = 0xf8; byt <= 0xfb; byt++) {
        CHECKINVALID(0, byt, 5);
    }

    // Test 6-byte
    buf[5] = 0x80;
    for (byt = 0xfc; byt <= 0xfd; byt++) {
        CHECKINVALID(0, byt, 6);
    }

    // Test 7-byte
    buf[6] = 0x80;
    CHECKINVALID(0, 0xfe, 7);

    // Three and above byte sequences
    for (byt = 0xe0; byt < 0xf0; byt++) {
        // Lead followed by only 1 continuation byte
        CHECKINVALID(0, byt, 2);
        // Lead ended by non-continuation character < 0x80
        CHECKINVALID(2, 65, 3);
        // Lead ended by non-continuation character > 0xbf
        CHECKINVALID(2, 0xc0, 3);
    }

    // 3-byte encoded surrogate character(s)
    buf[0] = 0xed; buf[2] = 0x80;
    // Single surrogate
    CHECKINVALID(1, 0xa0, 3);
    // Trailing surrogate first
    CHECKINVALID(1, 0xb0, 3);

    // Four byte sequences
    buf[1] = 0x80;
    for (byt = 0xf0; byt < 0xf5; byt++) {
        // Lead followed by only 1 continuation bytes
        CHECKINVALID(0, byt, 2);
        // Lead followed by only 2 continuation bytes
        CHECKINVALID(0, byt, 3);
        // Lead followed by non-continuation character < 0x80
        CHECKINVALID(3, 65, 4);
        // Lead followed by non-continuation character > 0xbf
        CHECKINVALID(3, 0xc0, 4);

    }

     check(!error, "utf8proc_iterate FAILED %d tests out of %d", error, tests);
     printf("utf8proc_iterate tests SUCCEEDED, (%d) tests passed.\n", tests);

     return 0;
}
