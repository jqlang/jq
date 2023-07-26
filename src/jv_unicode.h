#ifndef JV_UNICODE_H
#define JV_UNICODE_H

#include <stdint.h>

const char* jvp_utf8_backtrack(const char* start, const char* min, int *missing_bytes);
const char* jvp_utf8_next(const char* in, const char* end, uint32_t *codepoint);
int jvp_utf8_is_valid(const char* in, const char* end);

int jvp_utf8_decode_length(char startchar);

int jvp_utf8_encode_length(uint32_t codepoint);
int jvp_utf8_encode(uint32_t codepoint, char* out);
#endif
