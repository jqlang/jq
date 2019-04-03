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
int jv_identical(jv, jv);
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
jv jv_array_indexes(jv, jv);
#define jv_array_foreach(a, i, x) \
  for (int jv_len__ = jv_array_length(jv_copy(a)), i=0, jv_j__ = 1;     \
       jv_j__; jv_j__ = 0)                                              \
    for (jv x;                                                          \
         i < jv_len__ ?                                                 \
           (x = jv_array_get(jv_copy(a), i), 1) : 0;                    \
         i++)

#define JV_ARRAY_1(e) (jv_array_append(jv_array(),e))
#define JV_ARRAY_2(e1,e2) (jv_array_append(JV_ARRAY_1(e1),e2))
#define JV_ARRAY_3(e1,e2,e3) (jv_array_append(JV_ARRAY_2(e1,e2),e3))
#define JV_ARRAY_4(e1,e2,e3,e4) (jv_array_append(JV_ARRAY_3(e1,e2,e3),e4))
#define JV_ARRAY_5(e1,e2,e3,e4,e5) (jv_array_append(JV_ARRAY_4(e1,e2,e3,e4),e5))
#define JV_ARRAY_6(e1,e2,e3,e4,e5,e6) (jv_array_append(JV_ARRAY_5(e1,e2,e3,e4,e5),e6))
#define JV_ARRAY_7(e1,e2,e3,e4,e5,e6,e7) (jv_array_append(JV_ARRAY_6(e1,e2,e3,e4,e5,e6),e7))
#define JV_ARRAY_8(e1,e2,e3,e4,e5,e6,e7,e8) (jv_array_append(JV_ARRAY_7(e1,e2,e3,e4,e5,e6,e7),e8))
#define JV_ARRAY_9(e1,e2,e3,e4,e5,e6,e7,e8,e9) (jv_array_append(JV_ARRAY_8(e1,e2,e3,e4,e5,e6,e7,e8),e9))
#define JV_ARRAY_IDX(_1,_2,_3,_4,_5,_6,_7,_8,_9,NAME,...) NAME
#define JV_ARRAY(...) \
  JV_ARRAY_IDX(__VA_ARGS__, JV_ARRAY_9, JV_ARRAY_8, JV_ARRAY_7, JV_ARRAY_6, JV_ARRAY_5, JV_ARRAY_4, JV_ARRAY_3, JV_ARRAY_2, JV_ARRAY_1, dummy)(__VA_ARGS__)

#ifdef __GNUC__
#define JV_PRINTF_LIKE(fmt_arg_num, args_num) \
  __attribute__ ((__format__( __printf__, fmt_arg_num, args_num)))
#define JV_VPRINTF_LIKE(fmt_arg_num) \
  __attribute__ ((__format__( __printf__, fmt_arg_num, 0)))
#endif


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
jv jv_string_vfmt(const char*, va_list) JV_VPRINTF_LIKE(1);
jv jv_string_fmt(const char*, ...) JV_PRINTF_LIKE(1, 2);
jv jv_string_append_codepoint(jv a, uint32_t c);
jv jv_string_append_buf(jv a, const char* buf, int len);
jv jv_string_append_str(jv a, const char* str);
jv jv_string_split(jv j, jv sep);
jv jv_string_explode(jv j);
jv jv_string_implode(jv j);

jv jv_object(void);
jv jv_object_get(jv object, jv key);
int jv_object_has(jv object, jv key);
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

#define jv_object_keys_foreach(t, k)                                 \
  for (int jv_i__ = jv_object_iter(t), jv_j__ = 1; jv_j__; jv_j__ = 0)  \
    for (jv k;                                                          \
         jv_object_iter_valid((t), jv_i__) ?                            \
           (k = jv_object_iter_key(t, jv_i__),                          \
            1)                                                          \
           : 0;                                                         \
         jv_i__ = jv_object_iter_next(t, jv_i__))

#define JV_OBJECT_1(k1) (jv_object_set(jv_object(),(k1),jv_null()))
#define JV_OBJECT_2(k1,v1) (jv_object_set(jv_object(),(k1),(v1)))
#define JV_OBJECT_3(k1,v1,k2) (jv_object_set(JV_OBJECT_2((k1),(v1)),(k2),jv_null()))
#define JV_OBJECT_4(k1,v1,k2,v2) (jv_object_set(JV_OBJECT_2((k1),(v1)),(k2),(v2)))
#define JV_OBJECT_5(k1,v1,k2,v2,k3) (jv_object_set(JV_OBJECT_4((k1),(v1),(k2),(v2)),(k3),jv_null()))
#define JV_OBJECT_6(k1,v1,k2,v2,k3,v3) (jv_object_set(JV_OBJECT_4((k1),(v1),(k2),(v2)),(k3),(v3)))
#define JV_OBJECT_7(k1,v1,k2,v2,k3,v3,k4) (jv_object_set(JV_OBJECT_6((k1),(v1),(k2),(v2),(k3),(v3)),(k4),jv_null()))
#define JV_OBJECT_8(k1,v1,k2,v2,k3,v3,k4,v4) (jv_object_set(JV_OBJECT_6((k1),(v1),(k2),(v2),(k3),(v3)),(k4),(v4)))
#define JV_OBJECT_9(k1,v1,k2,v2,k3,v3,k4,v4,k5) \
    (jv_object_set(JV_OBJECT_8((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4)),(k5),jv_null()))
