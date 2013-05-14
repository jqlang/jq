#include "jv_aux.h"
#include <string.h>
#include <stdlib.h>
#include "jv_alloc.h"

static int parse_slice(jv array, jv slice, int* pstart, int* pend) {
  // Array slices
  int len = jv_array_length(jv_copy(array));
  jv start_jv = jv_object_get(jv_copy(slice), jv_string("start"));
  jv end_jv = jv_object_get(slice, jv_string("end"));
  if (jv_get_kind(start_jv) == JV_KIND_NULL) {
    jv_free(start_jv);
    start_jv = jv_number(0);
  }
  if (jv_get_kind(end_jv) == JV_KIND_NULL) {
    jv_free(end_jv);
    end_jv = jv_number(len);
  }
  if (jv_get_kind(start_jv) != JV_KIND_NUMBER ||
      jv_get_kind(end_jv) != JV_KIND_NUMBER) {
    jv_free(start_jv);
    jv_free(end_jv);
    return 0;
  } else {
    int start = (int)jv_number_value(start_jv);
    int end = (int)jv_number_value(end_jv);
    if (start < 0) start = len + start;
    if (end < 0) end = len + end;

    if (start < 0) start = 0;
    if (start > len) start = len;
    if (end > len) end = len;
    if (end < start) end = start;
    assert(0 <= start && start <= end && end <= len);
    *pstart = start;
    *pend = end;
    return 1;
  }
}

