#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#include <float.h>

#include "jv_alloc.h"
#include "jv.h"
#include "jv_unicode.h"
#include "util.h"

/*
 * Internal refcounting helpers
 */

typedef struct jv_refcnt {
  int count;
} jv_refcnt;

static const jv_refcnt JV_REFCNT_INIT = {1};

static void jvp_refcnt_inc(jv_refcnt* c) {
  c->count++;
}

static int jvp_refcnt_dec(jv_refcnt* c) {
  c->count--;
  return c->count == 0;
}

static int jvp_refcnt_unshared(jv_refcnt* c) {
  assert(c->count > 0);
  return c->count == 1;
}

/*
 * Simple values (true, false, null)
 */

#define KIND_MASK 0xf

jv_kind jv_get_kind(jv x) {
  return x.kind_flags & KIND_MASK;
}

const char* jv_kind_name(jv_kind k) {
  switch (k) {
  case JV_KIND_INVALID: return "<invalid>";
  case JV_KIND_NULL:    return "null";
  case JV_KIND_FALSE:   return "boolean";
  case JV_KIND_TRUE:    return "boolean";
  case JV_KIND_NUMBER:  return "number";
  case JV_KIND_STRING:  return "string";
  case JV_KIND_ARRAY:   return "array";
  case JV_KIND_OBJECT:  return "object";
  }
  assert(0 && "invalid kind");
  return "<unknown>";
}

static const jv JV_NULL = {JV_KIND_NULL, 0, 0, 0, {0}};
static const jv JV_INVALID = {JV_KIND_INVALID, 0, 0, 0, {0}};
static const jv JV_FALSE = {JV_KIND_FALSE, 0, 0, 0, {0}};
static const jv JV_TRUE = {JV_KIND_TRUE, 0, 0, 0, {0}};

jv jv_true() {
  return JV_TRUE;
}

jv jv_false() {
  return JV_FALSE;
}

jv jv_null() {
  return JV_NULL;
}

jv jv_bool(int x) {
  return x ? JV_TRUE : JV_FALSE;
}

/*
 * Invalid objects, with optional error messages
 */

typedef struct {
  jv_refcnt refcnt;
  jv errmsg;
} jvp_invalid;

jv jv_invalid_with_msg(jv err) {
  if (jv_get_kind(err) == JV_KIND_NULL)
    return JV_INVALID;
  jvp_invalid* i = jv_mem_alloc(sizeof(jvp_invalid));
  i->refcnt = JV_REFCNT_INIT;
  i->errmsg = err;

  jv x = {JV_KIND_INVALID, 0, 0, 0, {&i->refcnt}};
  return x;
}

jv jv_invalid() {
  return JV_INVALID;
}

jv jv_invalid_get_msg(jv inv) {
  assert(jv_get_kind(inv) == JV_KIND_INVALID);
  jv x;
  if (inv.u.ptr == 0)
    x = jv_null();
  else
    x = jv_copy(((jvp_invalid*)inv.u.ptr)->errmsg);
  jv_free(inv);
  return x;
}

int jv_invalid_has_msg(jv inv) {
  jv msg = jv_invalid_get_msg(inv);
  int r = jv_get_kind(msg) != JV_KIND_NULL;
  jv_free(msg);
  return r;
}

static void jvp_invalid_free(jv x) {
  assert(jv_get_kind(x) == JV_KIND_INVALID);
  if (x.u.ptr != 0 && jvp_refcnt_dec(x.u.ptr)) {
    jv_free(((jvp_invalid*)x.u.ptr)->errmsg);
    jv_mem_free(x.u.ptr);
  }
}

/*
 * Numbers
 */

jv jv_number(double x) {
  jv j = {JV_KIND_NUMBER, 0, 0, 0, {.number = x}};
  return j;
}

double jv_number_value(jv j) {
  assert(jv_get_kind(j) == JV_KIND_NUMBER);
  return j.u.number;
}

int jv_is_integer(jv j){
  if(jv_get_kind(j) != JV_KIND_NUMBER){
    return 0;
  }
  double x = jv_number_value(j);

  double ipart;
  double fpart = modf(x, &ipart);

  return fabs(fpart) < DBL_EPSILON;
}

/*
 * Arrays (internal helpers)
 */

#define ARRAY_SIZE_ROUND_UP(n) (((n)*3)/2)

static int imax(int a, int b) {
  if (a>b) return a;
  else return b;
}

//FIXME signed vs unsigned
typedef struct {
  jv_refcnt refcnt;
  int length, alloc_length;
  jv elements[];
} jvp_array;

static jvp_array* jvp_array_ptr(jv a) {
  assert(jv_get_kind(a) == JV_KIND_ARRAY);
  return (jvp_array*)a.u.ptr;
}

static jvp_array* jvp_array_alloc(unsigned size) {
  jvp_array* a = jv_mem_alloc(sizeof(jvp_array) + sizeof(jv) * size);
  a->refcnt.count = 1;
  a->length = 0;
  a->alloc_length = size;
  return a;
}

static jv jvp_array_new(unsigned size) {
  jv r = {JV_KIND_ARRAY, 0, 0, 0, {&jvp_array_alloc(size)->refcnt}};
  return r;
}

