#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "builtin.h"
#include "compile.h"
#include "jq_parser.h"
#include "bytecode.h"
#include "locfile.h"
#include "jv_unicode.h"

#ifndef WIFEXITED
#define WIFEXITED(s) 1
#endif
#ifndef WIFSIGNALED
#define WIFSIGNALED(s) 0
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(s) (s)
#endif
#ifndef WCOREDUMP
#define WCOREDUMP(s) (0)
#endif
#ifndef WTERMSIG
#define WTERMSIG(s) (0)
#endif


static jv type_error(jv bad, const char* msg) {
  jv err = jv_invalid_with_msg(jv_string_fmt("%s %s",
                                             jv_kind_name(jv_get_kind(bad)),
                                             msg));
  jv_free(bad);
  return err;
}

static jv type_error2(jv bad1, jv bad2, const char* msg) {
  jv err = jv_invalid_with_msg(jv_string_fmt("%s and %s %s",
                                             jv_kind_name(jv_get_kind(bad1)),
                                             jv_kind_name(jv_get_kind(bad2)),
                                             msg));
  jv_free(bad1);
  jv_free(bad2);
  return err;
}

static jv f_plus(jq_state *jq, jv input, jv a, jv b) {
  jv_free(input);
  if (jv_get_kind(a) == JV_KIND_NULL) {
    jv_free(a);
    return b;
  } else if (jv_get_kind(b) == JV_KIND_NULL) {
    jv_free(b);
    return a;
  } else if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    return jv_number(jv_number_value(a) + 
                     jv_number_value(b));
  } else if (jv_get_kind(a) == JV_KIND_STRING && jv_get_kind(b) == JV_KIND_STRING) {
    return jv_string_concat(a, b);
  } else if (jv_get_kind(a) == JV_KIND_ARRAY && jv_get_kind(b) == JV_KIND_ARRAY) {
    return jv_array_concat(a, b);
  } else if (jv_get_kind(a) == JV_KIND_OBJECT && jv_get_kind(b) == JV_KIND_OBJECT) {
    return jv_object_merge(a, b);
  } else {
    return type_error2(a, b, "cannot be added");
  }
}

static jv f_floor(jq_state *jq, jv input) {
  if (jv_get_kind(input) != JV_KIND_NUMBER) {
    return type_error(input, "cannot be floored");
  }
  jv ret = jv_number(floor(jv_number_value(input)));
  jv_free(input);
  return ret;
}

static jv f_sqrt(jq_state *jq, jv input) {
  if (jv_get_kind(input) != JV_KIND_NUMBER) {
    return type_error(input, "has no square root");
  }
  jv ret = jv_number(sqrt(jv_number_value(input)));
  jv_free(input);
  return ret;
}

static jv f_negate(jq_state *jq, jv input) {
  if (jv_get_kind(input) != JV_KIND_NUMBER) {
    return type_error(input, "cannot be negated");
  }
  jv ret = jv_number(-jv_number_value(input));
  jv_free(input);
  return ret;
}

static jv f_startswith(jq_state *jq, jv a, jv b) {
  int alen = jv_string_length_bytes(jv_copy(a));
  int blen = jv_string_length_bytes(jv_copy(b));
  jv ret;

  if (blen <= alen && memcmp(jv_string_value(a), jv_string_value(b), blen) == 0)
    ret = jv_true();
  else
    ret = jv_false();
  jv_free(a);
  jv_free(b);
  return ret;
}

static jv f_endswith(jq_state *jq, jv a, jv b) {
  const char *astr = jv_string_value(a);
  const char *bstr = jv_string_value(b);
  size_t alen = jv_string_length_bytes(jv_copy(a));
  size_t blen = jv_string_length_bytes(jv_copy(b));
  jv ret;;

  if (alen < blen ||
     memcmp(astr + (alen - blen), bstr, blen) != 0)
    ret = jv_false();
  else
    ret = jv_true();
  jv_free(a);
  jv_free(b);
  return ret;
}

static jv f_ltrimstr(jq_state *jq, jv input, jv left) {
  if (jv_get_kind(f_startswith(jq, jv_copy(input), jv_copy(left))) != JV_KIND_TRUE) {
    jv_free(left);
    return input;
  }
  /*
   * FIXME It'd be better to share the suffix with the original input --
   * that we could do, we just can't share prefixes.
   */
  int prefixlen = jv_string_length_bytes(left);
  jv res = jv_string_sized(jv_string_value(input) + prefixlen,
                           jv_string_length_bytes(jv_copy(input)) - prefixlen);
  jv_free(input);
  return res;
}

static jv f_rtrimstr(jq_state *jq, jv input, jv right) {
  if (jv_get_kind(f_endswith(jq, jv_copy(input), jv_copy(right))) == JV_KIND_TRUE) {
    jv res = jv_string_sized(jv_string_value(input),
                             jv_string_length_bytes(jv_copy(input)) - jv_string_length_bytes(right));
    jv_free(input);
    return res;
  }
  jv_free(right);
  return input;
}