#define JV_OBJECT_10(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5) \
    (jv_object_set(JV_OBJECT_8((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4)),(k5),(v5)))
#define JV_OBJECT_11(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6) \
    (jv_object_set(JV_OBJECT_10((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5)),(k6),jv_null()))
#define JV_OBJECT_12(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6) \
    (jv_object_set(JV_OBJECT_10((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5)),(k6),(v6)))
#define JV_OBJECT_13(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7) \
    (jv_object_set(JV_OBJECT_12((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6)),(k7),jv_null()))
#define JV_OBJECT_14(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7) \
    (jv_object_set(JV_OBJECT_12((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6)),(k7),(v7)))
#define JV_OBJECT_15(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7,k8) \
    (jv_object_set(JV_OBJECT_14((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6),(k7),(v7)),(k8),jv_null()))
#define JV_OBJECT_16(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7,k8,v8) \
    (jv_object_set(JV_OBJECT_14((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6),(k7),(v7)),(k8),(v8)))
#define JV_OBJECT_17(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7,k8,v8,k9) \
    (jv_object_set(JV_OBJECT_16((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6),(k7),(v7),(k8),(v8)),(k9),jv_null()))
#define JV_OBJECT_18(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7,k8,v8,k9,v9) \
    (jv_object_set(JV_OBJECT_16((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6),(k7),(v7),(k8),(v8)),(k9),(v9)))
#define JV_OBJECT_IDX(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,NAME,...) NAME
#define JV_OBJECT(...) \
  JV_OBJECT_IDX(__VA_ARGS__, \
                JV_OBJECT_18, JV_OBJECT_17, JV_OBJECT_16, JV_OBJECT_15, \
                JV_OBJECT_14, JV_OBJECT_13, JV_OBJECT_12, JV_OBJECT_11, \
                JV_OBJECT_10, JV_OBJECT_9, JV_OBJECT_8, JV_OBJECT_7,    \
                JV_OBJECT_6, JV_OBJECT_5, JV_OBJECT_4, JV_OBJECT_3,     \
                JV_OBJECT_2, JV_OBJECT_1)(__VA_ARGS__)



int jv_get_refcnt(jv);

enum jv_print_flags {
  JV_PRINT_PRETTY   = 1,
  JV_PRINT_ASCII    = 2,
  JV_PRINT_COLOR    = 4, JV_PRINT_COLOUR = 4,
  JV_PRINT_SORTED   = 8,
  JV_PRINT_INVALID  = 16,
  JV_PRINT_REFCOUNT = 32,
  JV_PRINT_TAB      = 64,
  JV_PRINT_ISATTY   = 128,
  JV_PRINT_SPACE0   = 256,
  JV_PRINT_SPACE1   = 512,
  JV_PRINT_SPACE2   = 1024,
};
#define JV_PRINT_INDENT_FLAGS(n) \
    ((n) < 0 || (n) > 7 ? JV_PRINT_TAB | JV_PRINT_PRETTY : (n) == 0 ? 0 : (n) << 8 | JV_PRINT_PRETTY)
void jv_dumpf(jv, FILE *f, int flags);
void jv_dump(jv, int flags);
void jv_show(jv, int flags);
jv jv_dump_string(jv, int flags);
char *jv_dump_string_trunc(jv x, char *outbuf, size_t bufsize);

enum {
  JV_PARSE_SEQ              = 1,
  JV_PARSE_STREAMING        = 2,
  JV_PARSE_STREAM_ERRORS    = 4,
};

jv jv_parse(const char* string);
jv jv_parse_sized(const char* string, int length);

typedef void (*jv_nomem_handler_f)(void *);
void jv_nomem_handler(jv_nomem_handler_f, void *);

jv jv_load_file(const char *, int);

typedef struct jv_parser jv_parser;
jv_parser* jv_parser_new(int);
void jv_parser_set_buf(jv_parser*, const char*, int, int);
int jv_parser_remaining(jv_parser*);
jv jv_parser_next(jv_parser*);
void jv_parser_free(jv_parser*);

jv jv_get(jv, jv);
jv jv_set(jv, jv, jv);
jv jv_has(jv, jv);
jv jv_setpath(jv, jv, jv);
jv jv_getpath(jv, jv);
jv jv_delpaths(jv, jv);
jv jv_keys(jv /*object or array*/);
jv jv_keys_unsorted(jv /*object or array*/);
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