static void jvp_array_free(jv a) {
  assert(jv_get_kind(a) == JV_KIND_ARRAY);
  if (jvp_refcnt_dec(a.u.ptr)) {
    jvp_array* array = jvp_array_ptr(a);
    for (int i=0; i<array->length; i++) {
      jv_free(array->elements[i]);
    }
    jv_mem_free(array);
  }
}

static int jvp_array_length(jv a) {
  assert(jv_get_kind(a) == JV_KIND_ARRAY);
  return a.size;
}

static int jvp_array_offset(jv a) {
  assert(jv_get_kind(a) == JV_KIND_ARRAY);
  return a.offset;
}

static jv* jvp_array_read(jv a, int i) {
  assert(jv_get_kind(a) == JV_KIND_ARRAY);
  if (i >= 0 && i < jvp_array_length(a)) {
    jvp_array* array = jvp_array_ptr(a);
    assert(i + jvp_array_offset(a) < array->length);
    return &array->elements[i + jvp_array_offset(a)];
  } else {
    return 0;
  }
}

static jv* jvp_array_write(jv* a, int i) {
  assert(i >= 0);
  jvp_array* array = jvp_array_ptr(*a);

  int pos = i + jvp_array_offset(*a);
  if (pos < array->alloc_length && jvp_refcnt_unshared(a->u.ptr)) {
    // use existing array space
    for (int j = array->length; j <= pos; j++) {
      array->elements[j] = JV_NULL;
    }
    array->length = imax(pos + 1, array->length);
    a->size = imax(i + 1, a->size);
    return &array->elements[pos];
  } else {
    // allocate a new array
    int new_length = imax(i + 1, jvp_array_length(*a));
    jvp_array* new_array = jvp_array_alloc(ARRAY_SIZE_ROUND_UP(new_length));
    int j;
    for (j = 0; j < jvp_array_length(*a); j++) {
      new_array->elements[j] =
        jv_copy(array->elements[j + jvp_array_offset(*a)]);
    }
    for (; j < new_length; j++) {
      new_array->elements[j] = JV_NULL;
    }
    new_array->length = new_length;
    jvp_array_free(*a);
    jv new_jv = {JV_KIND_ARRAY, 0, 0, new_length, {&new_array->refcnt}};
    *a = new_jv;
    return &new_array->elements[i];
  }
}

static int jvp_array_equal(jv a, jv b) {
  if (jvp_array_length(a) != jvp_array_length(b))
    return 0;
  if (jvp_array_ptr(a) == jvp_array_ptr(b) &&
      jvp_array_offset(a) == jvp_array_offset(b))
    return 1;
  for (int i=0; i<jvp_array_length(a); i++) {
    if (!jv_equal(jv_copy(*jvp_array_read(a, i)),
                  jv_copy(*jvp_array_read(b, i))))
      return 0;
  }
  return 1;
}

static void jvp_clamp_slice_params(int len, int *pstart, int *pend)
{
  if (*pstart < 0) *pstart = len + *pstart;
  if (*pend < 0) *pend = len + *pend;

  if (*pstart < 0) *pstart = 0;
  if (*pstart > len) *pstart = len;
  if (*pend > len) *pend = len;
  if (*pend < *pstart) *pend = *pstart;
}

static jv jvp_array_slice(jv a, int start, int end) {
  assert(jv_get_kind(a) == JV_KIND_ARRAY);
  int len = jvp_array_length(a);
  jvp_clamp_slice_params(len, &start, &end);
  assert(0 <= start && start <= end && end <= len);

  // FIXME: maybe slice should reallocate if the slice is small enough
  if (start == end) {
    jv_free(a);
    return jv_array();
  }

  if (a.offset + start >= 1 << (sizeof(a.offset) * CHAR_BIT)) {
    jv r = jv_array_sized(end - start);
    for (int i = start; i < end; i++)
      r = jv_array_append(r, jv_array_get(jv_copy(a), i));
    jv_free(a);
    return r;
  } else {
    a.offset += start;
    a.size = end - start;
    return a;
  }
}

/*
 * Arrays (public interface)
 */

jv jv_array_sized(int n) {
  return jvp_array_new(n);
}

jv jv_array() {
  return jv_array_sized(16);
}

int jv_array_length(jv j) {
  assert(jv_get_kind(j) == JV_KIND_ARRAY);
  int len = jvp_array_length(j);
  jv_free(j);
  return len;
}

jv jv_array_get(jv j, int idx) {
  assert(jv_get_kind(j) == JV_KIND_ARRAY);
  jv* slot = jvp_array_read(j, idx);
  jv val;
  if (slot) {
    val = jv_copy(*slot);
  } else {
    val = jv_invalid();
  }
  jv_free(j);
  return val;
}

jv jv_array_set(jv j, int idx, jv val) {
  assert(jv_get_kind(j) == JV_KIND_ARRAY);

  if (idx < 0)
    idx = jvp_array_length(j) + idx;
  if (idx < 0) {
    jv_free(j);
    jv_free(val);
    return jv_invalid_with_msg(jv_string("Out of bounds negative array index"));
  }
  // copy/free of val,j coalesced
  jv* slot = jvp_array_write(&j, idx);
  jv_free(*slot);
  *slot = val;
  return j;
}

jv jv_array_append(jv j, jv val) {
  // copy/free of val,j coalesced
  return jv_array_set(j, jv_array_length(jv_copy(j)), val);
}