static jv f_minus(jq_state *jq, jv input, jv a, jv b) {
  jv_free(input);
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    return jv_number(jv_number_value(a) - jv_number_value(b));
  } else if (jv_get_kind(a) == JV_KIND_ARRAY && jv_get_kind(b) == JV_KIND_ARRAY) {
    jv out = jv_array();
    jv_array_foreach(a, i, x) {
      int include = 1;
      jv_array_foreach(b, j, y) {
        if (jv_equal(jv_copy(x), y)) {
          include = 0;
          break;
        }
      }
      if (include)
        out = jv_array_append(out, jv_copy(x));
      jv_free(x);
    }
    jv_free(a);
    jv_free(b);
    return out;
  } else {
    return type_error2(a, b, "cannot be subtracted");
  }
}

static jv f_multiply(jq_state *jq, jv input, jv a, jv b) {
  jv_free(input);
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    return jv_number(jv_number_value(a) * jv_number_value(b));
  } else if (jv_get_kind(a) == JV_KIND_STRING && jv_get_kind(b) == JV_KIND_NUMBER) {
    int n;
    size_t alen = jv_string_length_bytes(jv_copy(a));
    jv res = a;

    for (n = jv_number_value(b) - 1; n > 0; n--)
      res = jv_string_append_buf(res, jv_string_value(a), alen);

    if (n < 0) {
      jv_free(a);
      return jv_null();
    }
    return res;
  } else {
    return type_error2(a, b, "cannot be multiplied");
  }  
}

static jv f_divide(jq_state *jq, jv input, jv a, jv b) {
  jv_free(input);
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    return jv_number(jv_number_value(a) / jv_number_value(b));
  } else if (jv_get_kind(a) == JV_KIND_STRING && jv_get_kind(b) == JV_KIND_STRING) {
    return jv_string_split(a, b);
  } else {
    return type_error2(a, b, "cannot be divided");
  }  
}

static jv f_mod(jq_state *jq, jv input, jv a, jv b) {
  jv_free(input);
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    return jv_number((intmax_t)jv_number_value(a) % (intmax_t)jv_number_value(b));
  } else {
    return type_error2(a, b, "cannot be divided");
  }  
}

static jv f_equal(jq_state *jq, jv input, jv a, jv b) {
  jv_free(input);
  return jv_bool(jv_equal(a, b));
}

static jv f_notequal(jq_state *jq, jv input, jv a, jv b) {
  jv_free(input);
  return jv_bool(!jv_equal(a, b));
}

enum cmp_op {
  CMP_OP_LESS,
  CMP_OP_GREATER,
  CMP_OP_LESSEQ,
  CMP_OP_GREATEREQ
};

static jv order_cmp(jv input, jv a, jv b, enum cmp_op op) {
  jv_free(input);
  int r = jv_cmp(a, b);
  return jv_bool((op == CMP_OP_LESS && r < 0) ||
                 (op == CMP_OP_LESSEQ && r <= 0) ||
                 (op == CMP_OP_GREATEREQ && r >= 0) ||
                 (op == CMP_OP_GREATER && r > 0));
}

static jv f_less(jq_state *jq, jv input, jv a, jv b) {
  return order_cmp(input, a, b, CMP_OP_LESS);
}

static jv f_greater(jq_state *jq, jv input, jv a, jv b) {
  return order_cmp(input, a, b, CMP_OP_GREATER);
}

static jv f_lesseq(jq_state *jq, jv input, jv a, jv b) {
  return order_cmp(input, a, b, CMP_OP_LESSEQ);
}

static jv f_greatereq(jq_state *jq, jv input, jv a, jv b) {
  return order_cmp(input, a, b, CMP_OP_GREATEREQ);
}

static jv f_contains(jq_state *jq, jv a, jv b) {
  if (jv_get_kind(a) == jv_get_kind(b)) {
    return jv_bool(jv_contains(a, b));
  } else {
    return type_error2(a, b, "cannot have their containment checked");
  }
}

static jv f_dump(jq_state *jq, jv input) {
  return jv_dump_string(input, 0);
}

static jv f_json_parse(jq_state *jq, jv input) {
  if (jv_get_kind(input) != JV_KIND_STRING)
    return type_error(input, "only strings can be parsed");
  jv res = jv_parse_sized(jv_string_value(input),
                          jv_string_length_bytes(jv_copy(input)));
  jv_free(input);
  return res;
}

static jv f_tonumber(jq_state *jq, jv input) {
  if (jv_get_kind(input) == JV_KIND_NUMBER) {
    return input;
  }
  if (jv_get_kind(input) == JV_KIND_STRING) {
    jv parsed = jv_parse(jv_string_value(input));
    if (!jv_is_valid(parsed) || jv_get_kind(parsed) == JV_KIND_NUMBER) {
      jv_free(input);
      return parsed;
    }
  }
  return type_error(input, "cannot be parsed as a number");
}

