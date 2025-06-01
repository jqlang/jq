#include "jv.h"
#include <string.h>

// Helper: Given an array of objects, produce {"headings": ..., "rows": ...}
// Preserves key order from the first object.
jv convert_array_of_objects(jv arr) {
  int n = jv_array_length(jv_copy(arr));
  if (n == 0) {
    jv_free(arr);
    return jv_invalid_with_msg(jv_string("Cannot render empty array of objects as table"));
  }
  jv first_obj = jv_array_get(jv_copy(arr), 0);
  jv keys = jv_array();
  if (jv_get_kind(first_obj) == JV_KIND_OBJECT) {
    int iter = jv_object_iter(first_obj);
    while (jv_object_iter_valid(first_obj, iter)) {
      jv key = jv_object_iter_key(first_obj, iter);
      keys = jv_array_append(keys, jv_copy(key));
      jv_free(key);
      iter = jv_object_iter_next(first_obj, iter);
    }
  }
  jv_free(first_obj);
  for (int i = 1; i < n; ++i) {
    jv obj = jv_array_get(jv_copy(arr), i);
    if (jv_get_kind(obj) != JV_KIND_OBJECT) { jv_free(obj); continue; }
    jv iter = jv_keys(jv_copy(obj));
    int key_count = jv_array_length(jv_copy(iter));
    for (int j = 0; j < key_count; ++j) {
      jv k = jv_array_get(jv_copy(iter), j);
      int already = 0;
      int keys_len = jv_array_length(jv_copy(keys));
      for (int ki = 0; ki < keys_len; ++ki) {
        jv k2 = jv_array_get(jv_copy(keys), ki);
        if (jv_equal(jv_copy(k), jv_copy(k2))) already = 1;
        jv_free(k2);
        if (already) break;
      }
      if (!already) keys = jv_array_append(keys, jv_copy(k));
      jv_free(k);
    }
    jv_free(iter);
    jv_free(obj);
  }
  jv rows = jv_array();
  for (int i = 0; i < n; ++i) {
    jv obj = jv_array_get(jv_copy(arr), i);
    if (jv_get_kind(obj) != JV_KIND_OBJECT) { jv_free(obj); continue; }
    jv row = jv_array();
    int key_count = jv_array_length(jv_copy(keys));
    for (int j = 0; j < key_count; ++j) {
      jv k = jv_array_get(jv_copy(keys), j);
      jv v = jv_object_get(jv_copy(obj), jv_copy(k));
      if (!jv_is_valid(v)) v = jv_null();
      row = jv_array_append(row, v);
      jv_free(k);
    }
    rows = jv_array_append(rows, row);
    jv_free(obj);
  }
  jv result = jv_object();
  result = jv_object_set(result, jv_string("headings"), keys);
  result = jv_object_set(result, jv_string("rows"), rows);
  jv_free(arr);
  return result;
}

jv normalize_rows_object(jv input) {
  jv rows = jv_object_get(jv_copy(input), jv_string("rows"));
  jv headings = jv_object_get(jv_copy(input), jv_string("headings"));
  if (jv_get_kind(headings) == JV_KIND_ARRAY) {
    jv_free(rows);
    jv_free(headings);
    return input; // already normalized
  }
  if (jv_get_kind(rows) == JV_KIND_ARRAY && jv_array_length(jv_copy(rows)) > 0) {
    jv first = jv_array_get(jv_copy(rows), 0);
    if (jv_get_kind(first) == JV_KIND_OBJECT) {
      jv expanded = convert_array_of_objects(jv_copy(rows));
      jv color_words = jv_object_get(jv_copy(input), jv_string("color_words"));
      if (jv_get_kind(color_words) == JV_KIND_ARRAY) {
        expanded = jv_object_set(expanded, jv_string("color_words"), color_words);
      } else {
        jv_free(color_words);
        jv color_terms = jv_object_get(jv_copy(input), jv_string("color_terms"));
        if (jv_get_kind(color_terms) == JV_KIND_ARRAY) {
          expanded = jv_object_set(expanded, jv_string("color_terms"), color_terms);
        } else {
          jv_free(color_terms);
        }
      }
      jv_free(first);
      jv_free(rows);
      jv_free(input);
      return expanded;
    }
    jv_free(first);
  }
  jv_free(rows);
  jv_free(headings);
  return input;
}