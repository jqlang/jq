#ifndef JV_UNICODE_H
#define JV_UNICODE_H

#include <stdint.h>
#include <string.h>

// Word-at-a-time ("SWAR") byte tests on 8 bytes packed in a uint64_t.
// Each returns nonzero iff at least one byte of the word matches; they say
// nothing about which byte, so callers finish a matching word bytewise.
// This keeps them independent of byte order.
#define JVP_SWAR_ONES  UINT64_C(0x0101010101010101)
#define JVP_SWAR_HIGHS UINT64_C(0x8080808080808080)

static inline uint64_t jvp_swar_load(const char* p) {
  uint64_t w;
  memcpy(&w, p, sizeof(w));
  return w;
}

// Some byte of w is less than n; exact for n <= 128.
static inline uint64_t jvp_swar_has_less(uint64_t w, unsigned char n) {
  return (w - JVP_SWAR_ONES * n) & ~w & JVP_SWAR_HIGHS;
}

// Some byte of w equals c.
static inline uint64_t jvp_swar_has_byte(uint64_t w, unsigned char c) {
  return jvp_swar_has_less(w ^ (JVP_SWAR_ONES * c), 1);
}

const char* jvp_utf8_backtrack(const char* start, const char* min, int *missing_bytes);
const char* jvp_utf8_next(const char* in, const char* end, int* codepoint);
int jvp_utf8_is_valid(const char* in, const char* end);

int jvp_utf8_decode_length(char startchar);

int jvp_utf8_encode_length(int codepoint);
int jvp_utf8_encode(int codepoint, char* out);

int jvp_codepoint_is_whitespace(int c);
#endif