static jv f_length(jq_state *jq, jv input) {
  if (jv_get_kind(input) == JV_KIND_ARRAY) {
    return jv_number(jv_array_length(input));
  } else if (jv_get_kind(input) == JV_KIND_OBJECT) {
    return jv_number(jv_object_length(input));
  } else if (jv_get_kind(input) == JV_KIND_STRING) {
    return jv_number(jv_string_length_codepoints(input));
  } else if (jv_get_kind(input) == JV_KIND_NUMBER) {
    return jv_number(fabs(jv_number_value(input)));
  } else if (jv_get_kind(input) == JV_KIND_NULL) {
    jv_free(input);
    return jv_number(0);
  } else {
    return type_error(input, "has no length");
  }
}

static jv f_tostring(jq_state *jq, jv input) {
  if (jv_get_kind(input) == JV_KIND_STRING) {
    return input;
  } else {
    return jv_dump_string(input, 0);
  }
}

#define CHARS_ALPHANUM "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"

static jv escape_string(jv input, const char* escapings) {

  assert(jv_get_kind(input) == JV_KIND_STRING);
  const char* lookup[128] = {0};
  const char* p = escapings;
  while (*p) {
    lookup[(int)*p] = p+1;
    p++;
    p += strlen(p);
    p++;
  }

  jv ret = jv_string("");
  const char* i = jv_string_value(input);
  const char* end = i + jv_string_length_bytes(jv_copy(input));
  const char* cstart;
  int c = 0;
  while ((i = jvp_utf8_next((cstart = i), end, &c))) {
    assert(c != -1);
    if (c < 128 && lookup[c]) {
      ret = jv_string_append_str(ret, lookup[c]);
    } else {
      ret = jv_string_append_buf(ret, cstart, i - cstart);
    }
  }
  jv_free(input);
  return ret;

}

static jv f_format(jq_state *jq, jv input, jv fmt) {
  if (jv_get_kind(fmt) != JV_KIND_STRING) {
    jv_free(input);
    return type_error(fmt, "is not a valid format");
  }
  const char* fmt_s = jv_string_value(fmt);
  if (!strcmp(fmt_s, "json")) {
    jv_free(fmt);
    return jv_dump_string(input, 0);
  } else if (!strcmp(fmt_s, "text")) {
    jv_free(fmt);
    return f_tostring(jq, input);
  } else if (!strcmp(fmt_s, "csv")) {
    jv_free(fmt);
    if (jv_get_kind(input) != JV_KIND_ARRAY)
      return type_error(input, "cannot be csv-formatted, only array");
    jv line = jv_string("");
    jv_array_foreach(input, i, x) {
      if (i) line = jv_string_append_str(line, ",");
      switch (jv_get_kind(x)) {
      case JV_KIND_NULL:
        /* null rendered as empty string */
        jv_free(x);
        break;
      case JV_KIND_TRUE:
      case JV_KIND_FALSE:
        line = jv_string_concat(line, jv_dump_string(x, 0));
        break;
      case JV_KIND_NUMBER:
        if (jv_number_value(x) != jv_number_value(x)) {
          /* NaN, render as empty string */
          jv_free(x);
        } else {
          line = jv_string_concat(line, jv_dump_string(x, 0));
        }
        break;
      case JV_KIND_STRING: {
        line = jv_string_append_str(line, "\"");
        line = jv_string_concat(line, escape_string(x, "\"\"\"\0"));
        line = jv_string_append_str(line, "\"");
        break;
      }
      default:
        jv_free(input);
        jv_free(line);
        return type_error(x, "is not valid in a csv row");
      }
    }
    jv_free(input);
    return line;
  } else if (!strcmp(fmt_s, "html")) {
    jv_free(fmt);
    return escape_string(f_tostring(jq, input), "&&amp;\0<&lt;\0>&gt;\0'&apos;\0\"&quot;\0");
  } else if (!strcmp(fmt_s, "uri")) {
    jv_free(fmt);
    input = f_tostring(jq, input);

    int unreserved[128] = {0};
    const char* p = CHARS_ALPHANUM "-_.!~*'()";
    while (*p) unreserved[(int)*p++] = 1;

    jv line = jv_string("");
    const char* s = jv_string_value(input);
    for (int i=0; i<jv_string_length_bytes(jv_copy(input)); i++) {
      unsigned ch = (unsigned)(unsigned char)*s;
      if (ch < 128 && unreserved[ch]) {
        line = jv_string_append_buf(line, s, 1);
      } else {
        line = jv_string_concat(line, jv_string_fmt("%%%02x", ch));
      }
      s++;
    }
    jv_free(input);
    return line;
  } else if (!strcmp(fmt_s, "sh")) {
    jv_free(fmt);
    if (jv_get_kind(input) != JV_KIND_ARRAY)
      input = jv_array_set(jv_array(), 0, input);
    jv line = jv_string("");
    jv_array_foreach(input, i, x) {
      if (i) line = jv_string_append_str(line, " ");
      switch (jv_get_kind(x)) {
      case JV_KIND_NULL:
      case JV_KIND_TRUE:
      case JV_KIND_FALSE:
      case JV_KIND_NUMBER:
        line = jv_string_concat(line, jv_dump_string(x, 0));
        break;

      case JV_KIND_STRING: {
        line = jv_string_append_str(line, "'");
        line = jv_string_concat(line, escape_string(x, "''\\''\0"));
        line = jv_string_append_str(line, "'");
        break;
      }

      default:
        jv_free(input);
        jv_free(line);
        return type_error(x, "can not be escaped for shell");
      }
    }
    jv_free(input);
    return line;
  } else if (!strcmp(fmt_s, "base64")) {
    jv_free(fmt);
    input = f_tostring(jq, input);
    jv line = jv_string("");
    const char b64[64 + 1] = CHARS_ALPHANUM "+/";
    const char* data = jv_string_value(input);
    int len = jv_string_length_bytes(jv_copy(input));
    for (int i=0; i<len; i+=3) {
      uint32_t code = 0;
      int n = len - i >= 3 ? 3 : len-i;
      for (int j=0; j<3; j++) {
        code <<= 8;
        code |= j < n ? (unsigned)data[i+j] : 0;
      }
      char buf[4];
      for (int j=0; j<4; j++) {
        buf[j] = b64[(code >> (18 - j*6)) & 0x3f];
      }
      if (n < 3) buf[3] = '=';
      if (n < 2) buf[2] = '=';
      line = jv_string_append_buf(line, buf, sizeof(buf));
    }
    jv_free(input);
    return line;
  } else {
    jv_free(input);
    return jv_invalid_with_msg(jv_string_concat(fmt, jv_string(" is not a valid format")));
  }
}

