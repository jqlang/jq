#include <stdio.h>
#include <assert.h>
#include "jv_unicode.h"
#include "jv_utf8_tables.h"

// jvp_utf8_backtrack returns the beginning of the last codepoint in the
// string, assuming that start is the last byte in the string.
// If the last codepoint is incomplete, returns the number of missing bytes via
// *missing_bytes.  If there are no leading bytes or an invalid byte is
// encountered, NULL is returned and *missing_bytes is not altered.
const char* jvp_utf8_backtrack(const char* start, const char* min, int *missing_bytes) {
  assert(min <= start);
  if (min == start) {
    return min;
  }
  int length = 0;
  int seen = 1;
  while (start >= min && (length = utf8_coding_length[(unsigned char)*start]) == UTF8_CONTINUATION_BYTE) {
    start--;
    seen++;
  }
  if (length == 0 || length == UTF8_CONTINUATION_BYTE || length - seen < 0) {
    return NULL;
  }
  if (missing_bytes) *missing_bytes = length - seen;
  return start;
}

const char* jvp_utf8_next(const char* in, const char* end, int* codepoint_ret) {
  return jvp_utf8_wtf_next(in, end, JVP_UTF8_REPLACE, codepoint_ret);
}

// jvp_utf8_wtf_next_bytes iterates through chunks of UTF-8 bytes represented
// by a WTF-8b string. *bytes_out is set to the start of the current chunk and
// *bytes_len is set to the size of the current chunk. Valid sequences of UTF-8
// bytes are emitted as maximally sized chunks (pointing into the `in` string).
// Ill-formed UTF-8 bytes are emitted individually (pointing into a static
// array containing the byte). Ill-formed UTF-16 code units are emitted as
// UTF-8 replacement characters (pointing into a static array containing the
// bytes for U+FFFD).
const char* jvp_utf8_wtf_next_bytes(const char* in, const char* end, const char** bytes_out, uint32_t* bytes_len) {
  // U+FFFD REPLACEMENT CHARACTER
  static const unsigned char UTF8_REPLACEMENT[] = {0xEF,0xBF,0xBD};
  // array of bytes from 0x80 to 0xFF (inclusive)
  static const unsigned char UTF8_ILL_FORMED[] = {
    #define ROW(x) \
      x + 0, x + 1, x + 2, x + 3, \
      x + 4, x + 5, x + 6, x + 7, \
      x + 8, x + 9, x + 10, x + 11, \
      x + 12, x + 13, x + 14, x + 15
    ROW(0x80), ROW(0x90), ROW(0xA0), ROW(0xB0),
    ROW(0xC0), ROW(0xD0), ROW(0xE0), ROW(0xF0)
    #undef ROW
  };

  const char* i = in;
  const char* cstart;
  int c;

  while ((i = jvp_utf8_wtf_next((cstart = i), end, JVP_UTF8_ERRORS_ALL, &c))) {
    if (c >= -0xFF && c <= -0x80) {
      // invalid UTF-8 byte; pass through
      if (cstart > in) {
        // can't emit single byte yet; emit previous chunk first
        break;
      }
      *bytes_len = 1;
      *bytes_out = (const char*)&UTF8_ILL_FORMED[-c - 0x80];
      return i;
    }
    if (c >= 0xD800 && c <= 0xDFFF) {
      // lone surrogate; can't be encoded to UTF-8
      if (cstart > in) {
        // can't emit replacement bytes yet; emit previous chunk first
        break;
      }
      *bytes_len = sizeof UTF8_REPLACEMENT;
      *bytes_out = (const char*)UTF8_REPLACEMENT;
      return i;
    }
  }

  uint32_t len = cstart - in;
  *bytes_len = len;
  *bytes_out = in;
  return len == 0? NULL : cstart;
}

/*
  The internal representation of jv strings uses an encoding that is hereby
  referred to as "WTF-8b" (until someone demonstrates use of another term to
  refer to the same encoding).

  WTF-8b is an extension of WTF-8, which is an extension of UTF-8. Any sequence
  of Unicode scalar values is represented by the same bytes in UTF-8, WTF-8 and
  WTF-8b, therefore any well-formed UTF-8 string is interpreted as the same
  sequence of Unicode scalar values (roughly, code points) in WTF-8b.

  Like WTF-8, WTF-8b is able to encode UTF-16 errors (lone surrogates) using
  the "generalized UTF-8" representation of code points between U+D800 and
  U+DFFF. These errors occur in JSON terms such as:
    "_\uD8AB_\uDBCD_"

  Unlike WTF-8, WTF-8b is also able to encode UTF-8 errors (bytes 0x80 to 0xFF
  that are not part of a valid UTF-8 sequence) using the first 128 "overlong"
  codings (unused 2-byte representations of U+00 to U+7F). These errors can
  occur in any byte stream that is interpreted as UTF-8, for example:
    "\xED\xA2\xAB"
  The above example is in fact the WTF-8b (and WTF-8) encoding for the lone
  UTF-16 surrogate "\uD8AB", which demonstrates the need for a distinct
  encoding of UTF-8 errors. If a distinction were not made, then "\xED\xA2\xAB"
  and "\uD8AB" would be interpreted as the same string, so at least one of the
  forms would not be preserved when printed as JSON output.

  It should also be noted that the process of converting from invalid UTF-8 to
  WTF-8b is not (and can not be) idempotent, since the "generalized UTF-8"
  representation of UTF-16 surrogates is intentionally not able to be
  generated from invalid UTF-8, only through some other means (usually "\uXXXX"
  notation).

  Each UTF-16 error is encoded as 3 WTF-8b (or WTF-8) bytes.
  Each UTF-8 error is encoded as 2 WTF-8b bytes.

  When iterating over code points using `JVP_UTF8_ERRORS_UTF16`, encoded UTF-16
  errors are emitted in the form of code points in the range U+D800 to U+DFFF.
  These code points can be reencoded as usual using `jvp_utf8_encode`.

  When iterating over code points using `JVP_UTF8_ERRORS_UTF8`, encoded UTF-8
  errors are emitted in the form of code points in the negative range -0x80 to
  -0xFF. These negative code points can be negated to determine the original
  error bytes. These code points can be reencoded as usual using
  `jvp_utf8_encode`.
*/

