#include "jv_aux.h"

jv jv_lookup(jv t, jv k) {
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

jv jv_modify(jv t, jv k, jv v) {
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

jv jv_insert(jv root, jv value, jv* path, int pathlen) {
  if (pathlen == 0) {
    jv_free(root);
    return value;
  }
  return jv_modify(root, jv_copy(*path), 
                   jv_insert(jv_lookup(jv_copy(root), jv_copy(*path)), value, path+1, pathlen-1));
}
