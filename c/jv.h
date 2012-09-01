#ifndef JV_H
#define JV_H

#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#ifndef NO_JANSSON
#include <jansson.h>
static json_t* jv_lookup(json_t* t, json_t* k) {
  json_t* v;
  if (json_is_object(t) && json_is_string(k)) {
    v = json_object_get(t, json_string_value(k));
  } else if (json_is_array(t) && json_is_number(k)) {
    v = json_array_get(t, json_integer_value(k));
  } else {
    assert(0&&"bad lookup");
  }
  if (v) 
    return v;
  else
    return json_null();
}

static json_t* jv_modify(json_t* t, json_t* k, json_t* v) {
  t = json_copy(t);
  if (json_is_object(t) && json_is_string(k)) {
    json_object_set(t, json_string_value(k), v);
  } else if (json_is_array(t) && json_is_number(k)) {
    json_array_set(t, json_integer_value(k), v);
  } else {
    assert(0 && "bad mod");
  }
  return t;
}

static json_t* jv_insert(json_t* root, json_t* value, json_t** path, int pathlen) {
  if (pathlen == 0) {
    return value;
  }
  return jv_modify(root, *path, jv_insert(jv_lookup(root, *path), value, path+1, pathlen-1));
}
#endif



typedef enum {
  JV_KIND_NULL,
  JV_KIND_FALSE,
  JV_KIND_TRUE,
  JV_KIND_NUMBER,
  JV_KIND_STRING,
  JV_KIND_OBJECT,
  JV_KIND_ARRAY
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

jv jv_copy(jv);
void jv_free(jv);

int jv_equal(jv, jv);

jv jv_null();
jv jv_true();
jv jv_false();

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


jv jv_string(char*);
jv jv_string_sized(char*, int);
int jv_string_length(jv);
uint32_t jv_string_hash(jv);
const char* jv_string_value(jv);

jv jv_object();
jv jv_object_get(jv object, jv key);
jv jv_object_set(jv object, jv key, jv value);
jv jv_object_delete(jv object, jv key);

int jv_object_iter(jv);
int jv_object_iter_next(jv, int);
int jv_object_iter_valid(jv, int);
jv jv_object_iter_key(jv, int);
jv jv_object_iter_value(jv, int);


void jv_dump(jv);



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
