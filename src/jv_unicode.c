#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "jv_unicode.h"
#include "jv_utf8_tables.h"

// length of encoding of erroneous UTF-8 byte
#define UTF8_ERR_LEN 2
// length of encoding of erroneous UTF-16 surrogate
#define UTF16_ERR_LEN 3

#define U32(a, b, c, d) ( \
  (uint32_t) (a) << 0 | \
  (uint32_t) (b) << 8 | \
  (uint32_t) (c) << 16 | \
  (uint32_t) (d) << 24 \
)

#define BYTE(u32, n) ((uint32_t) (((u32) >> (n)*8) & 0xFF))

#define B0 0x00 // 00000000
#define B1 0x80 // 10000000
#define B2 0xC0 // 11000000
#define B3 0xE0 // 11100000
#define B4 0xF0 // 11110000
#define B5 0xF8 // 11111000

// NOTE: these flags are likely to be optimised out as `decode` gets inlined
enum decode_flags {
  DECODE_1 = 1,
  DECODE_2 = 2,
  DECODE_3 = 8,
  DECODE_4 = 16
};

// decode up to 4 bytes of "generalised UTF-8"; no checking for overlong
// codings or out-of-range code points, works by testing all fixed bits in each
// of the 4 coding patterns, then shifting the value bits according to the
// pattern
static int decode(enum decode_flags flags, uint32_t data, int* codepoint_ret) {
  if((flags & DECODE_1) && (data & U32(B1, B0, B0, B0)) == 0){
    *codepoint_ret = BYTE(data, 0);
    return 1;
  }
  if((flags & DECODE_2) && (data & U32(B3, B2, B0, B0)) == U32(B2, B1, B0, B0)){
    *codepoint_ret =
      (BYTE(data, 0) & ~B3) << 6 |
      (BYTE(data, 1) & ~B2) << 0;
    return 2;
  }
  if((flags & DECODE_3) && (data & U32(B4, B2, B2, B0)) == U32(B3, B1, B1, B0)){
    *codepoint_ret =
      (BYTE(data, 0) & ~B4) << 12 |
      (BYTE(data, 1) & ~B2) << 6 |
      (BYTE(data, 2) & ~B2) << 0;
    return 3;
  }
  if((flags & DECODE_4) && (data & U32(B5, B2, B2, B2)) == U32(B4, B1, B1, B1)){
    *codepoint_ret =
      (BYTE(data, 0) & ~B5) << 18 |
      (BYTE(data, 1) & ~B2) << 12 |
      (BYTE(data, 2) & ~B2) << 6 |
      (BYTE(data, 3) & ~B2) << 0;
    return 4;
  }
  *codepoint_ret = -1;
  return 1;
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
  uint32_t data = in[0] & 0xFF;
  if ((data & B1) == 0) {
    /* Fast-path for ASCII */
    *codepoint_ret = data;
    return in + 1;
  }
  switch (end - in) {
    default: // fall through
    case 4: data |= (uint32_t)(in[3] & 0xFF) << 24; // fall through
    case 3: data |= (uint32_t)(in[2] & 0xFF) << 16; // fall through
    case 2: data |= (uint32_t)(in[1] & 0xFF) << 8; // fall through
    case 1: break;
  }
  int codepoint;
  int length = decode(DECODE_2 | DECODE_3 | DECODE_4, data, &codepoint);
  if (codepoint == -1) {
    if (flags & JVP_UTF8_ERRORS_UTF8) assert(0 && "Invalid WTF-8b sequence: no match");
  } else if (codepoint < utf8_first_codepoint[length]) {
    /* Overlong UTF-8 sequence */
    if ((flags & JVP_UTF8_ERRORS_UTF8) && length == UTF8_ERR_LEN && 0x00 <= codepoint && codepoint <= 0x7F) {
      /* UTF-8 error is emitted as a negative codepoint */
      codepoint = -(codepoint + 0x80);
    } else {
      if (flags & JVP_UTF8_ERRORS_ALL) assert(0 && "Invalid WTF-8b sequence: overlong");
      codepoint = -1;
    }
  } else if (0xD800 <= codepoint && codepoint <= 0xDFFF) {
    /* Surrogate codepoints are allowed in WTF-8/WTF-8b */
    if (!(flags & JVP_UTF8_ERRORS_UTF16)) {
      /* Surrogate codepoints can't be encoded in UTF8 */
      codepoint = -1;
    }
  } else if (codepoint > 0x10FFFF) {
    /* Outside Unicode range */
    if (flags & JVP_UTF8_ERRORS_ALL) assert(0 && "Invalid WTF-8b sequence: out of range");
    codepoint = -1;
  }
  if (codepoint == -1 && (flags & JVP_UTF8_REPLACE))
    codepoint = 0xFFFD; // U+FFFD REPLACEMENT CHARACTER
  assert(length > 0);
  *codepoint_ret = codepoint;
  return in + length;
}

