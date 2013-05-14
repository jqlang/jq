#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "jv_alloc.h"
#include "jv.h"
#include "jv_unicode.h"

/*
 * Internal refcounting helpers
 */

static void jvp_refcnt_init(jv_nontrivial* c) {
  c->ptr->count = 1;
}

static void jvp_refcnt_inc(jv_nontrivial* c) {
  c->ptr->count++;
}

static int jvp_refcnt_dec(jv_nontrivial* c) {
  c->ptr->count--;
  return c->ptr->count == 0;
}

static int jvp_refcnt_unshared(jv_nontrivial* c) {
  assert(c->ptr->count > 0);
  return c->ptr->count == 1;
}

/*
 * Simple values (true, false, null)
 */

jv_kind jv_get_kind(jv x) {
  return x.kind;
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

static const jv JV_NULL = {JV_KIND_NULL, {0}};
static const jv JV_FALSE = {JV_KIND_FALSE, {0}};
static const jv JV_TRUE = {JV_KIND_TRUE, {0}};

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
  jv x;
  x.kind = JV_KIND_INVALID;
  x.val.nontrivial.i[0] = x.val.nontrivial.i[1] = 0;
  jvp_invalid* i = jv_mem_alloc(sizeof(jvp_invalid));
  x.val.nontrivial.ptr = &i->refcnt;
  i->refcnt.count = 1;
  i->errmsg = err;
  return x;
}

jv jv_invalid() {
  return jv_invalid_with_msg(jv_null());
}

jv jv_invalid_get_msg(jv inv) {
  jv x = jv_copy(((jvp_invalid*)inv.val.nontrivial.ptr)->errmsg);
  jv_free(inv);
  return x;
}

int jv_invalid_has_msg(jv inv) {
  jv msg = jv_invalid_get_msg(inv);
  int r = jv_get_kind(msg) != JV_KIND_NULL;
  jv_free(msg);
  return r;
}

static void jvp_invalid_free(jv_nontrivial* x) {
  if (jvp_refcnt_dec(x)) {
    jv_free(((jvp_invalid*)x->ptr)->errmsg);
    jv_mem_free(x->ptr);
  }
}

/*
 * Numbers
 */

jv jv_number(double x) {
  jv j;
  j.kind = JV_KIND_NUMBER;
  j.val.number = x;
  return j;
}