static jv f_keys(jq_state *jq, jv input) {
  if (jv_get_kind(input) == JV_KIND_OBJECT || jv_get_kind(input) == JV_KIND_ARRAY) {
    return jv_keys(input);
  } else {
    return type_error(input, "has no keys");
  }
}

static jv f_sort(jq_state *jq, jv input){
  if (jv_get_kind(input) == JV_KIND_ARRAY) {
    return jv_sort(input, jv_copy(input));
  } else {
    return type_error(input, "cannot be sorted, as it is not an array");
  }
}

static jv f_sort_by_impl(jq_state *jq, jv input, jv keys) {
  if (jv_get_kind(input) == JV_KIND_ARRAY && 
      jv_get_kind(keys) == JV_KIND_ARRAY &&
      jv_array_length(jv_copy(input)) == jv_array_length(jv_copy(keys))) {
    return jv_sort(input, keys);
  } else {
    return type_error2(input, keys, "cannot be sorted, as they are not both arrays");
  }
}

static jv f_group_by_impl(jq_state *jq, jv input, jv keys) {
  if (jv_get_kind(input) == JV_KIND_ARRAY && 
      jv_get_kind(keys) == JV_KIND_ARRAY &&
      jv_array_length(jv_copy(input)) == jv_array_length(jv_copy(keys))) {
    return jv_group(input, keys);
  } else {
    return type_error2(input, keys, "cannot be sorted, as they are not both arrays");
  }
}

static jv minmax_by(jv values, jv keys, int is_min) {
  if (jv_get_kind(values) != JV_KIND_ARRAY)
    return type_error2(values, keys, "cannot be iterated over");
  if (jv_get_kind(keys) != JV_KIND_ARRAY)
    return type_error2(values, keys, "cannot be iterated over");
  if (jv_array_length(jv_copy(values)) != jv_array_length(jv_copy(keys)))
    return type_error2(values, keys, "have wrong length");

  if (jv_array_length(jv_copy(values)) == 0) {
    jv_free(values);
    jv_free(keys);
    return jv_null();
  }
  jv ret = jv_array_get(jv_copy(values), 0);
  jv retkey = jv_array_get(jv_copy(keys), 0);
  for (int i=1; i<jv_array_length(jv_copy(values)); i++) {
    jv item = jv_array_get(jv_copy(keys), i);
    int cmp = jv_cmp(jv_copy(item), jv_copy(retkey));
    if ((cmp < 0) == (is_min == 1)) {
      jv_free(retkey);
      retkey = item;
      jv_free(ret);
      ret = jv_array_get(jv_copy(values), i);
    } else {
      jv_free(item);
    }
  }
  jv_free(values);
  jv_free(keys);
  jv_free(retkey);
  return ret;
}

static jv f_min(jq_state *jq, jv x) {
  return minmax_by(x, jv_copy(x), 1);
}

static jv f_max(jq_state *jq, jv x) {
  return minmax_by(x, jv_copy(x), 0);
}

static jv f_min_by_impl(jq_state *jq, jv x, jv y) {
  return minmax_by(x, y, 1);
}

