#include <stdio.h>
#include <assert.h>
#include "jv_unicode.h"
#include "jv_utf8_tables.h"

const char* jvp_utf8_next(const char* in, const char* end, int* codepoint_ret) {
  assert(in <= end);
  if (in == end) {
    return 0;
  }
  int codepoint = -1;
  unsigned char first = (unsigned char)in[0];
  int length = utf8_coding_length[first];
  if ((first & 0x80) == 0) {
    /* Fast-path for ASCII */
    codepoint = first;
    length = 1;
  } else if (length == 0 || length == UTF8_CONTINUATION_BYTE) {
    /* Bad single byte - either an invalid byte or an out-of-place continuation byte */
    length = 1;
  } else if (in + length > end) {
    /* String ends before UTF8 sequence ends */
    length = end - in;
  } else {
    codepoint = ((unsigned)in[0]) & utf8_coding_bits[first];
    for (int i=1; i<length; i++) {
      unsigned ch = (unsigned char)in[i];
      if (utf8_coding_length[ch] != UTF8_CONTINUATION_BYTE){
        /* Invalid UTF8 sequence - not followed by the right number of continuation bytes */
        codepoint = -1;
        length = i;
        break;
      }
      codepoint = (codepoint << 6) | (ch & 0x3f);
    }
    if (codepoint < utf8_first_codepoint[length]) {
      /* Overlong UTF8 sequence */
      codepoint = -1;
    }
    if (0xD800 <= codepoint && codepoint <= 0xDFFF) {
      /* Surrogate codepoints can't be encoded in UTF8 */
      codepoint = -1;
    }
    if (codepoint > 0x10FFFF) {
      /* Outside Unicode range */
      codepoint = -1;
    }
  }
  assert(length > 0);
  *codepoint_ret = codepoint;
  return in + length;
}

int jvp_utf8_is_valid(const char* in, const char* end) {
  int codepoint;
  while ((in = jvp_utf8_next(in, end, &codepoint))) {
    if (codepoint == -1) return 0;
  }
  return 1;
}

int jvp_utf8_encode_length(int codepoint) {
  if (codepoint <= 0x7F) return 1;
  else if (codepoint <= 0x7FF) return 2;
  else if (codepoint <= 0xFFFF) return 3;
  else return 4;
}

int jvp_utf8_encode(int codepoint, char* out) {
  assert(codepoint >= 0 && codepoint <= 0x10FFFF);
  char* start = out;
  if (codepoint <= 0x7F) {
    *out++ = codepoint;
  } else if (codepoint <= 0x7FF) {
    *out++ = 0xC0 + ((codepoint & 0x7C0) >> 6);
    *out++ = 0x80 + ((codepoint & 0x03F));
  } else if(codepoint <= 0xFFFF) {
    *out++ = 0xE0 + ((codepoint & 0xF000) >> 12);
    *out++ = 0x80 + ((codepoint & 0x0FC0) >> 6);
    *out++ = 0x80 + ((codepoint & 0x003F));
  } else {
    *out++ = 0xF0 + ((codepoint & 0x1C0000) >> 18);
    *out++ = 0x80 + ((codepoint & 0x03F000) >> 12);
    *out++ = 0x80 + ((codepoint & 0x000FC0) >> 6);
    *out++ = 0x80 + ((codepoint & 0x00003F));
  }
  assert(out - start == jvp_utf8_encode_length(codepoint));
  return out - start;
}
