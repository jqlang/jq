#include "jv_aux.h"
#include <string.h>
#include <stdlib.h>
#include "jv_alloc.h"

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


static int string_cmp(const void* pa, const void* pb){
  const jv* a = pa;
  const jv* b = pb;
  int lena = jv_string_length(jv_copy(*a));
  int lenb = jv_string_length(jv_copy(*b));
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
    jv_object_foreach(i, x) {
      keys[kidx++] = jv_object_iter_key(x, i);
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
      for (int i=0; i<jv_array_length(jv_copy(keys_a)); i++) {
        jv key = jv_array_get(jv_copy(keys_a), i);
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