static jv f_max_by_impl(jq_state *jq, jv x, jv y) {
  return minmax_by(x, y, 0);
}


static jv f_type(jq_state *jq, jv input) {
  jv out = jv_string(jv_kind_name(jv_get_kind(input)));
  jv_free(input);
  return out;
}

static jv f_error(jq_state *jq, jv input, jv msg) {
  jv_free(input);
  msg = f_tostring(jq, msg);
  return jv_invalid_with_msg(msg);
}

static int flag_is_set(jv o, const char *s) {
  if (jv_get_kind(o) != JV_KIND_OBJECT)
    return 0;
  jv k = jv_string(s);
  jv v = jv_object_get(jv_copy(o), k);
  jv_kind kind = jv_get_kind(v);
  jv_free(v);
  switch (kind) {
  case JV_KIND_NULL:
  case JV_KIND_FALSE:
  case JV_KIND_INVALID: // can't happen
    return 0;
  default:
    return 1;
  }
}

static jv get_option(jv o, const char *s, jv def) {
  if (jv_get_kind(o) != JV_KIND_OBJECT)
    return jv_null();
  jv k = jv_string(s);
  jv v = jv_object_get(jv_copy(o), k);
  if (jv_get_kind(v) == JV_KIND_NULL || jv_get_kind(v) == JV_KIND_INVALID) {
    jv_free(v);
    return def;
  }
  jv_free(def);
  return v;
}

static jv f_buffer(jq_state *jq, jv input, jv def) {
  jv_free(input);
  int hdl = jq_handle_create_buffer(jq, -1);
  jv *v;
  if (jv_get_kind(def) != JV_KIND_NULL && jq_handle_get(jq, "buffer", hdl, (void **)&v, NULL)) {
    jv_free(*v);
    *v = def;
  } else {
    jv_free(def);
  }
  return jv_number(hdl);
}

static jv f_fopen(jq_state *jq, jv input, jv options) {
  FILE *f = NULL;
  int hdl = -1;
  jv err = jv_null();
  jq_runtime_flags flags = jq_flags(jq);

  if (jv_get_kind(input) == JV_KIND_STRING) {
    /* Open a named file; first validate inputs */
    if (!(flags & JQ_OPEN_FILES)) {
      err = jv_invalid_with_msg(jv_string("file opens not allowed"));
      goto out;
    }
    jv fopen_mode = get_option(options, "mode", jv_string("r"));
    if (jv_get_kind(fopen_mode) != JV_KIND_STRING) {
      err = type_error(fopen_mode, "not a file open mode");
      goto out;
    }
    if (!(flags & JQ_OPEN_WRITE) &&
        strspn(jv_string_value(fopen_mode), "cemr") != strlen(jv_string_value(fopen_mode))) {
      err = jv_invalid_with_msg(jv_string("file opens for write not allowed"));
      goto out;
    }
    /*
     * FIXME: What should we do if f == NULL?  Returning a jv_invalid with
     * the error would be nice, but without a way to catch errors in jq
     * programs that's not too useful.  We could return a string instead
     * of a number, but later, when we add a way to catch errors we'd be
     * breaking this.  For now we return null.
     */
    f = fopen(jv_string_value(input), jv_string_value(fopen_mode));
    jv_free(fopen_mode);
    if (f == NULL)
      goto out;
    hdl = jq_handle_create_stdio(jq, -1, f, 1, 0, flag_is_set(options, "raw"),
                                 flag_is_set(options, "slurp"));
  } else if (jv_get_kind(input) == JV_KIND_NULL) {
    /* Open a notional /dev/null */
    hdl = jq_handle_create_null(jq, -1);
  } else if (jv_get_kind(input) != JV_KIND_NULL) {
    err = type_error(input, "not an filename string");
    goto out;
  }

out:
  jv_free(input);
  if (jv_get_kind(err) == JV_KIND_INVALID)
    return err;
  return jv_number(hdl);
}

/* FIXME: We've no way to return the result from pclose() */
static jv f_popen(jq_state *jq, jv input, jv options) {
  int hdl = -1;
  jv err = jv_null();
  jq_runtime_flags flags = jq_flags(jq);
  jv popen_type = get_option(options, "mode", jv_string("r"));
  if (!(flags & JQ_EXEC)) {
    err = jv_invalid_with_msg(jv_string("executing external programs not allowed"));
    goto out;
  }
  if (jv_get_kind(input) != JV_KIND_STRING) {
    err = type_error(input, "not a command");
    goto out;
  }
  if (jv_get_kind(popen_type) != JV_KIND_STRING) {
    err = type_error(popen_type, "not a popen type string");
    goto out;
  }
  FILE *f = popen(jv_string_value(input), jv_string_value(popen_type));

  /*
   * FIXME: What should we do if f == NULL?  Returning a jv_invalid with
   * the error would be nice, but without a way to catch errors in jq
   * programs that's not too useful.  We could return a string instead
   * of a number, but later, when we add a way to catch errors we'd be
   * breaking this.  For now we return null.
   */
  if (f == NULL)
    goto out;
  
  hdl = jq_handle_create_stdio(jq, -1, f, 1, 1, flag_is_set(options, "raw"),
                                 flag_is_set(options, "slurp"));

out:
  jv_free(popen_type);
  jv_free(input);
  if (jv_get_kind(err) == JV_KIND_INVALID)
    return err;
  return jv_number(hdl);
}