jv jv_array_concat(jv a, jv b) {
  assert(jv_get_kind(a) == JV_KIND_ARRAY);
  assert(jv_get_kind(b) == JV_KIND_ARRAY);

  // FIXME: could be faster
  jv_array_foreach(b, i, elem) {
    a = jv_array_append(a, elem);
  }
  jv_free(b);
  return a;
}

jv jv_array_slice(jv a, int start, int end) {
  assert(jv_get_kind(a) == JV_KIND_ARRAY);
  // copy/free of a coalesced
  return jvp_array_slice(a, start, end);
}

int jv_array_contains(jv a, jv b) {
  int r = 1;
  jv_array_foreach(b, bi, belem) {
    int ri = 0;
    jv_array_foreach(a, ai, aelem) {
      if (jv_contains(aelem, jv_copy(belem))) {
        ri = 1;
        break;
      }
    }
    jv_free(belem);
    if (!ri) {
      r = 0;
      break;
    }
  }
  jv_free(a);
  jv_free(b);
  return r;
}

jv jv_array_indexes(jv a, jv b) {
  jv res = jv_array();
  int idx = -1;
  jv_array_foreach(a, ai, aelem) {
    jv_array_foreach(b, bi, belem) {
      // quieten compiler warnings about aelem not being used... by
      // using it
      if ((bi == 0 && !jv_equal(jv_copy(aelem), jv_copy(belem))) ||
          (bi > 0 && !jv_equal(jv_array_get(jv_copy(a), ai + bi), jv_copy(belem))))
        idx = -1;
      else if (bi == 0 && idx == -1)
        idx = ai;
    }
    if (idx > -1)
      res = jv_array_append(res, jv_number(idx));
    idx = -1;
  }
  jv_free(a);
  jv_free(b);
  return res;
}


/*
 * Strings (internal helpers)
 */

typedef struct {
  jv_refcnt refcnt;
  uint32_t hash;
  // high 31 bits are length, low bit is a flag
  // indicating whether hash has been computed.
  uint32_t length_hashed;
  uint32_t alloc_length;
  char data[];
} jvp_string;

static jvp_string* jvp_string_ptr(jv a) {
  assert(jv_get_kind(a) == JV_KIND_STRING);
  return (jvp_string*)a.u.ptr;
}

static jvp_string* jvp_string_alloc(uint32_t size) {
  jvp_string* s = jv_mem_alloc(sizeof(jvp_string) + size + 1);
  s->refcnt.count = 1;
  s->alloc_length = size;
  return s;
}

/* Copy a UTF8 string, replacing all badly encoded points with U+FFFD */
static jv jvp_string_copy_replace_bad(const char* data, uint32_t length) {
  const char* end = data + length;
  const char* i = data;
  const char* cstart;

  uint32_t maxlength = length * 3 + 1; // worst case: all bad bytes, each becomes a 3-byte U+FFFD
  jvp_string* s = jvp_string_alloc(maxlength);
  char* out = s->data;
  int c = 0;

  while ((i = jvp_utf8_next((cstart = i), end, &c))) {
    if (c == -1) {
      c = 0xFFFD; // U+FFFD REPLACEMENT CHARACTER
    }
    out += jvp_utf8_encode(c, out);
    assert(out < s->data + maxlength);
  }
  length = out - s->data;
  s->data[length] = 0;
  s->length_hashed = length << 1;
  jv r = {JV_KIND_STRING, 0, 0, 0, {&s->refcnt}};
  return r;
}

/* Assumes valid UTF8 */
static jv jvp_string_new(const char* data, uint32_t length) {
  jvp_string* s = jvp_string_alloc(length);
  s->length_hashed = length << 1;
  if (data != NULL)
    memcpy(s->data, data, length);
  s->data[length] = 0;
  jv r = {JV_KIND_STRING, 0, 0, 0, {&s->refcnt}};
  return r;
}

static jv jvp_string_empty_new(uint32_t length) {
  jvp_string* s = jvp_string_alloc(length);
  s->length_hashed = 0;
  memset(s->data, 0, length);
  jv r = {JV_KIND_STRING, 0, 0, 0, {&s->refcnt}};
  return r;
}


static void jvp_string_free(jv js) {
  jvp_string* s = jvp_string_ptr(js);
  if (jvp_refcnt_dec(&s->refcnt)) {
    jv_mem_free(s);
  }
}

static uint32_t jvp_string_length(jvp_string* s) {
  return s->length_hashed >> 1;
}

static uint32_t jvp_string_remaining_space(jvp_string* s) {
  assert(s->alloc_length >= jvp_string_length(s));
  uint32_t r = s->alloc_length - jvp_string_length(s);
  return r;
}

static jv jvp_string_append(jv string, const char* data, uint32_t len) {
  jvp_string* s = jvp_string_ptr(string);
  uint32_t currlen = jvp_string_length(s);

  if (jvp_refcnt_unshared(string.u.ptr) &&
      jvp_string_remaining_space(s) >= len) {
    // the next string fits at the end of a
    memcpy(s->data + currlen, data, len);
    s->data[currlen + len] = 0;
    s->length_hashed = (currlen + len) << 1;
    return string;
  } else {
    // allocate a bigger buffer and copy
    uint32_t allocsz = (currlen + len) * 2;
    if (allocsz < 32) allocsz = 32;
    jvp_string* news = jvp_string_alloc(allocsz);
    news->length_hashed = (currlen + len) << 1;
    memcpy(news->data, s->data, currlen);
    memcpy(news->data + currlen, data, len);
    news->data[currlen + len] = 0;
    jvp_string_free(string);
    jv r = {JV_KIND_STRING, 0, 0, 0, {&news->refcnt}};
    return r;
  }
}