const char* jvp_utf8_wtf_next(const char* in, const char* end, enum jvp_utf8_flags flags, int* codepoint_ret) {
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
    if (flags & JVP_UTF8_ERRORS_ALL) assert(0 && "Invalid WTF-8b sequence: bad single byte");
    length = 1;
  } else if (in + length > end) {
    /* String ends before UTF8 sequence ends */
    if (flags & JVP_UTF8_ERRORS_ALL) assert(0 && "Invalid WTF-8b sequence: underrun");
    length = end - in;
  } else {
    codepoint = ((unsigned)in[0]) & utf8_coding_bits[first];
    for (int i=1; i<length; i++) {
      unsigned ch = (unsigned char)in[i];
      if (utf8_coding_length[ch] != UTF8_CONTINUATION_BYTE){
        /* Invalid UTF8 sequence - not followed by the right number of continuation bytes */
        if (flags & JVP_UTF8_ERRORS_ALL) assert(0 && "Invalid WTF-8b sequence: wrong bytes");
        codepoint = -1;
        length = i;
        break;
      }
      codepoint = (codepoint << 6) | (ch & 0x3f);
    }
    if (codepoint < utf8_first_codepoint[length]) {
      /* Overlong UTF8 sequence */
      if ((flags & JVP_UTF8_ERRORS_UTF8) && 0x00 <= codepoint && codepoint <= 0x7F) {
        /* UTF-8 error is emitted as a negative codepoint */
        codepoint = -(codepoint + 0x80);
      } else {
        if (flags & JVP_UTF8_ERRORS_ALL) assert(0 && "Invalid WTF-8b sequence: overlong");
        codepoint = -1;
      }
    }
    if (0xD800 <= codepoint && codepoint <= 0xDFFF) {
      /* Surrogate codepoints are allowed in WTF-8/WTF-8b */
      if (!(flags & JVP_UTF8_ERRORS_UTF16)) {
        /* Surrogate codepoints can't be encoded in UTF8 */
        codepoint = -1;
      }
    }
    if (codepoint > 0x10FFFF) {
      /* Outside Unicode range */
      if (flags & JVP_UTF8_ERRORS_ALL) assert(0 && "Invalid WTF-8b sequence: out of range");
      codepoint = -1;
    }
  }
  if (codepoint == -1 && (flags & JVP_UTF8_REPLACE))
    codepoint = 0xFFFD; // U+FFFD REPLACEMENT CHARACTER
  assert(length > 0);
  *codepoint_ret = codepoint;
  return in + length;
}

int jvp_utf8_is_valid(const char* in, const char* end) {
  int codepoint;
  while ((in = jvp_utf8_wtf_next(in, end, 0, &codepoint))) {
    if (codepoint == -1) return 0;
  }
  return 1;
}

/* Assumes startchar is the first byte of a valid character sequence */
int jvp_utf8_decode_length(char startchar) {
	if ((startchar & 0x80) == 0) return 1;         // 0___ ____
	else if ((startchar & 0xE0) == 0xC0) return 2; // 110_ ____
	else if ((startchar & 0xF0) == 0xE0) return 3; // 1110 ____
	else return 4;                                 // 1111 ____
}

int jvp_utf8_encode_length(int codepoint) {
  if (codepoint >= 0 && codepoint <= 0x7F) return 1;
  else if (codepoint <= 0x7FF) return 2;
  else if (codepoint <= 0xFFFF) return 3;
  else return 4;
}

int jvp_utf8_encode(int codepoint, char* out) {
  assert((codepoint >= 0 && codepoint <= 0x10FFFF) || (codepoint >= -0xFF && codepoint <= -0x80));
  char* start = out;
  if (codepoint >= 0 && codepoint <= 0x7F) {
    *out++ = codepoint;
  } else if (codepoint <= 0x7FF) {
    // encode UTF-8 errors as overlong representations of U+00 to U+7F
    int cp = codepoint >= -0xFF && codepoint <= -0x80?
      -codepoint - 0x80 :
      codepoint;
    *out++ = 0xC0 + ((cp & 0x7C0) >> 6);
    *out++ = 0x80 + ((cp & 0x03F));
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