static jv f_write(jq_state *jq, jv input, jv handle, jv flags) {
  struct jq_stdio_handle *h;
  int hdl;
  int fl = 0;
  int raw = 0;
  int newline = 1;
  int do_fflush = 0;
  jv *v;

  if (jv_get_kind(handle) != JV_KIND_NUMBER) {
    jv_free(flags);
    return type_error(handle, "not a handle");
  }
  hdl = jv_number_value(handle);
  if (jv_number_value(handle) != (double)hdl) {
    jv_free(flags);
    return type_error(handle, "not a handle (must be integer)");
  }
  if (hdl < 0) {
    jv_free(flags);
    return type_error(handle, "not a valid handle (must be non-negative)");
  }
  jv_free(handle);

  if (jq_handle_get(jq, "null", hdl, NULL, NULL))
    return input; /* "/dev/null" */

  if (jq_handle_get(jq, "buffer", hdl, (void **)&v, NULL)) {
    jv_free(*v);
    *v = jv_copy(input);
    return input;
  }

  if (!jq_handle_get(jq, "FILE", hdl, (void **)&h, NULL)) {
    jv_free(flags);
    jv err = jv_invalid_with_msg(jv_string_fmt("%d is not a valid file handle", hdl));
    return err;
  }

  if (jv_get_kind(flags) == JV_KIND_OBJECT) {
    if (flag_is_set(flags, "ascii"))
        fl |= JV_PRINT_ASCII;
    if (flag_is_set(flags, "raw"))
      raw = 1;
    if (flag_is_set(flags, "no-newline"))
      newline = 0;
    if (flag_is_set(flags, "pretty"))
      fl |= JV_PRINT_PRETTY;
    if (flag_is_set(flags, "color") || flag_is_set(flags, "colour"))
      fl |= JV_PRINT_COLOUR;
    if (flag_is_set(flags, "sorted"))
      fl |= JV_PRINT_SORTED;
    if (flag_is_set(flags, "unbuffered"))
      do_fflush = 1;
  }
  jv_free(flags);

  if (raw && jv_get_kind(input) == JV_KIND_STRING)
    fwrite(jv_string_value(input), 1, jv_string_length_bytes(jv_copy(input)), h->f);
  else
    jv_dumpf(jv_copy(input), h->f, fl);
  if (newline)
    fwrite("\n", 1, sizeof("\n") - 1, h->f);
  if (do_fflush)
    fflush(h->f);
  return input;
}

static jv f_read(jq_state *jq, jv input) {
  struct jq_stdio_handle *h;
  int hdl;
  jv *v;

  if (jv_get_kind(input) != JV_KIND_NUMBER)
    return type_error(input, "not a handle");
  hdl = jv_number_value(input);
  if (jv_number_value(input) != (double)hdl)
    return type_error(input, "not a handle (must be integer)");
  if (hdl < 0)
    return type_error(input, "not a valid handle (must be non-negative)");
  jv_free(input);

  jv a = jv_array_sized(2);

  if (jq_handle_get(jq, "null", hdl, NULL, NULL))
    /* "/dev/null" */
    return a;

  if (jq_handle_get(jq, "buffer", hdl, (void **)&v, NULL)) {
    a = jv_array_append(a, jv_copy(*v));
    return a;
  }

  if (!jq_handle_get(jq, "FILE", hdl, (void **)&h, NULL)) {
    jv err = jv_invalid_with_msg(jv_string_fmt("%d is not a valid file handle", hdl));
    jv_free(a);
    return err;
  }

  if (h->f == NULL)
    return a; // EOF received earlier

  jv value = jv_invalid();
  int len;

  while (!feof(h->f)) {
    if (!fgets(h->buf, sizeof(h->buf), h->f)) {
      h->buf[0] = 0;
      break;
    }
    len = strlen(h->buf);
    if (len < 1)
      continue;
    if (h->p != NULL) {
      jv_parser_set_buf(h->p, h->buf, strlen(h->buf), !feof(h->f));
      value = jv_parser_next(h->p);
      if (jv_is_valid(value)) {
        if (jv_is_valid(h->s))
          /* Slurp parsed values */
          h->s = jv_array_append(h->s, value);
        else
          break;
      }
    } else if (jv_is_valid(h->s)) {
      /* Slurp raw text */
      h->s = jv_string_concat(h->s, jv_string(h->buf));
    } else {
      /* Read raw lines up to sizeof(h->buf) */
      if (h->buf[len-1] == '\n')
        h->buf[len-1] = 0;
      value = jv_string(h->buf);
      break;
    }
  }

  if (h->is_pipe && feof(h->f)) {
    int status = pclose(h->f);
    jv stat = jv_object();

    stat = jv_object_set(stat, jv_string("status"), jv_number(status));
    if (WIFEXITED(status))
      stat = jv_object_set(stat, jv_string("exitstatus"), jv_number(WEXITSTATUS(status)));
    if (WIFSIGNALED(status))
      stat = jv_object_set(stat, jv_string("termsig"), jv_number(WTERMSIG(status)));
    if (WCOREDUMP(status))
      stat = jv_object_set(stat, jv_string("coredump"), jv_true());
    h->f = NULL;

    a = jv_array_append(a, stat);
  }
  if (jv_is_valid(h->s)) {
    value = h->s;
    h->s = jv_null();
  }
  if (jv_is_valid(value))
    return jv_array_append(a, value);
  return a;
}