static const uint32_t HASH_SEED = 0x432A9843;

static uint32_t rotl32 (uint32_t x, int8_t r){
  return (x << r) | (x >> (32 - r));
}

static uint32_t jvp_string_hash(jv jstr) {
  jvp_string* str = jvp_string_ptr(jstr);
  if (str->length_hashed & 1)
    return str->hash;

  /* The following is based on MurmurHash3.
     MurmurHash3 was written by Austin Appleby, and is placed
     in the public domain. */

  const uint8_t* data = (const uint8_t*)str->data;
  int len = (int)jvp_string_length(str);
  const int nblocks = len / 4;

  uint32_t h1 = HASH_SEED;

  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;
  const uint32_t* blocks = (const uint32_t *)(data + nblocks*4);

  for(int i = -nblocks; i; i++) {
    uint32_t k1 = blocks[i]; //FIXME: endianness/alignment

    k1 *= c1;
    k1 = rotl32(k1,15);
    k1 *= c2;

    h1 ^= k1;
    h1 = rotl32(h1,13);
    h1 = h1*5+0xe6546b64;
  }

  const uint8_t* tail = (const uint8_t*)(data + nblocks*4);

  uint32_t k1 = 0;

  switch(len & 3) {
  case 3: k1 ^= tail[2] << 16;
  case 2: k1 ^= tail[1] << 8;
  case 1: k1 ^= tail[0];
          k1 *= c1; k1 = rotl32(k1,15); k1 *= c2; h1 ^= k1;
  }

  h1 ^= len;

  h1 ^= h1 >> 16;
  h1 *= 0x85ebca6b;
  h1 ^= h1 >> 13;
  h1 *= 0xc2b2ae35;
  h1 ^= h1 >> 16;

  str->length_hashed |= 1;
  str->hash = h1;

  return h1;
}

static int jvp_string_equal(jv a, jv b) {
  assert(jv_get_kind(a) == JV_KIND_STRING);
  assert(jv_get_kind(b) == JV_KIND_STRING);
  jvp_string* stra = jvp_string_ptr(a);
  jvp_string* strb = jvp_string_ptr(b);
  if (jvp_string_length(stra) != jvp_string_length(strb)) return 0;
  return memcmp(stra->data, strb->data, jvp_string_length(stra)) == 0;
}

/*
 * Strings (public API)
 */

jv jv_string_sized(const char* str, int len) {
  return
    jvp_utf8_is_valid(str, str+len) ?
    jvp_string_new(str, len) :
    jvp_string_copy_replace_bad(str, len);
}

jv jv_string_empty(int len) {
  return jvp_string_empty_new(len);
}

jv jv_string(const char* str) {
  return jv_string_sized(str, strlen(str));
}

int jv_string_length_bytes(jv j) {
  assert(jv_get_kind(j) == JV_KIND_STRING);
  int r = jvp_string_length(jvp_string_ptr(j));
  jv_free(j);
  return r;
}

int jv_string_length_codepoints(jv j) {
  assert(jv_get_kind(j) == JV_KIND_STRING);
  const char* i = jv_string_value(j);
  const char* end = i + jv_string_length_bytes(jv_copy(j));
  int c = 0, len = 0;
  while ((i = jvp_utf8_next(i, end, &c))) len++;
  jv_free(j);
  return len;
}


jv jv_string_indexes(jv j, jv k) {
  assert(jv_get_kind(j) == JV_KIND_STRING);
  assert(jv_get_kind(k) == JV_KIND_STRING);
  const char *jstr = jv_string_value(j);
  const char *idxstr = jv_string_value(k);
  const char *p;
  int jlen = jv_string_length_bytes(jv_copy(j));
  int idxlen = jv_string_length_bytes(jv_copy(k));
  jv a = jv_array();

  if (idxlen != 0) {
    p = jstr;
    while ((p = _jq_memmem(p, (jstr + jlen) - p, idxstr, idxlen)) != NULL) {
      a = jv_array_append(a, jv_number(p - jstr));
      p += idxlen;
    }
  }
  jv_free(j);
  jv_free(k);
  return a;
}

jv jv_string_split(jv j, jv sep) {
  assert(jv_get_kind(j) == JV_KIND_STRING);
  assert(jv_get_kind(sep) == JV_KIND_STRING);
  const char *jstr = jv_string_value(j);
  const char *jend = jstr + jv_string_length_bytes(jv_copy(j));
  const char *sepstr = jv_string_value(sep);
  const char *p, *s;
  int seplen = jv_string_length_bytes(jv_copy(sep));
  jv a = jv_array();

  assert(jv_get_refcnt(a) == 1);

  if (seplen == 0) {
    int c;
    while ((jstr = jvp_utf8_next(jstr, jend, &c)))
      a = jv_array_append(a, jv_string_append_codepoint(jv_string(""), c));
  } else {
    for (p = jstr; p < jend; p = s + seplen) {
      s = _jq_memmem(p, jend - p, sepstr, seplen);
      if (s == NULL)
        s = jend;
      a = jv_array_append(a, jv_string_sized(p, s - p));
      // Add an empty string to denote that j ends on a sep
      if (s + seplen == jend && seplen != 0)
        a = jv_array_append(a, jv_string(""));
    }
  }
  jv_free(j);
  jv_free(sep);
  return a;
}