// assumes two bytes are readable from `in`
static int decode_utf8_error(const char* in) {
  uint32_t data = U32(in[0]  & 0xFF, in[1] & 0xFF, 0, 0);
  int codepoint;
  if (decode(DECODE_2, data, &codepoint) == UTF8_ERR_LEN && codepoint < 0x80)
    return codepoint + 0x80;
  return -1;
}

// assumes three bytes are readable from `in`
static int decode_utf16_error(const char* in) {
  uint32_t data = U32(in[0] & 0xFF, in[1] & 0xFF, in[2] & 0xFF, 0);
  int codepoint;
  if (decode(DECODE_3, data, &codepoint) == UTF16_ERR_LEN && codepoint >= 0xD800 && codepoint < 0xDFFF)
    return codepoint;
  return -1;
}

// jvp_utf8_wtf_join attempts to turn errors at the end of `a` and the
// beginning of `b` into a valid code point. if a correction is possible,
// `*alen_io`, `*bstart_io` and `*blen_io` are updated to exclude the existing
// errors, and the UTF-8 encoding of the code point to insert is stored in
// `out`. the number of bytes that should be inserted from `out` into the
// middle of the strings is returned (up to 4). this will be 0 if there are no
// bytes to insert.
int jvp_utf8_wtf_join(const char* astart, uint32_t* alen_io, const char** bstart_io, uint32_t* blen_io, char* out) {
  const char* aend = astart + *alen_io;
  const char* bstart = *bstart_io;
  const char* bend = bstart + *blen_io;
  int bcp;
  bstart = jvp_utf8_wtf_next(bstart, bend, JVP_UTF8_ERRORS_ALL, &bcp);
  if (!bstart) {
    // end of string
    return 0;
  }
  if (bcp >= 0xDC00 && bcp <= 0xDFFF) {
    // UTF-16 tail surrogate, look for lead surrogate at the end of `a`
    assert(bstart == *bstart_io + UTF16_ERR_LEN);
    if (aend - astart < UTF16_ERR_LEN)
      return 0;
    int acp = decode_utf16_error(aend - UTF16_ERR_LEN);
    if (acp >= 0xD800 && acp <= 0xDBFF) {
      // UTF-16 lead surrogate, decode matching UTF-16 pair
      *alen_io -= UTF16_ERR_LEN;
      *blen_io -= UTF16_ERR_LEN;
      *bstart_io += UTF16_ERR_LEN;
      int codepoint = 0x10000 + (((acp - 0xD800) << 10) | (bcp - 0xDC00));
      return jvp_utf8_encode(codepoint, out);
    }
    return 0;
  }
  if (bcp >= -0xFF && bcp <= -0x80) {
    // UTF-8 error, if it's a continuation byte, search backwards in `a` for the leading byte
    bcp = -bcp;
    assert(bstart == *bstart_io + UTF8_ERR_LEN);
    if (utf8_coding_length[bcp] != UTF8_CONTINUATION_BYTE)
      return 0;
    // if there's a correctable error, we will consume up to 4 encoded error bytes total, with up to 3 bytes from each of `a` and `b`
    unsigned char buf[6];
    unsigned char* bufstart = buf + 3;
    unsigned char* bufend = bufstart;
    *bufend++ = bcp;
    int length;
    // search backwards in `a` for a leading byte
    for (;;) {
      if (aend - astart < UTF8_ERR_LEN)
        return 0; // `a` is too short
      int acp = decode_utf8_error(aend - UTF8_ERR_LEN);
      if (acp == -1)
        return 0; // not a UTF-8 error
      aend -= UTF8_ERR_LEN;
      length = utf8_coding_length[acp];
      if (length == 0)
        return 0; // not a possible UTF-8 byte
      *--bufstart = acp;
      if (length != UTF8_CONTINUATION_BYTE)
        break; // found leading byte
      if (bufstart == buf)
        return 0; // too many continuation bytes
    }
    if (bufend - bufstart > length)
      return 0; // too many continuation bytes
    // search forwards in `b` for any more needed continuation bytes
    while (bufend - bufstart < length) {
      if (bend - bstart < UTF8_ERR_LEN)
        return 0; // `b` is too short
      bcp = decode_utf8_error(bstart);
      if (bcp == -1 || utf8_coding_length[bcp] != UTF8_CONTINUATION_BYTE)
        return 0; // not a UTF-8 error, didn't find enough continuation bytes
      bstart += UTF8_ERR_LEN;
      *bufend++ = bcp;
    }
    int codepoint;
    // check that the bytes are strict UTF-8
    jvp_utf8_wtf_next((char*)bufstart, (char*)bufend, 0, &codepoint);
    if (codepoint != -1) {
      memcpy(out, bufstart, 4);
      *alen_io = aend - astart;
      *blen_io = bend - bstart;
      *bstart_io = bstart;
      return bufend - bufstart;
    }
  }
  return 0;
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
