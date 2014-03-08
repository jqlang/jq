#ifndef JV_H
#define JV_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

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

struct jv_refcnt;

/* All of the fields of this struct are private.
   Really. Do not play with them. */
typedef struct {
  unsigned char kind_flags;
  unsigned char pad_;
  unsigned short offset;  /* array offsets */
  int size;
  union {
    struct jv_refcnt* ptr;
    double number;
  } u;
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

int jv_get_refcnt(jv);

int jv_equal(jv, jv);
int jv_contains(jv, jv);

jv jv_invalid(void);
jv jv_invalid_with_msg(jv);
jv jv_invalid_get_msg(jv);
int jv_invalid_has_msg(jv);


jv jv_null(void);
jv jv_true(void);
jv jv_false(void);
jv jv_bool(int);

jv jv_number(double);
double jv_number_value(jv);
int jv_is_integer(jv);

jv jv_array(void);
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
jv jv_string_empty(int len);
int jv_string_length_bytes(jv);
int jv_string_length_codepoints(jv);
unsigned long jv_string_hash(jv);
const char* jv_string_value(jv);
jv jv_string_indexes(jv j, jv k);
jv jv_string_slice(jv j, int start, int end);
jv jv_string_concat(jv, jv);
jv jv_string_vfmt(const char*, va_list);
jv jv_string_fmt(const char*, ...);
jv jv_string_append_codepoint(jv a, uint32_t c);
jv jv_string_append_buf(jv a, const char* buf, int len);
jv jv_string_append_str(jv a, const char* str);
jv jv_string_split(jv j, jv sep);
jv jv_string_explode(jv j);
jv jv_string_implode(jv j);

jv jv_object(void);
jv jv_object_get(jv object, jv key);
jv jv_object_set(jv object, jv key, jv value);
jv jv_object_delete(jv object, jv key);
int jv_object_length(jv object);
jv jv_object_merge(jv, jv);
jv jv_object_merge_recursive(jv, jv);

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

enum { JV_PRINT_PRETTY = 1, JV_PRINT_ASCII = 2, JV_PRINT_COLOUR = 4, JV_PRINT_SORTED = 8 };
void jv_dumpf(jv, FILE *f, int flags);
void jv_dump(jv, int flags);
void jv_show(jv, int flags);
jv jv_dump_string(jv, int flags);

jv jv_parse(const char* string);
jv jv_parse_sized(const char* string, int length);

typedef void (*jv_nomem_handler_f)(void *);
void jv_nomem_handler(jv_nomem_handler_f, void *);

jv jv_load_file(const char *, int);

typedef enum {
  JV_PARSE_EXPLODE_TOPLEVEL_ARRAY = 1
} jv_parser_flags;
struct jv_parser;
struct jv_parser* jv_parser_new(jv_parser_flags);
void jv_parser_set_buf(struct jv_parser*, const char*, int, int);
jv jv_parser_next(struct jv_parser*);
void jv_parser_free(struct jv_parser*);

jv jv_get(jv, jv);
jv jv_set(jv, jv, jv);
jv jv_has(jv, jv);
jv jv_setpath(jv, jv, jv);
jv jv_getpath(jv, jv);
jv jv_delpaths(jv, jv);
jv jv_keys(jv /*object or array*/);
int jv_cmp(jv, jv);
jv jv_group(jv, jv);
jv jv_sort(jv, jv);


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