static jv f_string_split(jq_state *jq, jv input, jv sep) {
  return jv_string_split(input, sep);
}

static jv f_string_explode(jq_state *jq, jv input) {
  return jv_string_explode(input);
}

static jv f_string_implode(jq_state *jq, jv input) {
  return jv_string_implode(input);
}

static jv f_setpath(jq_state *jq, jv input, jv path, jv value) {
  return jv_setpath(input, path, value);
}

static jv f_getpath(jq_state *jq, jv input, jv path) {
  return jv_getpath(input, path);
}

static jv f_delpaths(jq_state *jq, jv input, jv paths) {
  return jv_delpaths(input, paths);
}

static jv f_has(jq_state *jq, jv input, jv k) {
  return jv_has(input, k);
}

static const struct cfunction function_list[] = {
  {(cfunction_ptr)f_floor, "_floor", 1},
  {(cfunction_ptr)f_sqrt, "_sqrt", 1},
  {(cfunction_ptr)f_plus, "_plus", 3},
  {(cfunction_ptr)f_negate, "_negate", 1},
  {(cfunction_ptr)f_minus, "_minus", 3},
  {(cfunction_ptr)f_multiply, "_multiply", 3},
  {(cfunction_ptr)f_divide, "_divide", 3},
  {(cfunction_ptr)f_mod, "_mod", 3},
  {(cfunction_ptr)f_dump, "tojson", 1},
  {(cfunction_ptr)f_json_parse, "fromjson", 1},
  {(cfunction_ptr)f_tonumber, "tonumber", 1},
  {(cfunction_ptr)f_tostring, "tostring", 1},
  {(cfunction_ptr)f_keys, "keys", 1},
  {(cfunction_ptr)f_startswith, "startswith", 2},
  {(cfunction_ptr)f_endswith, "endswith", 2},
  {(cfunction_ptr)f_ltrimstr, "ltrimstr", 2},
  {(cfunction_ptr)f_rtrimstr, "rtrimstr", 2},
  {(cfunction_ptr)f_string_split, "split", 2},
  {(cfunction_ptr)f_string_explode, "explode", 1},
  {(cfunction_ptr)f_string_implode, "implode", 1},
  {(cfunction_ptr)f_setpath, "setpath", 3}, // FIXME typechecking
  {(cfunction_ptr)f_getpath, "getpath", 2},
  {(cfunction_ptr)f_delpaths, "delpaths", 2},
  {(cfunction_ptr)f_has, "has", 2},
  {(cfunction_ptr)f_equal, "_equal", 3},
  {(cfunction_ptr)f_notequal, "_notequal", 3},
  {(cfunction_ptr)f_less, "_less", 3},
  {(cfunction_ptr)f_greater, "_greater", 3},
  {(cfunction_ptr)f_lesseq, "_lesseq", 3},
  {(cfunction_ptr)f_greatereq, "_greatereq", 3},
  {(cfunction_ptr)f_contains, "contains", 2},
  {(cfunction_ptr)f_length, "length", 1},
  {(cfunction_ptr)f_type, "type", 1},
  {(cfunction_ptr)f_sort, "sort", 1},
  {(cfunction_ptr)f_sort_by_impl, "_sort_by_impl", 2},
  {(cfunction_ptr)f_group_by_impl, "_group_by_impl", 2},
  {(cfunction_ptr)f_min, "min", 1},
  {(cfunction_ptr)f_max, "max", 1},
  {(cfunction_ptr)f_min_by_impl, "_min_by_impl", 2},
  {(cfunction_ptr)f_max_by_impl, "_max_by_impl", 2},
  {(cfunction_ptr)f_error, "error", 2},
  {(cfunction_ptr)f_format, "format", 2},
  {(cfunction_ptr)f_read, "_read", 1},
  {(cfunction_ptr)f_write, "write", 3},
  {(cfunction_ptr)f_buffer, "buffer", 2},
  {(cfunction_ptr)f_fopen, "fopen", 2},
  {(cfunction_ptr)f_popen, "popen", 2},
  {(cfunction_ptr)f_feof, "eof", 1},
};

