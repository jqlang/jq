#ifndef JV_UNICODE_H
#define JV_UNICODE_H

#include <assert.h>

const char* jvp_utf8_backtrack(const char* start, const char* min, int *missing_bytes);
const char* jvp_utf8_next_multibyte(const char* in, const char* end, int* codepoint);
int jvp_utf8_is_valid(const char* in, const char* end);

// Decode one codepoint.  ASCII is handled here so that callers looping over
// a string do not make a call per byte; everything else goes to
// jvp_utf8_next_multibyte().
static inline const char* jvp_utf8_next(const char* in, const char* end, int* codepoint) {
  if (in >= end) {
    assert(in == end);
    return 0;
  }
  if (!(*in & 0x80)) {
    *codepoint = (unsigned char)*in;
    return in + 1;
  }
  return jvp_utf8_next_multibyte(in, end, codepoint);
}

int jvp_utf8_decode_length(char startchar);

int jvp_utf8_encode_length(int codepoint);
int jvp_utf8_encode(int codepoint, char* out);

int jvp_codepoint_is_whitespace(int c);
#endif