jv jv_get(jv t, jv k) {
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
  } else if (jv_get_kind(t) == JV_KIND_ARRAY && jv_get_kind(k) == JV_KIND_OBJECT) {
    int start, end;
    if (parse_slice(t, k, &start, &end)) {
      v = jv_array_slice(t, start, end);
    } else {
      v = jv_invalid_with_msg(jv_string_fmt("Start and end indices of an array slice must be numbers"));
      jv_free(t);
    }
  } else if (jv_get_kind(t) == JV_KIND_NULL && 
             (jv_get_kind(k) == JV_KIND_STRING || 
              jv_get_kind(k) == JV_KIND_NUMBER || 
              jv_get_kind(k) == JV_KIND_OBJECT)) {
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

jv jv_set(jv t, jv k, jv v) {
  if (!jv_is_valid(v)) {
    jv_free(t);
    jv_free(k);
    return v;
  }
  int isnull = jv_get_kind(t) == JV_KIND_NULL;
  if (jv_get_kind(k) == JV_KIND_STRING && 
      (jv_get_kind(t) == JV_KIND_OBJECT || isnull)) {
    if (isnull) t = jv_object();
    t = jv_object_set(t, k, v);
  } else if (jv_get_kind(k) == JV_KIND_NUMBER &&
             (jv_get_kind(t) == JV_KIND_ARRAY || isnull)) {
    if (isnull) t = jv_array();
    t = jv_array_set(t, (int)jv_number_value(k), v);
  } else if (jv_get_kind(k) == JV_KIND_OBJECT &&
             (jv_get_kind(t) == JV_KIND_ARRAY || isnull)) {
    if (isnull) t = jv_array();
    int start, end;
    if (parse_slice(t, k, &start, &end)) {
      if (jv_get_kind(v) == JV_KIND_ARRAY) {
        int array_len = jv_array_length(jv_copy(t));
        assert(0 <= start && start <= end && end <= array_len);
        int slice_len = end - start;
        int insert_len = jv_array_length(jv_copy(v));
        if (slice_len < insert_len) {
          // array is growing
          int shift = insert_len - slice_len;
          for (int i = array_len - 1; i >= end; i--) {
            t = jv_array_set(t, i + shift, jv_array_get(jv_copy(t), i));
          }
        } else if (slice_len > insert_len) {
          // array is shrinking
          int shift = slice_len - insert_len;
          for (int i = end; i < array_len; i++) {
            t = jv_array_set(t, i - shift, jv_array_get(jv_copy(t), i));
          }
          t = jv_array_slice(t, 0, array_len - shift);
        }
        for (int i=0; i < insert_len; i++) {
          t = jv_array_set(t, start + i, jv_array_get(jv_copy(v), i));
        }
        jv_free(v);
      } else {
        jv_free(t);
        jv_free(v);
        t = jv_invalid_with_msg(jv_string_fmt("A slice of an array can only be assigned another array"));
      }
    } else {
      jv_free(t);
      jv_free(k);
      jv_free(v);
      t = jv_invalid_with_msg(jv_string_fmt("Start and end indices of an array slice must be numbers"));
    }
  } else {
    jv err = jv_invalid_with_msg(jv_string_fmt("Cannot update field at %s index of %s",
                                               jv_kind_name(jv_get_kind(k)),
                                               jv_kind_name(jv_get_kind(t))));
    jv_free(t);
    jv_free(k);
    jv_free(v);
    t = err;
  }
  return t;
}

jv jv_has(jv t, jv k) {
  assert(jv_is_valid(t));
  assert(jv_is_valid(k));
  jv ret;
  if (jv_get_kind(t) == JV_KIND_NULL) {
    jv_free(t);
    jv_free(k);
    ret = jv_false();
  } else if (jv_get_kind(t) == JV_KIND_OBJECT &&
             jv_get_kind(k) == JV_KIND_STRING) {
    jv elem = jv_object_get(t, k);
    ret = jv_bool(jv_is_valid(elem));
    jv_free(elem);
  } else if (jv_get_kind(t) == JV_KIND_ARRAY &&
             jv_get_kind(k) == JV_KIND_NUMBER) {
    jv elem = jv_array_get(t, (int)jv_number_value(k));
    ret = jv_bool(jv_is_valid(elem));
    jv_free(elem);
  } else {
    ret = jv_invalid_with_msg(jv_string_fmt("Cannot check whether %s has a %s key",
                                            jv_kind_name(jv_get_kind(t)),
                                            jv_kind_name(jv_get_kind(k))));
    jv_free(t);
    jv_free(k);
  }
  return ret;
}

// assumes keys is a sorted array
jv jv_dels(jv t, jv keys) {
  assert(jv_get_kind(keys) == JV_KIND_ARRAY);
  assert(jv_is_valid(t));
  
  if (jv_get_kind(t) == JV_KIND_NULL || jv_array_length(jv_copy(keys)) == 0) {
    // no change
  } else if (jv_get_kind(t) == JV_KIND_ARRAY) {
    // extract slices, they must be handled differently
    jv orig_keys = keys;
    keys = jv_array();
    jv new_array = jv_array();
    jv starts = jv_array(), ends = jv_array();
    jv_array_foreach(orig_keys, i, key) {
      if (jv_get_kind(key) == JV_KIND_NUMBER) {
        keys = jv_array_append(keys, key);
      } else if (jv_get_kind(key) == JV_KIND_OBJECT) {
        int start, end;
        if (parse_slice(t, key, &start, &end)) {
          starts = jv_array_append(starts, jv_number(start));
          ends = jv_array_append(ends, jv_number(end));
        } else {
          jv_free(new_array);
          jv_free(key);
          new_array = jv_invalid_with_msg(jv_string_fmt("Start and end indices of an array slice must be numbers"));
          goto arr_out;
        }
      } else {
        jv_free(new_array);
        new_array = jv_invalid_with_msg(jv_string_fmt("Cannot delete %s element of array",
                                                      jv_kind_name(jv_get_kind(key))));
        jv_free(key);
        goto arr_out;
      }
    }

    int kidx = 0;
    jv_array_foreach(t, i, elem) {
      int del = 0;
      while (kidx < jv_array_length(jv_copy(keys))) {
        int delidx = (int)jv_number_value(jv_array_get(jv_copy(keys), kidx));
        if (i == delidx) {
          del = 1;
        }
        if (i < delidx) {
          break;
        }
        kidx++;
      }
      for (int sidx=0; !del && sidx<jv_array_length(jv_copy(starts)); sidx++) {
        if ((int)jv_number_value(jv_array_get(jv_copy(starts), sidx)) <= i &&
            i < (int)jv_number_value(jv_array_get(jv_copy(ends), sidx))) {
          del = 1;
        }
      }
      if (!del)
        new_array = jv_array_append(new_array, elem);
      else
        jv_free(elem);
    }
  arr_out:
    jv_free(starts);
    jv_free(ends);
    jv_free(orig_keys);
    jv_free(t);
    t = new_array;
  } else if (jv_get_kind(t) == JV_KIND_OBJECT) {
    jv_array_foreach(keys, i, k) {
      if (jv_get_kind(k) != JV_KIND_STRING) {
        jv_free(t);
        t = jv_invalid_with_msg(jv_string_fmt("Cannot delete %s field of object",
                                              jv_kind_name(jv_get_kind(k))));
        jv_free(k);
        break;
      }
      t = jv_object_delete(t, k);      
    }
  } else {
    jv err = jv_invalid_with_msg(jv_string_fmt("Cannot delete fields from %s",
                                               jv_kind_name(jv_get_kind(t))));
    jv_free(t);
    t = err;
  }
  jv_free(keys);
  return t;
}

jv jv_setpath(jv root, jv path, jv value) {
  if (jv_get_kind(path) != JV_KIND_ARRAY) {
    jv_free(value);
    jv_free(root);
    jv_free(path);
    return jv_invalid_with_msg(jv_string("Path must be specified as an array"));
  }
  if (!jv_is_valid(root)){
    jv_free(value);
    jv_free(path);
    return root;
  }
  if (jv_array_length(jv_copy(path)) == 0) {
    jv_free(path);
    jv_free(root);
    return value;
  }
  jv pathcurr = jv_array_get(jv_copy(path), 0);
  jv pathrest = jv_array_slice(path, 1, jv_array_length(jv_copy(path)));
  return jv_set(root, pathcurr, 
                jv_setpath(jv_get(jv_copy(root), jv_copy(pathcurr)), pathrest, value));
}

jv jv_getpath(jv root, jv path) {
  if (jv_get_kind(path) != JV_KIND_ARRAY) {
    jv_free(root);
    jv_free(path);
    return jv_invalid_with_msg(jv_string("Path must be specified as an array"));
  }
  if (!jv_is_valid(root)) {
    jv_free(path);
    return root;
  }
  if (jv_array_length(jv_copy(path)) == 0) {
    jv_free(path);
    return root;
  }
  jv pathcurr = jv_array_get(jv_copy(path), 0);
  jv pathrest = jv_array_slice(path, 1, jv_array_length(jv_copy(path)));
  return jv_getpath(jv_get(root, pathcurr), pathrest);
}

// assumes paths is a sorted array of arrays
static jv delpaths_sorted(jv object, jv paths, int start) {
  jv delkeys = jv_array();
  for (int i=0; i<jv_array_length(jv_copy(paths));) {
    int j = i;
    assert(jv_array_length(jv_array_get(jv_copy(paths), i)) > start);
    int delkey = jv_array_length(jv_array_get(jv_copy(paths), i)) == start + 1;
    jv key = jv_array_get(jv_array_get(jv_copy(paths), i), start);
    while (j < jv_array_length(jv_copy(paths)) &&
           jv_equal(jv_copy(key), jv_array_get(jv_array_get(jv_copy(paths), j), start)))
      j++;
    // if i <= entry < j, then entry starts with key
    if (delkey) {
      // deleting this entire key, we don't care about any more specific deletions
      delkeys = jv_array_append(delkeys, key);
    } else {
      // deleting certain sub-parts of this key
      jv subobject = jv_get(jv_copy(object), jv_copy(key));
      if (!jv_is_valid(subobject)) {
        jv_free(key);
        jv_free(object);
        object = subobject;
        break;
      } else if (jv_get_kind(subobject) == JV_KIND_NULL) {
        jv_free(key);
        jv_free(subobject);
      } else {
        jv newsubobject = delpaths_sorted(subobject, jv_array_slice(jv_copy(paths), i, j), start+1);
        if (!jv_is_valid(newsubobject)) {
          jv_free(key);
          jv_free(object);
          object = newsubobject;
          break;
        }
        object = jv_set(object, key, newsubobject);
      }
      if (!jv_is_valid(object)) break;
    }
    i = j;
  }
  jv_free(paths);
  if (jv_is_valid(object))
    object = jv_dels(object, delkeys);
  else 
    jv_free(delkeys);
  return object;
}

jv jv_delpaths(jv object, jv paths) {
  paths = jv_sort(paths, jv_copy(paths));
  jv_array_foreach(paths, i, elem) {
    if (jv_get_kind(elem) != JV_KIND_ARRAY) {
      jv_free(object);
      jv_free(paths);
      jv err = jv_invalid_with_msg(jv_string_fmt("Path must be specified as array, not %s",
                                                 jv_kind_name(jv_get_kind(elem))));
      jv_free(elem);
      return err;
    }
    jv_free(elem);
  }
  if (jv_array_length(jv_copy(paths)) == 0) {
    // nothing is being deleted
    jv_free(paths);
    return object;
  }
  if (jv_array_length(jv_array_get(jv_copy(paths), 0)) == 0) {
    // everything is being deleted
    jv_free(paths);
    jv_free(object);
    return jv_null();
  }
  return delpaths_sorted(object, paths, 0);
}


static int string_cmp(const void* pa, const void* pb){
  const jv* a = pa;
  const jv* b = pb;
  int lena = jv_string_length_bytes(jv_copy(*a));
  int lenb = jv_string_length_bytes(jv_copy(*b));
  int minlen = lena < lenb ? lena : lenb;
  int r = memcmp(jv_string_value(*a), jv_string_value(*b), minlen);
  if (r == 0) r = lena - lenb;
  return r;
}

jv jv_keys(jv x) {
  if (jv_get_kind(x) == JV_KIND_OBJECT) {
    int nkeys = jv_object_length(jv_copy(x));
    jv* keys = jv_mem_alloc(sizeof(jv) * nkeys);
    int kidx = 0;
    jv_object_foreach(x, key, value) {
      keys[kidx++] = key;
      jv_free(value);
    }
    qsort(keys, nkeys, sizeof(jv), string_cmp);
    jv answer = jv_array_sized(nkeys);
    for (int i = 0; i<nkeys; i++) {
      answer = jv_array_append(answer, keys[i]);
    }
    jv_mem_free(keys);
    jv_free(x);
    return answer;
  } else if (jv_get_kind(x) == JV_KIND_ARRAY) {
    int n = jv_array_length(x);
    jv answer = jv_array();
    for (int i=0; i<n; i++){
      answer = jv_array_set(answer, i, jv_number(i));
    }
    return answer;
  } else {
    assert(0 && "jv_keys passed something neither object nor array");
    return jv_invalid();
  }
}

int jv_cmp(jv a, jv b) {
  if (jv_get_kind(a) != jv_get_kind(b)) {
    int r = (int)jv_get_kind(a) - (int)jv_get_kind(b);
    jv_free(a);
    jv_free(b);
    return r;
  }
  int r = 0;
  switch (jv_get_kind(a)) {
  default:
    assert(0 && "invalid kind passed to jv_cmp");
  case JV_KIND_NULL:
  case JV_KIND_FALSE:
  case JV_KIND_TRUE:
    // there's only one of each of these values
    r = 0;
    break;

  case JV_KIND_NUMBER: {
    double da = jv_number_value(a), db = jv_number_value(b);
    
    // handle NaN as though it were null
    if (da != da) r = jv_cmp(jv_null(), jv_number(db));
    else if (db != db) r = jv_cmp(jv_number(da), jv_null());
    else if (da < db) r = -1;
    else if (da == db) r = 0;
    else r = 1;
    break;
  }

  case JV_KIND_STRING: {
    r = string_cmp(&a, &b);
    break;
  }

  case JV_KIND_ARRAY: {
    // Lexical ordering of arrays
    int i = 0;
    while (r == 0) {
      int a_done = i >= jv_array_length(jv_copy(a));
      int b_done = i >= jv_array_length(jv_copy(b));
      if (a_done || b_done) {
        r = b_done - a_done; //suddenly, logic
        break;
      }
      jv xa = jv_array_get(jv_copy(a), i);
      jv xb = jv_array_get(jv_copy(b), i);
      r = jv_cmp(xa, xb);
      i++;
    }
    break;
  }

  case JV_KIND_OBJECT: {
    jv keys_a = jv_keys(jv_copy(a));
    jv keys_b = jv_keys(jv_copy(b));
    r = jv_cmp(jv_copy(keys_a), keys_b);
    if (r == 0) {
      jv_array_foreach(keys_a, i, key) {
        jv xa = jv_object_get(jv_copy(a), jv_copy(key));
        jv xb = jv_object_get(jv_copy(b), key);
        r = jv_cmp(xa, xb);
        if (r) break;
      }
    }
    jv_free(keys_a);
    break;
  }
  }

  jv_free(a);
  jv_free(b);
  return r;
}


struct sort_entry {
  jv object;
  jv key;
};

static int sort_cmp(const void* pa, const void* pb) {
  const struct sort_entry* a = pa;
  const struct sort_entry* b = pb;
  int r = jv_cmp(jv_copy(a->key), jv_copy(b->key));
  // comparing by address if r == 0 makes the sort stable
  return r ? r : (int)(a - b);
}

static struct sort_entry* sort_items(jv objects, jv keys) {
  assert(jv_get_kind(objects) == JV_KIND_ARRAY);
  assert(jv_get_kind(keys) == JV_KIND_ARRAY);
  assert(jv_array_length(jv_copy(objects)) == jv_array_length(jv_copy(keys)));
  int n = jv_array_length(jv_copy(objects));
  struct sort_entry* entries = jv_mem_alloc(sizeof(struct sort_entry) * n);
  for (int i=0; i<n; i++) {
    entries[i].object = jv_array_get(jv_copy(objects), i);
    entries[i].key = jv_array_get(jv_copy(keys), i);
  }
  jv_free(objects);
  jv_free(keys);
  qsort(entries, n, sizeof(struct sort_entry), sort_cmp);
  return entries;
}

jv jv_sort(jv objects, jv keys) {
  assert(jv_get_kind(objects) == JV_KIND_ARRAY);
  assert(jv_get_kind(keys) == JV_KIND_ARRAY);
  assert(jv_array_length(jv_copy(objects)) == jv_array_length(jv_copy(keys)));
  int n = jv_array_length(jv_copy(objects));
  struct sort_entry* entries = sort_items(objects, keys);
  jv ret = jv_array();
  for (int i=0; i<n; i++) {
    jv_free(entries[i].key);
    ret = jv_array_set(ret, i, entries[i].object);
  }
  jv_mem_free(entries);
  return ret;
}

jv jv_group(jv objects, jv keys) {
  assert(jv_get_kind(objects) == JV_KIND_ARRAY);
  assert(jv_get_kind(keys) == JV_KIND_ARRAY);
  assert(jv_array_length(jv_copy(objects)) == jv_array_length(jv_copy(keys)));
  int n = jv_array_length(jv_copy(objects));
  struct sort_entry* entries = sort_items(objects, keys);
  jv ret = jv_array();
  jv curr_key = entries[0].key;
  jv group = jv_array_append(jv_array(), entries[0].object);
  for (int i = 1; i < n; i++) {
    if (jv_equal(jv_copy(curr_key), jv_copy(entries[i].key))) {
      jv_free(entries[i].key);
    } else {
      jv_free(curr_key);
      curr_key = entries[i].key;
      ret = jv_array_append(ret, group);
      group = jv_array();
    }
    group = jv_array_append(group, entries[i].object);
  }
  jv_free(curr_key);
  ret = jv_array_append(ret, group);
  jv_mem_free(entries);
  return ret;
}