jv jv_string_explode(jv j) {
  assert(jv_get_kind(j) == JV_KIND_STRING);
  const char* i = jv_string_value(j);
  int len = jv_string_length_bytes(jv_copy(j));
  const char* end = i + len;
  jv a = jv_array_sized(len);
  int c;
  while ((i = jvp_utf8_next(i, end, &c)))
    a = jv_array_append(a, jv_number(c));
  jv_free(j);
  return a;
}

jv jv_string_implode(jv j) {
  assert(jv_get_kind(j) == JV_KIND_ARRAY);
  int len = jv_array_length(jv_copy(j));
  jv s = jv_string_empty(len);
  int i;

  assert(len >= 0);

  for (i = 0; i < len; i++) {
    jv n = jv_array_get(jv_copy(j), i);
    assert(jv_get_kind(n) == JV_KIND_NUMBER);
    int nv = jv_number_value(n);
    if (nv > 0x10FFFF)
      nv = 0xFFFD; // U+FFFD REPLACEMENT CHARACTER
    s = jv_string_append_codepoint(s, nv);
  }

  jv_free(j);
  return s;
}

unsigned long jv_string_hash(jv j) {
  assert(jv_get_kind(j) == JV_KIND_STRING);
  uint32_t hash = jvp_string_hash(j);
  jv_free(j);
  return hash;
}

const char* jv_string_value(jv j) {
  assert(jv_get_kind(j) == JV_KIND_STRING);
  return jvp_string_ptr(j)->data;
}

jv jv_string_slice(jv j, int start, int end) {
  assert(jv_get_kind(j) == JV_KIND_STRING);
  const char *s = jv_string_value(j);
  int len = jv_string_length_bytes(jv_copy(j));
  int i;
  const char *p, *e;
  int c;
  jv res;

  jvp_clamp_slice_params(len, &start, &end);
  assert(0 <= start && start <= end && end <= len);

  /* Look for byte offset corresponding to start codepoints */
  for (p = s, i = 0; i < start; i++) {
    p = jvp_utf8_next(p, s + len, &c);
    if (p == NULL) {
      jv_free(j);
      return jv_string_empty(16);
    }
    if (c == -1) {
      jv_free(j);
      return jv_invalid_with_msg(jv_string("Invalid UTF-8 string"));
    }
  }
  /* Look for byte offset corresponding to end codepoints */
  for (e = p; e != NULL && i < end; i++) {
    e = jvp_utf8_next(e, s + len, &c);
    if (e == NULL) {
      e = s + len;
      break;
    }
    if (c == -1) {
      jv_free(j);
      return jv_invalid_with_msg(jv_string("Invalid UTF-8 string"));
    }
  }

  /*
   * NOTE: Ideally we should do here what jvp_array_slice() does instead
   * of allocating a new string as we do!  However, we assume NUL-
   * terminated strings all over, and in the jv API, so for now we waste
   * memory like a drunken navy programmer.  There's probably nothing we
   * can do about it.
   */
  res = jv_string_sized(p, e - p);
  jv_free(j);
  return res;
}

jv jv_string_concat(jv a, jv b) {
  a = jvp_string_append(a, jv_string_value(b),
                        jvp_string_length(jvp_string_ptr(b)));
  jv_free(b);
  return a;
}

jv jv_string_append_buf(jv a, const char* buf, int len) {
  if (jvp_utf8_is_valid(buf, buf+len)) {
    a = jvp_string_append(a, buf, len);
  } else {
    jv b = jvp_string_copy_replace_bad(buf, len);
    a = jv_string_concat(a, b);
  }
  return a;
}

jv jv_string_append_codepoint(jv a, uint32_t c) {
  char buf[5];
  int len = jvp_utf8_encode(c, buf);
  a = jvp_string_append(a, buf, len);
  return a;
}

jv jv_string_append_str(jv a, const char* str) {
  return jv_string_append_buf(a, str, strlen(str));
}

jv jv_string_vfmt(const char* fmt, va_list ap) {
  int size = 1024;
  while (1) {
    char* buf = jv_mem_alloc(size);
    va_list ap2;
    va_copy(ap2, ap);
    int n = vsnprintf(buf, size, fmt, ap2);
    va_end(ap2);
    /*
     * NOTE: here we support old vsnprintf()s that return -1 because the
     * buffer is too small.
     */
    if (n >= 0 && n < size) {
      jv ret = jv_string_sized(buf, n);
      jv_mem_free(buf);
      return ret;
    } else {
      jv_mem_free(buf);
      size = (n > 0) ? /* standard */ (n * 2) : /* not standard */ (size * 2);
    }
  }
}

jv jv_string_fmt(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  jv res = jv_string_vfmt(fmt, args);
  va_end(args);
  return res;
}

/*
 * Objects (internal helpers)
 */

struct object_slot {
  int next; /* next slot with same hash, for collisions */
  uint32_t hash;
  jv string;
  jv value;
};

typedef struct {
  jv_refcnt refcnt;
  int next_free;
  struct object_slot elements[];
} jvp_object;


