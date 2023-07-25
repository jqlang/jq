#ifndef JV_UNICODE_H
#define JV_UNICODE_H

#include <stdint.h>

enum jvp_utf8_flags {
  /* Emit replacement character instead of -1 for errors */
  JVP_UTF8_REPLACE = 1,
  /* Treat input as WTF-8b, emit 0xD800 to 0xDFFF to denote encoded UTF-16 errors */
  JVP_UTF8_ERRORS_UTF16 = 2,
  /* Treat input as WTF-8b, emit -0x80 to -0xFF to denote encoded UTF-8 errors */
  JVP_UTF8_ERRORS_UTF8 = 4,
  JVP_UTF8_ERRORS_ALL = JVP_UTF8_ERRORS_UTF16 | JVP_UTF8_ERRORS_UTF8
};

const char* jvp_utf8_wtf_next(const char* in, const char* end, enum jvp_utf8_flags flags, int* codepoint);
const char* jvp_utf8_next(const char* in, const char* end, int* codepoint);
const char* jvp_utf8_wtf_next_bytes(const char* in, const char* end, const char** bytes_out, uint32_t* bytes_len);
int jvp_utf8_wtf_join(const char* astart, uint32_t* alen, const char** bstart, uint32_t* blen, char* out);
int jvp_utf8_is_valid(const char* in, const char* end);

int jvp_utf8_decode_length(char startchar);

int jvp_utf8_encode_length(int codepoint);
int jvp_utf8_encode(int codepoint, char* out);
#endif
