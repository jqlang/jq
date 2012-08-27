#ifndef JV_H
#define JV_H

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