/* warning: nontrivial justification of alignment */
static jv jvp_object_new(int size) {
  // Allocates an object of (size) slots and (size*2) hash buckets.

  // size must be a power of two
  assert(size > 0 && (size & (size - 1)) == 0);

  jvp_object* obj = jv_mem_alloc(sizeof(jvp_object) +
                                 sizeof(struct object_slot) * size +
                                 sizeof(int) * (size * 2));
  obj->refcnt.count = 1;
  for (int i=0; i<size; i++) {
    obj->elements[i].next = i - 1;
    obj->elements[i].string = JV_NULL;
    obj->elements[i].hash = 0;
    obj->elements[i].value = JV_NULL;
  }
  obj->next_free = 0;
  int* hashbuckets = (int*)(&obj->elements[size]);
  for (int i=0; i<size*2; i++) {
    hashbuckets[i] = -1;
  }
  jv r = {JV_KIND_OBJECT, 0, 0, size, {&obj->refcnt}};
  return r;
}

static jvp_object* jvp_object_ptr(jv o) {
  assert(jv_get_kind(o) == JV_KIND_OBJECT);
  return (jvp_object*)o.u.ptr;
}

static uint32_t jvp_object_mask(jv o) {
  assert(jv_get_kind(o) == JV_KIND_OBJECT);
  return (o.size * 2) - 1;
}

static int jvp_object_size(jv o) {
  assert(jv_get_kind(o) == JV_KIND_OBJECT);
  return o.size;
}

static int* jvp_object_buckets(jv o) {
  return (int*)(&jvp_object_ptr(o)->elements[o.size]);
}

static int* jvp_object_find_bucket(jv object, jv key) {
  return jvp_object_buckets(object) + (jvp_object_mask(object) & jvp_string_hash(key));
}

static struct object_slot* jvp_object_get_slot(jv object, int slot) {
  assert(slot == -1 || (slot >= 0 && slot < jvp_object_size(object)));
  if (slot == -1) return 0;
  else return &jvp_object_ptr(object)->elements[slot];
}

static struct object_slot* jvp_object_next_slot(jv object, struct object_slot* slot) {
  return jvp_object_get_slot(object, slot->next);
}

static struct object_slot* jvp_object_find_slot(jv object, jv keystr, int* bucket) {
  uint32_t hash = jvp_string_hash(keystr);
  for (struct object_slot* curr = jvp_object_get_slot(object, *bucket);
       curr;
       curr = jvp_object_next_slot(object, curr)) {
    if (curr->hash == hash && jvp_string_equal(keystr, curr->string)) {
      return curr;
    }
  }
  return 0;
}

static struct object_slot* jvp_object_add_slot(jv object, jv key, int* bucket) {
  jvp_object* o = jvp_object_ptr(object);
  int newslot_idx = o->next_free;
  if (newslot_idx == jvp_object_size(object)) return 0;
  struct object_slot* newslot = jvp_object_get_slot(object, newslot_idx);
  o->next_free++;
  newslot->next = *bucket;
  *bucket = newslot_idx;
  newslot->hash = jvp_string_hash(key);
  newslot->string = key;
  return newslot;
}

static jv* jvp_object_read(jv object, jv key) {
  assert(jv_get_kind(key) == JV_KIND_STRING);
  int* bucket = jvp_object_find_bucket(object, key);
  struct object_slot* slot = jvp_object_find_slot(object, key, bucket);
  if (slot == 0) return 0;
  else return &slot->value;
}

static void jvp_object_free(jv o) {
  assert(jv_get_kind(o) == JV_KIND_OBJECT);
  if (jvp_refcnt_dec(o.u.ptr)) {
    for (int i=0; i<jvp_object_size(o); i++) {
      struct object_slot* slot = jvp_object_get_slot(o, i);
      if (jv_get_kind(slot->string) != JV_KIND_NULL) {
        jvp_string_free(slot->string);
        jv_free(slot->value);
      }
    }
    jv_mem_free(jvp_object_ptr(o));
  }
}

static jv jvp_object_rehash(jv object) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  assert(jvp_refcnt_unshared(object.u.ptr));
  int size = jvp_object_size(object);
  jv new_object = jvp_object_new(size * 2);
  for (int i=0; i<size; i++) {
    struct object_slot* slot = jvp_object_get_slot(object, i);
    if (jv_get_kind(slot->string) == JV_KIND_NULL) continue;
    int* new_bucket = jvp_object_find_bucket(new_object, slot->string);
    assert(!jvp_object_find_slot(new_object, slot->string, new_bucket));
    struct object_slot* new_slot = jvp_object_add_slot(new_object, slot->string, new_bucket);
    assert(new_slot);
    new_slot->value = slot->value;
  }
  // references are transported, just drop the old table
  jv_mem_free(jvp_object_ptr(object));
  return new_object;
}

static jv jvp_object_unshare(jv object) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  if (jvp_refcnt_unshared(object.u.ptr))
    return object;

  jv new_object = jvp_object_new(jvp_object_size(object));
  jvp_object_ptr(new_object)->next_free = jvp_object_ptr(object)->next_free;
  for (int i=0; i<jvp_object_size(new_object); i++) {
    struct object_slot* old_slot = jvp_object_get_slot(object, i);
    struct object_slot* new_slot = jvp_object_get_slot(new_object, i);
    *new_slot = *old_slot;
    if (jv_get_kind(old_slot->string) != JV_KIND_NULL) {
      new_slot->string = jv_copy(old_slot->string);
      new_slot->value = jv_copy(old_slot->value);
    }
  }

  int* old_buckets = jvp_object_buckets(object);
  int* new_buckets = jvp_object_buckets(new_object);
  memcpy(new_buckets, old_buckets, sizeof(int) * jvp_object_size(new_object)*2);

  jvp_object_free(object);
  assert(jvp_refcnt_unshared(new_object.u.ptr));
  return new_object;
}

