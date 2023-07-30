#include "tests.h"

static int thunk_test = 1;

static utf8proc_int32_t custom(utf8proc_int32_t codepoint, void *thunk)
{
    check(((int *) thunk) == &thunk_test, "unexpected thunk passed");
    if (codepoint == 'a')
        return 'b';
    if (codepoint == 'S')
        return 0x00df; /* ß */
    return codepoint;
}

int main(void)
{
    utf8proc_uint8_t input[] = {0x41,0x61,0x53,0x62,0xef,0xbd,0x81,0x00}; /* "AaSb\uff41" */
    utf8proc_uint8_t correct[] = {0x61,0x62,0x73,0x73,0x62,0x61,0x00}; /* "abssba" */
    utf8proc_uint8_t *output;
    utf8proc_map_custom(input, 0, &output, UTF8PROC_CASEFOLD | UTF8PROC_COMPOSE | UTF8PROC_COMPAT | UTF8PROC_NULLTERM,
                        custom, &thunk_test);
    printf("mapped \"%s\" -> \"%s\"\n", (char*)input, (char*)output);
    check(strlen((char*) output) == 6, "incorrect output length");
    check(!memcmp(correct, output, 7), "incorrect output data");
    free(output);
    printf("map_custom tests SUCCEEDED.\n");
    return 0;
}