struct bytecoded_builtin { const char* name; block code; };
static block bind_bytecoded_builtins(block b) {
  block builtins = gen_noop();
  {
    struct bytecoded_builtin builtin_defs[] = {
      {"empty", gen_op_simple(BACKTRACK)},
      {"false", gen_const(jv_false())},
      {"true", gen_const(jv_true())},
      {"null", gen_const(jv_null())},
      {"not", gen_condbranch(gen_const(jv_false()),
                             gen_const(jv_true()))}
    };
    for (unsigned i=0; i<sizeof(builtin_defs)/sizeof(builtin_defs[0]); i++) {
      builtins = BLOCK(builtins, gen_function(builtin_defs[i].name, gen_noop(),
                                              builtin_defs[i].code));
    }
  }
  {
    struct bytecoded_builtin builtin_def_1arg[] = {
      {"path", BLOCK(gen_op_simple(PATH_BEGIN), 
                     gen_call("arg", gen_noop()),
                     gen_op_simple(PATH_END))},
    };
    for (unsigned i=0; i<sizeof(builtin_def_1arg)/sizeof(builtin_def_1arg[0]); i++) {
      builtins = BLOCK(builtins, gen_function(builtin_def_1arg[i].name,
                                              gen_param("arg"),
                                              builtin_def_1arg[i].code));
    }
  }
  {
    block rangevar = gen_op_var_fresh(STOREV, "rangevar");
    block init = BLOCK(gen_op_simple(DUP), gen_call("start", gen_noop()), rangevar);
    block range = BLOCK(init, 
                        gen_call("end", gen_noop()),
                        gen_op_bound(RANGE, rangevar));
    builtins = BLOCK(builtins, gen_function("range",
                                            BLOCK(gen_param("start"), gen_param("end")),
                                            range));
  }
  
  return block_bind_referenced(builtins, b, OP_IS_CALL_PSEUDO);
}

static const char* const jq_builtins[] = {
  "def map(f): [.[] | f];",
  "def select(f): if f then . else empty end;",
  "def read: _read|if length == 0 then empty elif length == 2 then . else .[0] end;",
  "def sort_by(f): _sort_by_impl(map([f]));",
  "def group_by(f): _group_by_impl(map([f]));",
  "def unique: group_by(.) | map(.[0]);",
  "def max_by(f): _max_by_impl(map([f]));",
  "def min_by(f): _min_by_impl(map([f]));",
  "def floor: _floor;",
  "def sqrt: _sqrt;",
  "def add: reduce .[] as $x (null; . + $x);",
  "def del(f): delpaths([path(f)]);",
  "def _assign(paths; value): value as $v | reduce path(paths) as $p (.; setpath($p; $v));",
  "def _modify(paths; update): reduce path(paths) as $p (.; setpath($p; getpath($p) | update));",
  "def recurse(f): ., (f | select(. != null) | recurse(f));",
  "def to_entries: [keys[] as $k | {key: $k, value: .[$k]}];",
  "def from_entries: map({(.key): .value}) | add;",
  "def with_entries(f): to_entries | map(f) | from_entries;",
  "def reverse: [.[length - 1 - range(0;length)]];",
  "def index(i): .[i][0];",
  "def rindex(i): .[i][-1:][0];",
};


static int builtins_bind_one(jq_state *jq, block* bb, const char* code) {
  struct locfile src;
  locfile_init(&src, jq, code, strlen(code));
  block funcs;
  int nerrors = jq_parse_library(&src, &funcs);
  if (nerrors == 0) {
    *bb = block_bind_referenced(funcs, *bb, OP_IS_CALL_PSEUDO);
  }
  locfile_free(&src);
  return nerrors;
}

static int slurp_lib(jq_state *jq, block* bb) {
  int nerrors = 0;
  char* home = getenv("HOME");
  if (home) {    // silently ignore no $HOME
    jv filename = jv_string_append_str(jv_string(home), "/.jq");
    jv data = jv_load_file(jv_string_value(filename), 1);
    if (jv_is_valid(data)) {
      nerrors = builtins_bind_one(jq, bb, jv_string_value(data) );
    }
    jv_free(filename);
    jv_free(data);
  }
  return nerrors;
}

int builtins_bind(jq_state *jq, block* bb) {
  int nerrors = slurp_lib(jq, bb);
  if (nerrors) {
    block_free(*bb);
    return nerrors;
  }
  for (int i=(int)(sizeof(jq_builtins)/sizeof(jq_builtins[0]))-1; i>=0; i--) {
    nerrors = builtins_bind_one(jq, bb, jq_builtins[i]);
    assert(!nerrors);
  }
  *bb = bind_bytecoded_builtins(*bb);
  *bb = gen_cbinding(function_list, sizeof(function_list)/sizeof(function_list[0]), *bb);
  return nerrors;
}
