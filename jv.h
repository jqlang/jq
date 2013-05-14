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
} jv_nontrivial;

typedef struct {
  jv_kind kind;
  union {
    double number;
    jv_nontrivial nontrivial;
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
#define jv_array_foreach(a, i, x) \
  for (int jv_len__ = jv_array_length(jv_copy(a)), i=0, jv_j__ = 1;     \
       jv_j__; jv_j__ = 0)                                              \
    for (jv x;                                                          \
         i < jv_len__ ?                                                 \
           (x = jv_array_get(jv_copy(a), i), 1) : 0;                    \
         i++)

jv jv_string(const char*);
jv jv_string_sized(const char*, int);
int jv_string_length_bytes(jv);
int jv_string_length_codepoints(jv);
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
#define jv_object_foreach(t, k, v)                                      \
  for (int jv_i__ = jv_object_iter(t), jv_j__ = 1; jv_j__; jv_j__ = 0)  \
    for (jv k, v;                                                       \
         jv_object_iter_valid((t), jv_i__) ?                            \
           (k = jv_object_iter_key(t, jv_i__),                          \
            v = jv_object_iter_value(t, jv_i__),                        \
            1)                                                          \
           : 0;                                                         \
         jv_i__ = jv_object_iter_next(t, jv_i__))                       \
 


int jv_get_refcnt(jv);

enum { JV_PRINT_PRETTY = 1, JV_PRINT_ASCII = 2, JV_PRINT_COLOUR = 4 };
void jv_dump(jv, int flags);
jv jv_dump_string(jv, int flags);

jv jv_parse(const char* string);
jv jv_parse_sized(const char* string, int length);










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