double jv_number_value(jv j) {
  assert(jv_get_kind(j) == JV_KIND_NUMBER);
  return j.val.number;
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

static jvp_array* jvp_array_ptr(jv_nontrivial* a) {
  return (jvp_array*)a->ptr;
}

static jvp_array* jvp_array_alloc(unsigned size) {
  jvp_array* a = jv_mem_alloc(sizeof(jvp_array) + sizeof(jv) * size);
  a->refcnt.count = 1;
  a->length = 0;
  a->alloc_length = size;
  return a;
}

static jv_nontrivial jvp_array_new(unsigned size) {
  jv_nontrivial r = {&jvp_array_alloc(size)->refcnt, {0, 0}};
  return r;
}

static void jvp_array_free(jv_nontrivial* a) {
  if (jvp_refcnt_dec(a)) {
    jvp_array* array = jvp_array_ptr(a);
    for (int i=0; i<array->length; i++) {
      jv_free(array->elements[i]);
    }
    jv_mem_free(array);
  }
}

static int jvp_array_length(jv_nontrivial* a) {
  return a->i[1] - a->i[0];
}

static jv* jvp_array_read(jv_nontrivial* a, int i) {
  if (i >= 0 && i < jvp_array_length(a)) {
    jvp_array* array = jvp_array_ptr(a);
    assert(i + a->i[0] < array->length);
    return &array->elements[i + a->i[0]];
  } else {
    return 0;
  }
}

static jv* jvp_array_write(jv_nontrivial* a, int i) {
  assert(i >= 0);
  jvp_array* array = jvp_array_ptr(a);

  int pos = i + a->i[0];
  if (pos < array->alloc_length) {
    // maybe we can update it in-place
    // FIXME: this "optimisation" can cause circular references
    #if 0
    int can_write_past_end = 
      array->length <= pos && /* the end of this array has never been used */
      a->i[1] == array->length; /* the current slice sees the end of the array */
    #endif
    int can_write_past_end = 0;
    if (can_write_past_end || jvp_refcnt_unshared(a)) {
      // extend the array
      for (int j = array->length; j <= pos; j++) {
        array->elements[j] = JV_NULL;
      }
      array->length = imax(pos + 1, array->length);
      a->i[1] = imax(pos + 1, a->i[1]);
      return &array->elements[pos];
    }
  }
  
  
  int new_length = imax(i + 1, jvp_array_length(a));
  jvp_array* new_array = jvp_array_alloc(ARRAY_SIZE_ROUND_UP(new_length));
  int j;
  for (j = 0; j < jvp_array_length(a); j++) {
    new_array->elements[j] = jv_copy(array->elements[j + a->i[0]]);
  }
  for (; j < new_length; j++) {
    new_array->elements[j] = JV_NULL;
  }
  new_array->length = new_length;
  jvp_array_free(a);
  a->ptr = &new_array->refcnt;
  a->i[0] = 0;
  a->i[1] = new_length;
  return &new_array->elements[i];
}

static int jvp_array_equal(jv_nontrivial* a, jv_nontrivial* b) {
  if (jvp_array_length(a) != jvp_array_length(b)) 
    return 0;
  if (jvp_array_ptr(a) == jvp_array_ptr(b) &&
      a->i[0] == b->i[0]) 
    return 1;
  for (int i=0; i<jvp_array_length(a); i++) {
    if (!jv_equal(jv_copy(*jvp_array_read(a, i)), 
                  jv_copy(*jvp_array_read(b,i))))
      return 0;
  }
  return 1;
}

static jv_nontrivial jvp_array_slice(jv_nontrivial* a, int start, int end) {
  // FIXME: maybe slice should reallocate if the slice is small enough
  assert(start <= end);
  assert(a->i[0] + end <= a->i[1]);
  jv_nontrivial slice = *a;
  slice.i[0] += start;
  slice.i[1] = slice.i[0] + (end - start);
  return slice;
}

/*
 * Arrays (public interface)
 */

jv jv_array_sized(int n) {
  jv j;
  j.kind = JV_KIND_ARRAY;
  j.val.nontrivial = jvp_array_new(n);
  return j;
}

jv jv_array() {
  return jv_array_sized(16);
}

int jv_array_length(jv j) {
  assert(jv_get_kind(j) == JV_KIND_ARRAY);
  int len = jvp_array_length(&j.val.nontrivial);
  jv_free(j);
  return len;
}

jv jv_array_get(jv j, int idx) {
  assert(jv_get_kind(j) == JV_KIND_ARRAY);
  jv* slot = jvp_array_read(&j.val.nontrivial, idx);
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
  // copy/free of val,j coalesced
  jv* slot = jvp_array_write(&j.val.nontrivial, idx);
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

  // FIXME: could be much faster
  jv_array_foreach(b, i, elem) {
    a = jv_array_append(a, elem);
  }
  jv_free(b);
  return a;
}

jv jv_array_slice(jv a, int start, int end) {
  assert(jv_get_kind(a) == JV_KIND_ARRAY);
  // copy/free of a coalesced
  a.val.nontrivial = jvp_array_slice(&a.val.nontrivial, start, end);
  return a;
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

static jvp_string* jvp_string_ptr(jv_nontrivial* a) {
  return (jvp_string*)a->ptr;
}

static jvp_string* jvp_string_alloc(uint32_t size) {
  jvp_string* s = jv_mem_alloc(sizeof(jvp_string) + size + 1);
  s->refcnt.count = 1;
  s->alloc_length = size;
  return s;
}

static jv_nontrivial jvp_string_new(const char* data, uint32_t length) {
  jvp_string* s = jvp_string_alloc(length);
  s->length_hashed = length << 1;
  memcpy(s->data, data, length);
  s->data[length] = 0;
  jv_nontrivial r = {&s->refcnt, {0,0}};
  return r;
}

static void jvp_string_free(jv_nontrivial* s) {
  if (jvp_refcnt_dec(s)) {
    jvp_string* str = jvp_string_ptr(s);
    jv_mem_free(str);
  }
}

static void jvp_string_free_p(jvp_string* s) {
  jv_nontrivial p = {&s->refcnt,{0,0}};
  jvp_string_free(&p);
}

static jvp_string* jvp_string_copy_p(jvp_string* s) {
  jv_nontrivial p = {&s->refcnt,{0,0}};
  jvp_refcnt_inc(&p);
  return s;
}

static uint32_t jvp_string_length(jvp_string* s) {
  return s->length_hashed >> 1;
}

static uint32_t jvp_string_remaining_space(jvp_string* s) {
  assert(s->alloc_length >= jvp_string_length(s));
  uint32_t r = s->alloc_length - jvp_string_length(s);
  return r;
}

static void jvp_string_append(jv_nontrivial* string, const char* data, uint32_t len) {
  jvp_string* s = jvp_string_ptr(string);
  uint32_t currlen = jvp_string_length(s);
    
  if (jvp_refcnt_unshared(string) &&
      jvp_string_remaining_space(s) >= len) {
    // the next string fits at the end of a
    memcpy(s->data + currlen, data, len);
    s->data[currlen + len] = 0;
    s->length_hashed = (currlen + len) << 1;
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
    jv_nontrivial r = {&news->refcnt, {0,0}};
    *string = r;
  }
}

static const uint32_t HASH_SEED = 0x432A9843;

static uint32_t rotl32 (uint32_t x, int8_t r){
  return (x << r) | (x >> (32 - r));
}

static uint32_t jvp_string_hash(jvp_string* str) {
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

static int jvp_string_equal_hashed(jvp_string* a, jvp_string* b) {
  assert(a->length_hashed & 1);
  assert(b->length_hashed & 1);
  if (a == b) return 1;
  if (a->hash != b->hash) return 0;
  if (a->length_hashed != b->length_hashed) return 0;
  return memcmp(a->data, b->data, jvp_string_length(a)) == 0;
}

static int jvp_string_equal(jv_nontrivial* a, jv_nontrivial* b) {
  jvp_string* stra = jvp_string_ptr(a);
  jvp_string* strb = jvp_string_ptr(b);
  if (jvp_string_length(stra) != jvp_string_length(strb)) return 0;
  return memcmp(stra->data, strb->data, jvp_string_length(stra)) == 0;
}

/*
 * Strings (public API)
 */

jv jv_string_sized(const char* str, int len) {
  jv j;
  j.kind = JV_KIND_STRING;
  j.val.nontrivial = jvp_string_new(str, len);
  return j;
}

jv jv_string(const char* str) {
  return jv_string_sized(str, strlen(str));
}

int jv_string_length_bytes(jv j) {
  assert(jv_get_kind(j) == JV_KIND_STRING);
  int r = jvp_string_length(jvp_string_ptr(&j.val.nontrivial));
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

uint32_t jv_string_hash(jv j) {
  assert(jv_get_kind(j) == JV_KIND_STRING);
  uint32_t hash = jvp_string_hash(jvp_string_ptr(&j.val.nontrivial));
  jv_free(j);
  return hash;
}

const char* jv_string_value(jv j) {
  assert(jv_get_kind(j) == JV_KIND_STRING);
  return jvp_string_ptr(&j.val.nontrivial)->data;
}

jv jv_string_concat(jv a, jv b) {
  jvp_string* sb = jvp_string_ptr(&b.val.nontrivial);
  jvp_string_append(&a.val.nontrivial, sb->data, jvp_string_length(sb));
  jv_free(b);
  return a;
}

jv jv_string_append_buf(jv a, const char* buf, int len) {
  jvp_string_append(&a.val.nontrivial, buf, len);
  return a;
}

jv jv_string_append_str(jv a, const char* str) {
  return jv_string_append_buf(a, str, strlen(str));
}
                        
jv jv_string_fmt(const char* fmt, ...) {
  int size = 1024;
  while (1) {
    char* buf = jv_mem_alloc(size);
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, size, fmt, args);
    va_end(args);
    if (n < size) {
      jv ret = jv_string_sized(buf, n);
      jv_mem_free(buf);
      return ret;
    } else {
      jv_mem_free(buf);
      size = n * 2;
    }
  }
}

/*
 * Objects (internal helpers)
 */

struct object_slot {
  int next;
  jvp_string* string;
  uint32_t hash;
  jv value;
};

typedef struct {
  jv_refcnt refcnt;
  int first_free;
  struct object_slot elements[];
} jvp_object;


/* warning: nontrivial justification of alignment */
static jv_nontrivial jvp_object_new(int size) {
  // Allocates an object of (size) slots and (size*2) hash buckets.

  // size must be a power of two
  assert(size > 0 && (size & (size - 1)) == 0);
  jvp_object* obj = jv_mem_alloc(sizeof(jvp_object) + 
                                 sizeof(struct object_slot) * size +
                                 sizeof(int) * (size * 2));
  obj->refcnt.count = 1;
  for (int i=0; i<size; i++) {
    obj->elements[i].next = i - 1;
    obj->elements[i].string = 0;
    obj->elements[i].hash = 0;
    obj->elements[i].value = JV_NULL;
  }
  obj->first_free = size - 1;
  int* hashbuckets = (int*)(&obj->elements[size]);
  jv_nontrivial r = {&obj->refcnt, 
                  {size*2 - 1, (char*)hashbuckets - (char*)obj}};
  for (int i=0; i<size*2; i++) {
    hashbuckets[i] = -1;
  }
  return r;
}

static jvp_object* jvp_object_ptr(jv_nontrivial* o) {
  return (jvp_object*)o->ptr;
}

static uint32_t jvp_object_mask(jv_nontrivial* o) {
  return o->i[0];
}

static int jvp_object_size(jv_nontrivial* o) {
  return (o->i[0] + 1) >> 1;
}

static int* jvp_object_buckets(jv_nontrivial* o) {
  int* buckets = (int*)((char*)o->ptr + o->i[1]);
  assert(buckets == (int*)&jvp_object_ptr(o)->elements[jvp_object_size(o)]);
  return buckets;
}

static int* jvp_object_find_bucket(jv_nontrivial* object, jvp_string* key) {
  return jvp_object_buckets(object) + (jvp_object_mask(object) & jvp_string_hash(key));
}

static struct object_slot* jvp_object_get_slot(jv_nontrivial* object, int slot) {
  assert(slot == -1 || (slot >= 0 && slot < jvp_object_size(object)));
  if (slot == -1) return 0;
  else return &jvp_object_ptr(object)->elements[slot];
}

static struct object_slot* jvp_object_next_slot(jv_nontrivial* object, struct object_slot* slot) {
  return jvp_object_get_slot(object, slot->next);
}

static struct object_slot* jvp_object_find_slot(jv_nontrivial* object, jvp_string* keystr, int* bucket) {
  for (struct object_slot* curr = jvp_object_get_slot(object, *bucket); 
       curr; 
       curr = jvp_object_next_slot(object, curr)) {
    if (jvp_string_equal_hashed(keystr, curr->string)) {
      return curr;
    }
  }
  return 0;
}

static struct object_slot* jvp_object_add_slot(jv_nontrivial* object, jvp_string* key, int* bucket) {
  jvp_object* o = jvp_object_ptr(object);
  int newslot_idx = o->first_free;
  struct object_slot* newslot = jvp_object_get_slot(object, newslot_idx);
  if (newslot == 0) return 0;
  o->first_free = newslot->next;
  newslot->next = *bucket;
  *bucket = newslot_idx;
  newslot->hash = jvp_string_hash(key);
  newslot->string = key;
  return newslot;
}

static void jvp_object_free_slot(jv_nontrivial* object, struct object_slot* slot) {
  jvp_object* o = jvp_object_ptr(object);
  slot->next = o->first_free;
  assert(slot->string);
  jvp_string_free_p(slot->string);
  slot->string = 0;
  jv_free(slot->value);
  o->first_free = slot - jvp_object_get_slot(object, 0);
}

static jv* jvp_object_read(jv_nontrivial* object, jvp_string* key) {
  int* bucket = jvp_object_find_bucket(object, key);
  struct object_slot* slot = jvp_object_find_slot(object, key, bucket);
  if (slot == 0) return 0;
  else return &slot->value;
}

static void jvp_object_free(jv_nontrivial* o) {
  if (jvp_refcnt_dec(o)) {
    for (int i=0; i<jvp_object_size(o); i++) {
      struct object_slot* slot = jvp_object_get_slot(o, i);
      if (slot->string) {
        jvp_string_free_p(slot->string);
        jv_free(slot->value);
      }
    }
    jv_mem_free(jvp_object_ptr(o));
  }
}

static void jvp_object_rehash(jv_nontrivial* object) {
  assert(jvp_refcnt_unshared(object));
  int size = jvp_object_size(object);
  jv_nontrivial new_object = jvp_object_new(size * 2);
  for (int i=0; i<size; i++) {
    struct object_slot* slot = jvp_object_get_slot(object, i);
    if (!slot->string) continue;
    
    int* new_bucket = jvp_object_find_bucket(&new_object, slot->string);
    assert(!jvp_object_find_slot(&new_object, slot->string, new_bucket));
    struct object_slot* new_slot = jvp_object_add_slot(&new_object, slot->string, new_bucket);
    assert(new_slot);
    new_slot->value = slot->value;
  }
  // references are transported, just drop the old table
  jv_mem_free(jvp_object_ptr(object));
  *object = new_object;
}

static void jvp_object_unshare(jv_nontrivial* object) {
  if (jvp_refcnt_unshared(object))
    return;

  jv_nontrivial new_object = jvp_object_new(jvp_object_size(object));
  jvp_object_ptr(&new_object)->first_free = jvp_object_ptr(object)->first_free;
  for (int i=0; i<jvp_object_size(&new_object); i++) {
    struct object_slot* old_slot = jvp_object_get_slot(object, i);
    struct object_slot* new_slot = jvp_object_get_slot(&new_object, i);
    *new_slot = *old_slot;
    if (old_slot->string) {
      new_slot->string = jvp_string_copy_p(old_slot->string);
      new_slot->value = jv_copy(old_slot->value);
    }
  }

  int* old_buckets = jvp_object_buckets(object);
  int* new_buckets = jvp_object_buckets(&new_object);
  memcpy(new_buckets, old_buckets, sizeof(int) * jvp_object_size(&new_object)*2);

  jvp_object_free(object);
  *object = new_object;
  assert(jvp_refcnt_unshared(object));
}

static jv* jvp_object_write(jv_nontrivial* object, jvp_string* key) {
  jvp_object_unshare(object);
  int* bucket = jvp_object_find_bucket(object, key);
  struct object_slot* slot = jvp_object_find_slot(object, key, bucket);
  if (slot) {
    // already has the key
    jvp_string_free_p(key);
    return &slot->value;
  }
  slot = jvp_object_add_slot(object, key, bucket);
  if (slot) {
    slot->value = jv_invalid();
  } else {
    jvp_object_rehash(object);
    bucket = jvp_object_find_bucket(object, key);
    assert(!jvp_object_find_slot(object, key, bucket));
    slot = jvp_object_add_slot(object, key, bucket);
    assert(slot);
    slot->value = jv_invalid();
  }
  return &slot->value;
}

static int jvp_object_delete(jv_nontrivial* object, jvp_string* key) {
  jvp_object_unshare(object);
  int* bucket = jvp_object_find_bucket(object, key);
  int* prev_ptr = bucket;
  for (struct object_slot* curr = jvp_object_get_slot(object, *bucket); 
       curr; 
       curr = jvp_object_next_slot(object, curr)) {
    if (jvp_string_equal_hashed(key, curr->string)) {
      *prev_ptr = curr->next;
      jvp_object_free_slot(object, curr);
      return 1;
    }
    prev_ptr = &curr->next;
  }
  return 0;
}

static int jvp_object_length(jv_nontrivial* object) {
  int n = 0;
  for (int i=0; i<jvp_object_size(object); i++) {
    struct object_slot* slot = jvp_object_get_slot(object, i);
    if (slot->string) n++;
  }
  return n;
}

static int jvp_object_equal(jv_nontrivial* o1, jv_nontrivial* o2) {
  int len2 = jvp_object_length(o2);
  int len1 = 0;
  for (int i=0; i<jvp_object_size(o1); i++) {
    struct object_slot* slot = jvp_object_get_slot(o1, i);
    if (!slot->string) continue;
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
  jv j;
  j.kind = JV_KIND_OBJECT;
  j.val.nontrivial = jvp_object_new(8);
  return j;
}

jv jv_object_get(jv object, jv key) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  assert(jv_get_kind(key) == JV_KIND_STRING);
  jv* slot = jvp_object_read(&object.val.nontrivial, jvp_string_ptr(&key.val.nontrivial));
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

jv jv_object_set(jv object, jv key, jv value) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  assert(jv_get_kind(key) == JV_KIND_STRING);
  // copy/free of object, key, value coalesced
  jv* slot = jvp_object_write(&object.val.nontrivial, jvp_string_ptr(&key.val.nontrivial));
  jv_free(*slot);
  *slot = value;
  return object;
}

jv jv_object_delete(jv object, jv key) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  assert(jv_get_kind(key) == JV_KIND_STRING);
  jvp_object_delete(&object.val.nontrivial, jvp_string_ptr(&key.val.nontrivial));
  jv_free(key);
  return object;
}

int jv_object_length(jv object) {
  assert(jv_get_kind(object) == JV_KIND_OBJECT);
  int n = jvp_object_length(&object.val.nontrivial);
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
  jv_nontrivial* o = &object.val.nontrivial;
  struct object_slot* slot;
  do {
    iter++;
    if (iter >= jvp_object_size(o)) 
      return ITER_FINISHED;
    slot = jvp_object_get_slot(o, iter);
  } while (!slot->string);
  return iter;
}

jv jv_object_iter_key(jv object, int iter) {
  jvp_string* s = jvp_object_get_slot(&object.val.nontrivial, iter)->string;
  assert(s);
  jv j;
  j.kind = JV_KIND_STRING;
  j.val.nontrivial.ptr = &s->refcnt;
  j.val.nontrivial.i[0] = 0;
  j.val.nontrivial.i[1] = 0;
  return jv_copy(j);
}

jv jv_object_iter_value(jv object, int iter) {
  return jv_copy(jvp_object_get_slot(&object.val.nontrivial, iter)->value);
}

/*
 * Memory management
 */
jv jv_copy(jv j) {
  if (jv_get_kind(j) == JV_KIND_ARRAY || 
      jv_get_kind(j) == JV_KIND_STRING || 
      jv_get_kind(j) == JV_KIND_OBJECT ||
      jv_get_kind(j) == JV_KIND_INVALID) {
    jvp_refcnt_inc(&j.val.nontrivial);
  }
  return j;
}

void jv_free(jv j) {
  if (jv_get_kind(j) == JV_KIND_ARRAY) {
    jvp_array_free(&j.val.nontrivial);
  } else if (jv_get_kind(j) == JV_KIND_STRING) {
    jvp_string_free(&j.val.nontrivial);
  } else if (jv_get_kind(j) == JV_KIND_OBJECT) {
    jvp_object_free(&j.val.nontrivial);
  } else if (jv_get_kind(j) == JV_KIND_INVALID) {
    jvp_invalid_free(&j.val.nontrivial);
  }
}

int jv_get_refcnt(jv j) {
  switch (jv_get_kind(j)) {
  case JV_KIND_ARRAY:
  case JV_KIND_STRING:
  case JV_KIND_OBJECT:
    return j.val.nontrivial.ptr->count;
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
  } else if (a.val.nontrivial.ptr == b.val.nontrivial.ptr &&
             a.val.nontrivial.i[0] == b.val.nontrivial.i[0] &&
             a.val.nontrivial.i[1] == b.val.nontrivial.i[1]) {
    r = 1;
  } else {
    switch (jv_get_kind(a)) {
    case JV_KIND_ARRAY:
      r = jvp_array_equal(&a.val.nontrivial, &b.val.nontrivial);
      break;
    case JV_KIND_STRING:
      r = jvp_string_equal(&a.val.nontrivial, &b.val.nontrivial);
      break;
    case JV_KIND_OBJECT:
      r = jvp_object_equal(&a.val.nontrivial, &b.val.nontrivial);
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
    r = strstr(jv_string_value(a), jv_string_value(b)) != 0;
  } else {
    r = jv_equal(jv_copy(a), jv_copy(b));
  }
  jv_free(a);
  jv_free(b);
  return r;
}