static jv* jvp_object_write(jv* object, jv key) {
  *object = jvp_object_unshare(*object);
  int* bucket = jvp_object_find_bucket(*object, key);
  struct object_slot* slot = jvp_object_find_slot(*object, key, bucket);
  if (slot) {
    // already has the key
    jvp_string_free(key);
    return &slot->value;
  }
  slot = jvp_object_add_slot(*object, key, bucket);
  if (slot) {
    slot->value = jv_invalid();
  } else {
    *object = jvp_object_rehash(*object);
    bucket = jvp_object_find_bucket(*object, key);
    assert(!jvp_object_find_slot(*object, key, bucket));
    slot = jvp_object_add_slot(*object, key, bucket);
    assert(slot);
    slot->value = jv_invalid();
  }
  return &slot->value;
}

static int jvp_object_delete(jv* object, jv key) {
  assert(jv_get_kind(key) == JV_KIND_STRING);
  *object = jvp_object_unshare(*object);
  int* bucket = jvp_object_find_bucket(*object, key);
  int* prev_ptr = bucket;
  uint32_t hash = jvp_string_hash(key);
  for (struct object_slot* curr = jvp_object_get_slot(*object, *bucket);
       curr;
       curr = jvp_object_next_slot(*object, curr)) {
    if (hash == curr->hash && jvp_string_equal(key, curr->string)) {
      *prev_ptr = curr->next;
      jvp_string_free(curr->string);
      curr->string = JV_NULL;
      jv_free(curr->value);
      return 1;
    }
    prev_ptr = &curr->next;
  }
  return 0;
}

static int jvp_object_length(jv object) {
  int n = 0;
  for (int i=0; i<jvp_object_size(object); i++) {
    struct object_slot* slot = jvp_object_get_slot(object, i);
    if (jv_get_kind(slot->string) != JV_KIND_NULL) n++;
  }
  return n;
}

static int jvp_object_equal(jv o1, jv o2) {
  int len2 = jvp_object_length(o2);
  int len1 = 0;
  for (int i=0; i<jvp_object_size(o1); i++) {
    struct object_slot* slot = jvp_object_get_slot(o1, i);
    if (jv_get_kind(slot->string) == JV_KIND_NULL) continue;
    jv* slot2 = jvp_object_read(o2, slot->string);
    if (!slot2) return 0;
    // FIXME: do less refcounting here
    if (!jv_equal(jv_copy(slot->value), jv_copy(*slot2))) return 0;
    len1++;
  }
  return len1 == len2;
}

/*
 * Objects (public interface)
 */
#define DEFAULT_OBJECT_SIZE 8
jv jv_object() {
  return jvp_object_new(8);
}

jv jv_object_get(jv object, jv key) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  assert(jv_get_kind(key) == JV_KIND_STRING);
  jv* slot = jvp_object_read(object, key);
  jv val;
  if (slot) {
    val = jv_copy(*slot);
  } else {
    val = jv_invalid();
  }
  jv_free(object);
  jv_free(key);
  return val;
}

int jv_object_has(jv object, jv key) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  assert(jv_get_kind(key) == JV_KIND_STRING);
  jv* slot = jvp_object_read(object, key);
  int res = slot ? 1 : 0;
  jv_free(object);
  jv_free(key);
  return res;
}

jv jv_object_set(jv object, jv key, jv value) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  assert(jv_get_kind(key) == JV_KIND_STRING);
  // copy/free of object, key, value coalesced
  jv* slot = jvp_object_write(&object, key);
  jv_free(*slot);
  *slot = value;
  return object;
}

jv jv_object_delete(jv object, jv key) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  assert(jv_get_kind(key) == JV_KIND_STRING);
  jvp_object_delete(&object, key);
  jv_free(key);
  return object;
}

int jv_object_length(jv object) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  int n = jvp_object_length(object);
  jv_free(object);
  return n;
}

jv jv_object_merge(jv a, jv b) {
  assert(jv_get_kind(a) == JV_KIND_OBJECT);
  jv_object_foreach(b, k, v) {
    a = jv_object_set(a, k, v);
  }
  jv_free(b);
  return a;
}

jv jv_object_merge_recursive(jv a, jv b) {
  assert(jv_get_kind(a) == JV_KIND_OBJECT);
  assert(jv_get_kind(b) == JV_KIND_OBJECT);

  jv_object_foreach(b, k, v) {
    jv elem = jv_object_get(jv_copy(a), jv_copy(k));
    if (jv_is_valid(elem) &&
        jv_get_kind(elem) == JV_KIND_OBJECT &&
        jv_get_kind(v) == JV_KIND_OBJECT) {
      a = jv_object_set(a, k, jv_object_merge_recursive(elem, v));
    } else {
      jv_free(elem);
      a = jv_object_set(a, k, v);
    }
  }
  jv_free(b);
  return a;
}

