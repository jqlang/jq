#include <assert.h>
#include "jv_unicode.h"
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
