#ifndef JV_H
#define JV_H

#include <stdint.h>
#include <assert.h>
#include <stddef.h>



typedef enum {
  JV_KIND_INVALID,
  JV_KIND_NULL,
  JV_KIND_FALSE,
  JV_KIND_TRUE,
  JV_KIND_NUMBER,
  JV_KIND_STRING,
  JV_KIND_ARRAY,
  JV_KIND_OBJECT
} jv_kind;

typedef struct {
  size_t count;
} jv_refcnt;

typedef struct{
  jv_refcnt* ptr;
  int i[2];
} jv_complex;

typedef struct {
  jv_kind kind;
  union {
    double number;
    jv_complex complex;
  } val;
} jv;

/*
 * All jv_* functions consume (decref) input and produce (incref) output
 * Except jv_copy
 */

jv_kind jv_get_kind(jv);
const char* jv_kind_name(jv_kind);
static int jv_is_valid(jv x) { return jv_get_kind(x) != JV_KIND_INVALID; }

jv jv_copy(jv);
void jv_free(jv);

int jv_equal(jv, jv);
int jv_contains(jv, jv);

jv jv_invalid();
jv jv_invalid_with_msg(jv);
jv jv_invalid_get_msg(jv);
int jv_invalid_has_msg(jv);


jv jv_null();
jv jv_true();
jv jv_false();
jv jv_bool(int);

jv jv_number(double);
double jv_number_value(jv);

jv jv_array();
jv jv_array_sized(int);
int jv_array_length(jv);
jv jv_array_get(jv, int);
jv jv_array_set(jv, int, jv);
jv jv_array_append(jv, jv);
jv jv_array_concat(jv, jv);
jv jv_array_slice(jv, int, int);


jv jv_string(const char*);
jv jv_string_sized(const char*, int);
int jv_string_length(jv);
uint32_t jv_string_hash(jv);
const char* jv_string_value(jv);
jv jv_string_concat(jv, jv);
jv jv_string_fmt(const char*, ...);
jv jv_string_append_buf(jv a, const char* buf, int len);
jv jv_string_append_str(jv a, const char* str);

jv jv_object();
jv jv_object_get(jv object, jv key);
jv jv_object_set(jv object, jv key, jv value);
jv jv_object_delete(jv object, jv key);
int jv_object_length(jv object);
jv jv_object_merge(jv, jv);

int jv_object_iter(jv);
int jv_object_iter_next(jv, int);
int jv_object_iter_valid(jv, int);
jv jv_object_iter_key(jv, int);
jv jv_object_iter_value(jv, int);
#define jv_object_foreach(i,t) \
  for (int i = jv_object_iter(t);               \
       jv_object_iter_valid(t, i);              \
       i = jv_object_iter_next(t, i))           \
 


int jv_get_refcnt(jv);

enum { JV_PRINT_PRETTY = 1, JV_PRINT_ASCII = 2 };
void jv_dump(jv, int flags);
jv jv_dump_string(jv, int flags);

jv jv_parse(const char* string);
jv jv_parse_sized(const char* string, int length);


static jv jv_lookup(jv t, jv k) {
  jv v;
  if (jv_get_kind(t) == JV_KIND_OBJECT && jv_get_kind(k) == JV_KIND_STRING) {
    v = jv_object_get(t, k);
    if (!jv_is_valid(v)) {
      jv_free(v);
      v = jv_null();
    }
  } else if (jv_get_kind(t) == JV_KIND_ARRAY && jv_get_kind(k) == JV_KIND_NUMBER) {
    // FIXME: don't do lookup for noninteger index
    v = jv_array_get(t, (int)jv_number_value(k));
    if (!jv_is_valid(v)) {
      jv_free(v);
      v = jv_null();
    }
  } else if (jv_get_kind(t) == JV_KIND_NULL && 
             (jv_get_kind(k) == JV_KIND_STRING || jv_get_kind(k) == JV_KIND_NUMBER)) {
    jv_free(t);
    jv_free(k);
    v = jv_null();
  } else {
    v = jv_invalid_with_msg(jv_string_fmt("Cannot index %s with %s",
                                          jv_kind_name(jv_get_kind(t)),
                                          jv_kind_name(jv_get_kind(k))));
    jv_free(t);
    jv_free(k);
  }
  return v;
}

static jv jv_modify(jv t, jv k, jv v) {
  int isnull = jv_get_kind(t) == JV_KIND_NULL;
  if (jv_get_kind(k) == JV_KIND_STRING && 
      (jv_get_kind(t) == JV_KIND_OBJECT || isnull)) {
    if (isnull) t = jv_object();
    t = jv_object_set(t, k, v);
  } else if (jv_get_kind(k) == JV_KIND_NUMBER &&
             (jv_get_kind(t) == JV_KIND_ARRAY || isnull)) {
    if (isnull) t = jv_array();
    t = jv_array_set(t, (int)jv_number_value(k), v);
  } else {
    jv err = jv_invalid_with_msg(jv_string_fmt("Cannot update field at %s index of %s",
                                               jv_kind_name(jv_get_kind(t)),
                                               jv_kind_name(jv_get_kind(v))));
    jv_free(t);
    jv_free(k);
    jv_free(v);
    t = err;
  }
  return t;
}

static jv jv_insert(jv root, jv value, jv* path, int pathlen) {
  if (pathlen == 0) {
    jv_free(root);
    return value;
  }
  return jv_modify(root, jv_copy(*path), 
                   jv_insert(jv_lookup(jv_copy(root), jv_copy(*path)), value, path+1, pathlen-1));
}









#endif


/*

true/false/null:
check kind

number:
introduce/eliminate jv
to integer

array:
copy
free
slice
index
update

updateslice?


 */