int jv_object_contains(jv a, jv b) {
  assert(jv_get_kind(a) == JV_KIND_OBJECT);
  assert(jv_get_kind(b) == JV_KIND_OBJECT);
  int r = 1;

  jv_object_foreach(b, key, b_val) {
    jv a_val = jv_object_get(jv_copy(a), jv_copy(key));

    r = jv_contains(a_val, b_val);
    jv_free(key);

    if (!r) break;
  }

  jv_free(a);
  jv_free(b);
  return r;
}

/*
 * Object iteration (internal helpers)
 */

enum { ITER_FINISHED = -2 };

int jv_object_iter_valid(jv object, int i) {
  return i != ITER_FINISHED;
}

int jv_object_iter(jv object) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  return jv_object_iter_next(object, -1);
}

int jv_object_iter_next(jv object, int iter) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  assert(iter != ITER_FINISHED);
  struct object_slot* slot;
  do {
    iter++;
    if (iter >= jvp_object_size(object))
      return ITER_FINISHED;
    slot = jvp_object_get_slot(object, iter);
  } while (jv_get_kind(slot->string) == JV_KIND_NULL);
  assert(jv_get_kind(jvp_object_get_slot(object,iter)->string)
         == JV_KIND_STRING);
  return iter;
}

jv jv_object_iter_key(jv object, int iter) {
  jv s = jvp_object_get_slot(object, iter)->string;
  assert(jv_get_kind(s) == JV_KIND_STRING);
  return jv_copy(s);
}

jv jv_object_iter_value(jv object, int iter) {
  return jv_copy(jvp_object_get_slot(object, iter)->value);
}

/*
 * Memory management
 */
jv jv_copy(jv j) {
  if (jv_get_kind(j) == JV_KIND_ARRAY ||
      jv_get_kind(j) == JV_KIND_STRING ||
      jv_get_kind(j) == JV_KIND_OBJECT ||
      (jv_get_kind(j) == JV_KIND_INVALID && j.u.ptr != 0)) {
    jvp_refcnt_inc(j.u.ptr);
  }
  return j;
}

void jv_free(jv j) {
  if (jv_get_kind(j) == JV_KIND_ARRAY) {
    jvp_array_free(j);
  } else if (jv_get_kind(j) == JV_KIND_STRING) {
    jvp_string_free(j);
  } else if (jv_get_kind(j) == JV_KIND_OBJECT) {
    jvp_object_free(j);
  } else if (jv_get_kind(j) == JV_KIND_INVALID) {
    jvp_invalid_free(j);
  }
}

int jv_get_refcnt(jv j) {
  switch (jv_get_kind(j)) {
  case JV_KIND_ARRAY:
  case JV_KIND_STRING:
  case JV_KIND_OBJECT:
    return j.u.ptr->count;
  default:
    return 1;
  }
}

/*
 * Higher-level operations
 */

int jv_equal(jv a, jv b) {
  int r;
  if (jv_get_kind(a) != jv_get_kind(b)) {
    r = 0;
  } else if (jv_get_kind(a) == JV_KIND_NUMBER) {
    r = jv_number_value(a) == jv_number_value(b);
  } else if (a.kind_flags == b.kind_flags &&
             a.size == b.size &&
             a.u.ptr == b.u.ptr) {
    r = 1;
  } else {
    switch (jv_get_kind(a)) {
    case JV_KIND_ARRAY:
      r = jvp_array_equal(a, b);
      break;
    case JV_KIND_STRING:
      r = jvp_string_equal(a, b);
      break;
    case JV_KIND_OBJECT:
      r = jvp_object_equal(a, b);
      break;
    default:
      r = 1;
      break;
    }
  }
  jv_free(a);
  jv_free(b);
  return r;
}

int jv_identical(jv a, jv b) {
  int r;
  if (a.kind_flags != b.kind_flags
      || a.offset != b.offset
      || a.size != b.size) {
    r = 0;
  } else {
    switch (jv_get_kind(a)) {
    case JV_KIND_ARRAY:
    case JV_KIND_STRING:
    case JV_KIND_OBJECT:
      r = a.u.ptr == b.u.ptr;
      break;
    case JV_KIND_NUMBER:
      r = memcmp(&a.u.number, &b.u.number, sizeof(a.u.number)) == 0;
      break;
    default:
      r = 1;
      break;
    }
  }
  jv_free(a);
  jv_free(b);
  return r;
}

int jv_contains(jv a, jv b) {
  int r = 1;
  if (jv_get_kind(a) != jv_get_kind(b)) {
    r = 0;
  } else if (jv_get_kind(a) == JV_KIND_OBJECT) {
    r = jv_object_contains(jv_copy(a), jv_copy(b));
  } else if (jv_get_kind(a) == JV_KIND_ARRAY) {
    r = jv_array_contains(jv_copy(a), jv_copy(b));
  } else if (jv_get_kind(a) == JV_KIND_STRING) {
    int b_len = jv_string_length_bytes(jv_copy(b));
    if (b_len != 0) {
      r = _jq_memmem(jv_string_value(a), jv_string_length_bytes(jv_copy(a)),
                     jv_string_value(b), b_len) != 0;
    } else {
      r = 1;
    }
  } else {
    r = jv_equal(jv_copy(a), jv_copy(b));
  }
  jv_free(a);
  jv_free(b);
  return r;
}
