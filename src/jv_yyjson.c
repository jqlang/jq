#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "jv_yyjson.h"
#include "jv_alloc.h"
#include "jv_dtoa.h"
#include "jv_dtoa_tsd.h"
#include "yyjson/src/yyjson.h"

/* Deeper documents are left to jv_parser, which enforces jq's depth limit. */
#define FAST_MAX_DEPTH 256

static int convert_number(yyjson_val *v, jv *out) {
  const char *raw = yyjson_get_raw(v);
  size_t n = yyjson_get_len(v);
  size_t i = 0;
  int neg = 0;
  if (n > 0 && raw[0] == '-') { neg = 1; i = 1; }
  if (n - i > 0 && n - i <= 15) {
    // Short plain integers are exactly representable; skip decNumber.
    // "-0" must stay on the literal path to keep its sign.
    int64_t acc = 0;
    size_t j;
    for (j = i; j < n && raw[j] >= '0' && raw[j] <= '9'; j++)
      acc = acc * 10 + (raw[j] - '0');
    if (j == n && !(neg && acc == 0)) {
      *out = jv_number(neg ? -(double)acc : (double)acc);
      return 1;
    }
  }
  // A plain decimal (digits and one '.', at most 15 digits) that survives a
  // round trip through a double and prints back as exactly the same text needs
  // no literal (decNumber) representation.  With <= 15 significant digits,
  // distinct decimals map to distinct doubles in the same order, so comparisons
  // between such numbers and literal ones are unaffected.
  int digits = 0, dots = 0, plain = 1;
  for (size_t j = i; j < n; j++) {
    if (raw[j] >= '0' && raw[j] <= '9') digits++;
    else if (raw[j] == '.') dots++;
    else { plain = 0; break; }
  }
  if (plain && dots == 1 && digits <= 15) {
    char tmp[32], fmt[64], *end;
    memcpy(tmp, raw, n);
    tmp[n] = 0;
    struct dtoa_context *C = tsd_dtoa_context_get();
    double d = jvp_strtod(C, tmp, &end);
    if (end == tmp + n && isfinite(d) && !strcmp(jvp_dtoa_fmt(C, fmt, d), tmp)) {
      *out = jv_number(d);
      return 1;
    }
  }
  char stackbuf[64];
  char *tmp = n < sizeof(stackbuf) ? stackbuf : jv_mem_alloc(n + 1);
  memcpy(tmp, raw, n);
  tmp[n] = 0;
  *out = jv_number_with_literal(tmp);
  if (tmp != stackbuf) jv_mem_free(tmp);
  return 1;
}

// Object keys repeat a lot (every record in an array of objects has the same
// ones).  Share one jv string per distinct short key; jq's string objects
// also cache their hash, so repeated keys are hashed once.
#define KEY_CACHE_SIZE 1024
#define KEY_CACHE_MAX_LEN 64
#if defined(__GNUC__) || defined(__clang__)
# define KEY_TLS __thread
#else
# define KEY_TLS
#endif
static KEY_TLS jv key_cache[KEY_CACHE_SIZE];
static KEY_TLS unsigned char key_cache_len[KEY_CACHE_SIZE]; // len + 1; 0 = empty

static jv make_key(yyjson_val *k) {
  const char *s = yyjson_get_str(k);
  size_t len = yyjson_get_len(k);
  if (len > KEY_CACHE_MAX_LEN)
    return jv_string_sized(s, (int)len);
  uint32_t h = 2166136261u;
  for (size_t i = 0; i < len; i++)
    h = (h ^ (unsigned char)s[i]) * 16777619u;
  unsigned slot = h & (KEY_CACHE_SIZE - 1);
  if (key_cache_len[slot] == len + 1 &&
      memcmp(jv_string_value(key_cache[slot]), s, len) == 0)
    return jv_copy(key_cache[slot]);
  jv key = jv_string_sized(s, (int)len);
  // Only cache keys that were already valid UTF-8 (no replacement happened).
  if ((size_t)jv_string_length_bytes(jv_copy(key)) == len) {
    if (key_cache_len[slot])
      jv_free(key_cache[slot]);
    key_cache[slot] = jv_copy(key);
    key_cache_len[slot] = (unsigned char)(len + 1);
  }
  return key;
}

static int convert(yyjson_val *v, int depth, jv *out) {
  if (depth > FAST_MAX_DEPTH) return 0;
  switch (yyjson_get_type(v)) {
  case YYJSON_TYPE_NULL: *out = jv_null(); return 1;
  case YYJSON_TYPE_BOOL: *out = yyjson_get_bool(v) ? jv_true() : jv_false(); return 1;
  case YYJSON_TYPE_NUM:  return convert_number(v, out);
  case YYJSON_TYPE_RAW:  return convert_number(v, out);
  case YYJSON_TYPE_STR:
    *out = jv_string_sized(yyjson_get_str(v), (int)yyjson_get_len(v));
    return 1;
  case YYJSON_TYPE_ARR: {
    size_t idx, max;
    yyjson_val *e;
    jv arr = jv_array_sized((int)yyjson_arr_size(v));
    yyjson_arr_foreach(v, idx, max, e) {
      jv x;
      if (!convert(e, depth + 1, &x)) { jv_free(arr); return 0; }
      arr = jv_array_append(arr, x);
    }
    *out = arr;
    return 1;
  }
  case YYJSON_TYPE_OBJ: {
    size_t idx, max;
    yyjson_val *k, *e;
    jv obj = jv_object();
    yyjson_obj_foreach(v, idx, max, k, e) {
      jv x;
      if (!convert(e, depth + 1, &x)) { jv_free(obj); return 0; }
      obj = jv_object_set(obj, make_key(k), x);
    }
    *out = obj;
    return 1;
  }
  default:
    return 0;
  }
}

// Flags: numbers are kept as raw text so jq's literal-preserving numbers
// (big integers, 1.000, 1E2, ...) behave exactly as with jv_parser.
static const yyjson_read_flag BASE_FLAGS = YYJSON_READ_NUMBER_AS_RAW;

int jv_yyjson_enabled(void) {
  static int state = -1;
  if (state < 0) {
    const char *e = getenv("JQ_NO_YYJSON");
    state = !(e && *e && strcmp(e, "0"));
  }
  return state;
}

int jv_yyjson_parse_all(const char *s, size_t len, jv *out) {
  if (!jv_yyjson_enabled()) return 0;
  if (len > (size_t)INT32_MAX) return 0;
  yyjson_doc *doc = yyjson_read(s, len, BASE_FLAGS);
  if (!doc) return 0;
  int ok = convert(yyjson_doc_get_root(doc), 0, out);
  yyjson_doc_free(doc);
  return ok;
}

int jv_yyjson_parse_next(char *buf, size_t len, size_t *consumed, jv *out) {
  if (len > (size_t)INT32_MAX) return 0;
  yyjson_doc *doc = yyjson_read_opts(buf, len, BASE_FLAGS | YYJSON_READ_INSITU | YYJSON_READ_STOP_WHEN_DONE, NULL, NULL);
  if (!doc) return 0;
  size_t used = yyjson_doc_get_read_size(doc);
  yyjson_val *root = yyjson_doc_get_root(doc);
  int ok = convert(root, 0, out);
  yyjson_doc_free(doc);
  if (!ok) return 0;
  *consumed = used;
  return 1;
}
