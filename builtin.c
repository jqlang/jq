#define _BSD_SOURCE
#define _GNU_SOURCE
#define _XOPEN_SOURCE
#include <sys/time.h>
#include <stdlib.h>
#include <stddef.h>
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif !defined alloca
# ifdef __GNUC__
#  define alloca __builtin_alloca
# elif defined _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# elif !defined HAVE_ALLOCA
#  ifdef  __cplusplus
extern "C"
#  endif
void *alloca (size_t);
# endif
#endif
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#ifdef HAVE_ONIGURUMA
#include <oniguruma.h>
#endif
#include <string.h>
#include <time.h>
#include "builtin.h"
#include "compile.h"
#include "jq_parser.h"
#include "bytecode.h"
#include "linker.h"
#include "locfile.h"
#include "jv_unicode.h"


static jv type_error(jv bad, const char* msg) {
  char errbuf[15];
  jv err = jv_invalid_with_msg(jv_string_fmt("%s (%s) %s",
                                             jv_kind_name(jv_get_kind(bad)),
                                             jv_dump_string_trunc(jv_copy(bad), errbuf, sizeof(errbuf)),
                                             msg));
  jv_free(bad);
  return err;
}

static jv type_error2(jv bad1, jv bad2, const char* msg) {
  char errbuf1[15],errbuf2[15];
  jv err = jv_invalid_with_msg(jv_string_fmt("%s (%s) and %s (%s) %s",
                                             jv_kind_name(jv_get_kind(bad1)),
                                             jv_dump_string_trunc(jv_copy(bad1), errbuf1, sizeof(errbuf1)),
                                             jv_kind_name(jv_get_kind(bad2)),
                                             jv_dump_string_trunc(jv_copy(bad2), errbuf2, sizeof(errbuf2)),
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

#define LIBM_DD(name) \
static jv f_ ## name(jq_state *jq, jv input) { \
  if (jv_get_kind(input) != JV_KIND_NUMBER) { \
    return type_error(input, "number required"); \
  } \
  jv ret = jv_number(name(jv_number_value(input))); \
  jv_free(input); \
  return ret; \
}
#define LIBM_DD_NO(name)

#define LIBM_DDD(name) \
static jv f_ ## name(jq_state *jq, jv input, jv a, jv b) { \
  if (jv_get_kind(a) != JV_KIND_NUMBER || jv_get_kind(b) != JV_KIND_NUMBER) \
    return type_error(input, "number required"); \
  jv_free(input); \
  jv ret = jv_number(name(jv_number_value(a), jv_number_value(b))); \
  jv_free(a); \
  jv_free(b); \
  return ret; \
}
#define LIBM_DDD_NO(name)
#include "libm.h"
#undef LIBM_DDD_NO
#undef LIBM_DD_NO
#undef LIBM_DDD
#undef LIBM_DD

static jv f_negate(jq_state *jq, jv input) {
  if (jv_get_kind(input) != JV_KIND_NUMBER) {
    return type_error(input, "cannot be negated");
  }
  jv ret = jv_number(-jv_number_value(input));
  jv_free(input);
  return ret;
}

static jv f_startswith(jq_state *jq, jv a, jv b) {
  if (jv_get_kind(a) != JV_KIND_STRING || jv_get_kind(b) != JV_KIND_STRING)
    return jv_invalid_with_msg(jv_string("startswith() requires string inputs"));
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
  if (jv_get_kind(a) != JV_KIND_STRING || jv_get_kind(b) != JV_KIND_STRING)
    return jv_invalid_with_msg(jv_string("endswith() requires string inputs"));
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
  jv_kind ak = jv_get_kind(a);
  jv_kind bk = jv_get_kind(b);
  jv_free(input);
  if (ak == JV_KIND_NUMBER && bk == JV_KIND_NUMBER) {
    return jv_number(jv_number_value(a) * jv_number_value(b));
  } else if ((ak == JV_KIND_STRING && bk == JV_KIND_NUMBER) ||
             (ak == JV_KIND_NUMBER && bk == JV_KIND_STRING)) {
    jv str = a;
    jv num = b;
    if (ak == JV_KIND_NUMBER) {
      str = b;
      num = a;
    }
    int n;
    size_t alen = jv_string_length_bytes(jv_copy(str));
    jv res = str;

    for (n = jv_number_value(num) - 1; n > 0; n--)
      res = jv_string_append_buf(res, jv_string_value(str), alen);

    jv_free(num);
    if (n < 0) {
      jv_free(str);
      return jv_null();
    }
    return res;
  } else if (ak == JV_KIND_OBJECT && bk == JV_KIND_OBJECT) {
    return jv_object_merge_recursive(a, b);
  } else {
    return type_error2(a, b, "cannot be multiplied");
  }
}

static jv f_divide(jq_state *jq, jv input, jv a, jv b) {
  jv_free(input);
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    if (jv_number_value(b) == 0.0)
      return type_error2(a, b, "cannot be divided because the divisor is zero");
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
    if ((intmax_t)jv_number_value(b) == 0)
      return type_error2(a, b, "cannot be divided (remainder) because the divisor is zero");
    return jv_number((intmax_t)jv_number_value(a) % (intmax_t)jv_number_value(b));
  } else {
    return type_error2(a, b, "cannot be divided (remainder)");
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
  lookup[0] = "\\0";
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
  } else if (!strcmp(fmt_s, "csv") || !strcmp(fmt_s, "tsv")) {
    const char *quotes, *sep, *escapings;
    const char *msg;
    if (!strcmp(fmt_s, "csv")) {
      msg = "cannot be csv-formatted, only array";
      quotes = "\"";
      sep = ",";
      escapings = "\"\"\"\0";
    } else {
      msg = "cannot be tsv-formatted, only array";
      assert(!strcmp(fmt_s, "tsv"));
      quotes = "";
      sep = "\t";
      escapings = "\t\\t\0\r\\r\0\n\\n\0\\\\\\\0";
    }
    jv_free(fmt);
    if (jv_get_kind(input) != JV_KIND_ARRAY)
      return type_error(input, msg);
    jv line = jv_string("");
    jv_array_foreach(input, i, x) {
      if (i) line = jv_string_append_str(line, sep);
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
        line = jv_string_append_str(line, quotes);
        line = jv_string_concat(line, escape_string(x, escapings));
        line = jv_string_append_str(line, quotes);
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
        line = jv_string_concat(line, jv_string_fmt("%%%02X", ch));
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
    const unsigned char* data = (const unsigned char*)jv_string_value(input);
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

static jv f_keys_unsorted(jq_state *jq, jv input) {
  if (jv_get_kind(input) == JV_KIND_OBJECT || jv_get_kind(input) == JV_KIND_ARRAY) {
    return jv_keys_unsorted(input);
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

#ifdef HAVE_ONIGURUMA
static int f_match_name_iter(const UChar* name, const UChar *name_end, int ngroups,
    int *groups, regex_t *reg, void *arg) {
  jv captures = *(jv*)arg;
  for (int i = 0; i < ngroups; ++i) {
    jv cap = jv_array_get(jv_copy(captures),groups[i]-1);
    if (jv_get_kind(cap) == JV_KIND_OBJECT) {
      cap = jv_object_set(cap, jv_string("name"), jv_string_sized((const char*)name, name_end-name));
      captures = jv_array_set(captures,groups[i]-1,cap);
    } else {
      jv_free(cap);
    }
  }
  *(jv *)arg = captures;
  return 0;
}

static jv f_match(jq_state *jq, jv input, jv regex, jv modifiers, jv testmode) {
  int test = jv_equal(testmode, jv_true());
  jv result;
  int onigret;
  int global = 0;
  regex_t *reg;
  OnigErrorInfo einfo;
  OnigRegion* region;

  if (jv_get_kind(input) != JV_KIND_STRING) {
    jv_free(regex);
    jv_free(modifiers);
    return type_error(input, "cannot be matched, as it is not a string");
  }

  if (jv_get_kind(regex) != JV_KIND_STRING) {
    jv_free(input);
    jv_free(modifiers);
    return type_error(regex, "is not a string");
  }

  OnigOptionType options = ONIG_OPTION_CAPTURE_GROUP;

  if (jv_get_kind(modifiers) == JV_KIND_STRING) {
    jv modarray = jv_string_explode(jv_copy(modifiers));
    jv_array_foreach(modarray, i, mod) {
      switch ((int)jv_number_value(mod)) {
        case 'g':
          global = 1;
          break;
        case 'i':
          options |= ONIG_OPTION_IGNORECASE;
          break;
        case 'x':
          options |= ONIG_OPTION_EXTEND;
          break;
        case 'm':
          options |= ONIG_OPTION_MULTILINE;
          break;
        case 's':
          options |= ONIG_OPTION_SINGLELINE;
          break;
        case 'p':
          options |= ONIG_OPTION_MULTILINE | ONIG_OPTION_SINGLELINE;
          break;
        case 'l':
          options |= ONIG_OPTION_FIND_LONGEST;
          break;
        case 'n':
          options |= ONIG_OPTION_FIND_NOT_EMPTY;
          break;
        default:
          jv_free(input);
          jv_free(regex);
          jv_free(modarray);
          return jv_invalid_with_msg(jv_string_concat(modifiers,
                jv_string(" is not a valid modifier string")));
      }
    }
    jv_free(modarray);
  } else if (jv_get_kind(modifiers) != JV_KIND_NULL) {
    // If it isn't a string or null, then it is the wrong type...
    jv_free(input);
    jv_free(regex);
    return type_error(modifiers, "is not a string");
  }

  jv_free(modifiers);

  onigret = onig_new(&reg, (const UChar*)jv_string_value(regex),
      (const UChar*)(jv_string_value(regex) + jv_string_length_bytes(jv_copy(regex))),
      options, ONIG_ENCODING_UTF8, ONIG_SYNTAX_PERL_NG, &einfo);
  if (onigret != ONIG_NORMAL) {
    UChar ebuf[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str(ebuf, onigret, &einfo);
    jv_free(input);
    jv_free(regex);
    return jv_invalid_with_msg(jv_string_concat(jv_string("Regex failure: "),
          jv_string((char*)ebuf)));
  }
  result = test ? jv_false() : jv_array();
  const char *input_string = jv_string_value(input);
  const UChar* start = (const UChar*)jv_string_value(input);
  const unsigned long length = jv_string_length_bytes(jv_copy(input));
  const UChar* end = start + length;
  region = onig_region_new();
  do {
    onigret = onig_search(reg,
        (const UChar*)jv_string_value(input), end, /* string boundaries */
        start, end, /* search boundaries */
        region, ONIG_OPTION_NONE);
    if (onigret >= 0) {
      if (test) {
        result = jv_true();
        break;
      }

      // Zero-width match
      if (region->end[0] == region->beg[0]) {
        unsigned long idx;
        const char *fr = (const char*)input_string;
        for (idx = 0; fr != input_string+region->beg[0]; idx++) {
          fr += jvp_utf8_decode_length(*fr);
        }
        jv match = jv_object_set(jv_object(), jv_string("offset"), jv_number(idx));
        match = jv_object_set(match, jv_string("length"), jv_number(0));
        match = jv_object_set(match, jv_string("string"), jv_string(""));
        match = jv_object_set(match, jv_string("captures"), jv_array());
        result = jv_array_append(result, match);
        start += 1;
        continue;
      }

      unsigned long idx;
      unsigned long len;
      const char *fr = (const char*)input_string;

      for (idx = len = 0; fr < input_string+region->end[0]; len++) {
        if (fr == input_string+region->beg[0]) idx = len, len=0;
        fr += jvp_utf8_decode_length(*fr);
      }

      jv match = jv_object_set(jv_object(), jv_string("offset"), jv_number(idx));

      unsigned long blen = region->end[0]-region->beg[0];
      match = jv_object_set(match, jv_string("length"), jv_number(len));
      match = jv_object_set(match, jv_string("string"), jv_string_sized(input_string+region->beg[0],blen));
      jv captures = jv_array();
      for (int i = 1; i < region->num_regs; ++i) {
        // Empty capture.
        if (region->beg[i] == region->end[i]) {
          // Didn't match.
          jv cap;
          if (region->beg[i] == -1) {
            cap = jv_object_set(jv_object(), jv_string("offset"), jv_number(-1));
            cap = jv_object_set(cap, jv_string("string"), jv_null());
          } else {
            fr = input_string;
            for (idx = 0; fr != input_string+region->beg[i]; idx++) {
              fr += jvp_utf8_decode_length(*fr);
            }
            cap = jv_object_set(jv_object(), jv_string("offset"), jv_number(idx));
            cap = jv_object_set(cap, jv_string("string"), jv_string(""));
          }
          cap = jv_object_set(cap, jv_string("length"), jv_number(0));
          cap = jv_object_set(cap, jv_string("name"), jv_null());
          captures = jv_array_append(captures, cap);
          continue;
        }
        fr = input_string;
        for (idx = len = 0; fr != input_string+region->end[i]; len++) {
          if (fr == input_string+region->beg[i]) idx = len, len=0;
          fr += jvp_utf8_decode_length(*fr);
        }

        blen = region->end[i]-region->beg[i];
        jv cap = jv_object_set(jv_object(), jv_string("offset"), jv_number(idx));
        cap = jv_object_set(cap, jv_string("length"), jv_number(len));
        cap = jv_object_set(cap, jv_string("string"), jv_string_sized(input_string+region->beg[i],blen));
        cap = jv_object_set(cap, jv_string("name"), jv_null());
        captures = jv_array_append(captures,cap);
      }
      onig_foreach_name(reg,f_match_name_iter,&captures);
      match = jv_object_set(match, jv_string("captures"), captures);
      result = jv_array_append(result, match);
      start = (const UChar*)(input_string+region->end[0]);
      onig_region_free(region,0);
    } else if (onigret == ONIG_MISMATCH) {
      break;
    } else { /* Error */
      UChar ebuf[ONIG_MAX_ERROR_MESSAGE_LEN];
      onig_error_code_to_str(ebuf, onigret, einfo);
      jv_free(result);
      result = jv_invalid_with_msg(jv_string_concat(jv_string("Regex failure: "),
            jv_string((char*)ebuf)));
      break;
    }
  } while (global && start != end);
  onig_region_free(region,1);
  region = NULL;
  if (region)
    onig_region_free(region,1);
  onig_free(reg);
  jv_free(input);
  jv_free(regex);
  return result;
}
#else /* ! HAVE_ONIGURUMA */
static jv f_match(jq_state *jq, jv input, jv regex, jv modifiers, jv testmode) {
  return jv_invalid_with_msg(jv_string("jq was compiled without ONIGURUMA regex libary. match/test/sub and related functions are not available."));
}
#endif /* HAVE_ONIGURUMA */

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

static jv f_isinfinite(jq_state *jq, jv input) {
  jv_kind k = jv_get_kind(input);
  if (k != JV_KIND_NUMBER) {
    jv_free(input);
    return jv_false();
  }
  double n = jv_number_value(input);
  jv_free(input);
  return isinf(n) ? jv_true() : jv_false();
}

static jv f_isnan(jq_state *jq, jv input) {
  jv_kind k = jv_get_kind(input);
  if (k != JV_KIND_NUMBER) {
    jv_free(input);
    return jv_false();
  }
  double n = jv_number_value(input);
  jv_free(input);
  return isnan(n) ? jv_true() : jv_false();
}

static jv f_isnormal(jq_state *jq, jv input) {
  jv_kind k = jv_get_kind(input);
  if (k != JV_KIND_NUMBER) {
    jv_free(input);
    return jv_false();
  }
  double n = jv_number_value(input);
  jv_free(input);
  return isnormal(n) ? jv_true() : jv_false();
}

static jv f_infinite(jq_state *jq, jv input) {
  jv_free(input);
  return jv_number(INFINITY);
}

static jv f_nan(jq_state *jq, jv input) {
  jv_free(input);
  return jv_number(NAN);
}

static jv f_error(jq_state *jq, jv input, jv msg) {
  jv_free(input);
  return jv_invalid_with_msg(msg);
}

// FIXME Should autoconf check for this!
#ifndef WIN32
extern const char **environ;
#endif

static jv f_env(jq_state *jq, jv input) {
  jv_free(input);
  jv env = jv_object();
  const char *var, *val;
  for (const char **e = environ; *e != NULL; e++) {
    var = e[0];
    val = strchr(e[0], '=');
    if (val == NULL)
      env = jv_object_set(env, jv_string(var), jv_null());
    else if (var - val < INT_MAX)
      env = jv_object_set(env, jv_string_sized(var, val - var), jv_string(val + 1));
  }
  return env;
}

static jv f_get_search_list(jq_state *jq, jv input) {
  jv_free(input);
  return jq_get_lib_dirs(jq);
}

static jv f_get_prog_origin(jq_state *jq, jv input) {
  jv_free(input);
  return jq_get_prog_origin(jq);
}

static jv f_get_jq_origin(jq_state *jq, jv input) {
  jv_free(input);
  return jq_get_jq_origin(jq);
}

static jv f_string_split(jq_state *jq, jv a, jv b) {
  if (jv_get_kind(a) != JV_KIND_STRING || jv_get_kind(b) != JV_KIND_STRING) {
    jv_free(a);
    jv_free(b);
    return jv_invalid_with_msg(jv_string("split input and separator must be strings"));
  }
  return jv_string_split(a, b);
}

static jv f_string_explode(jq_state *jq, jv a) {
  if (jv_get_kind(a) != JV_KIND_STRING) {
    jv_free(a);
    return jv_invalid_with_msg(jv_string("explode input must be a string"));
  }
  return jv_string_explode(a);
}

static jv f_string_indexes(jq_state *jq, jv a, jv b) {
  return jv_string_indexes(a, b);
}

static jv f_string_implode(jq_state *jq, jv a) {
  if (jv_get_kind(a) != JV_KIND_ARRAY) {
    jv_free(a);
    return jv_invalid_with_msg(jv_string("implode input must be an array"));
  }
  return jv_string_implode(a);
}

static jv f_setpath(jq_state *jq, jv a, jv b, jv c) { return jv_setpath(a, b, c); }
static jv f_getpath(jq_state *jq, jv a, jv b) { return jv_getpath(a, b); }
static jv f_delpaths(jq_state *jq, jv a, jv b) { return jv_delpaths(a, b); }
static jv f_has(jq_state *jq, jv a, jv b) { return jv_has(a, b); }

static jv f_modulemeta(jq_state *jq, jv a) {
  if (jv_get_kind(a) != JV_KIND_STRING) {
    jv_free(a);
    return jv_invalid_with_msg(jv_string("modulemeta input module name must be a string"));
  }
  return load_module_meta(jq, a);
}

static jv f_input(jq_state *jq, jv input) {
  jv_free(input);
  jq_input_cb cb;
  void *data;
  jq_get_input_cb(jq, &cb, &data);
  if (cb == NULL)
    return jv_invalid_with_msg(jv_string("break"));
  jv v = cb(jq, data);
  if (jv_is_valid(v) || jv_invalid_has_msg(jv_copy(v)))
    return v;
  return jv_invalid_with_msg(jv_string("break"));
}

static jv f_debug(jq_state *jq, jv input) {
  jq_msg_cb cb;
  void *data;
  jq_get_debug_cb(jq, &cb, &data);
  if (cb != NULL)
    cb(data, jv_copy(input));
  return input;
}

static jv f_stderr(jq_state *jq, jv input) {
  jv_dumpf(jv_copy(input), stderr, 0);
  fprintf(stderr, "\n");
  return input;
}

static jv tm2jv(struct tm *tm) {
  return JV_ARRAY(jv_number(tm->tm_year + 1900),
                  jv_number(tm->tm_mon),
                  jv_number(tm->tm_mday),
                  jv_number(tm->tm_hour),
                  jv_number(tm->tm_min),
                  jv_number(tm->tm_sec),
                  jv_number(tm->tm_wday),
                  jv_number(tm->tm_yday));
}

/*
 * mktime() has side-effects and anyways, returns time in the local
 * timezone, not UTC.  We want timegm(), which isn't standard.
 *
 * To make things worse, mktime() tells you what the timezone
 * adjustment is, but you have to #define _BSD_SOURCE to get this
 * field of struct tm on some systems.
 *
 * This is all to blame on POSIX, of course.
 *
 * Our wrapper tries to use timegm() if available, or mktime() and
 * correct for its side-effects if possible.
 *
 * Returns (time_t)-2 if mktime()'s side-effects cannot be corrected.
 */
static time_t my_mktime(struct tm *tm) {
#ifdef HAVE_TIMEGM
  return timegm(tm);
#else /* HAVE_TIMEGM */
  time_t t = mktime(tm);
  if (t == (time_t)-1)
    return t;
#ifdef HAVE_TM_TM_GMT_OFF
  return t + tm.tm_gmtoff;
#elif defined(HAVE_TM_TM_GMT_OFF)
  return t + tm.__tm_gmtoff;
#else
  return (time_t)-2; /* Not supported */
#endif
#endif /* !HAVE_TIMEGM */
}

#ifdef HAVE_STRPTIME
static jv f_strptime(jq_state *jq, jv a, jv b) {
  if (jv_get_kind(a) != JV_KIND_STRING || jv_get_kind(b) != JV_KIND_STRING)
    return jv_invalid_with_msg(jv_string("strptime/1 requires string inputs and arguments"));

  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  const char *input = jv_string_value(a);
  const char *fmt = jv_string_value(b);
  const char *end = strptime(input, fmt, &tm);

  if (end == NULL || (*end != '\0' && !isspace(*end))) {
    jv e = jv_invalid_with_msg(jv_string_fmt("date \"%s\" does not match format \"%s\"", input, fmt));
    jv_free(a);
    jv_free(b);
    return e;
  }
  jv_free(b);
  if (tm.tm_wday == 0 && tm.tm_yday == 0 && my_mktime(&tm) == (time_t)-2) {
    jv_free(a);
    return jv_invalid_with_msg(jv_string("strptime/1 not supported on this platform"));
  }
  jv r = tm2jv(&tm);
  if (*end != '\0')
    r = jv_array_append(r, jv_string(end));
  jv_free(a); // must come after `*end` because `end` is a pointer into `a`'s string
  return r;
}
#else
static jv f_strptime(jq_state *jq, jv a, jv b) {
  jv_free(a);
  jv_free(b);
  return jv_invalid_with_msg(jv_string("strptime/1 not implemented on this platform"));
}
#endif

#define TO_TM_FIELD(t, j, i)                    \
    do {                                        \
      jv n = jv_array_get(jv_copy(j), (i));     \
      if (jv_get_kind(n) != (JV_KIND_NUMBER))   \
        return 0;                               \
      t = jv_number_value(n);                   \
      jv_free(n);                               \
    } while (0)

static int jv2tm(jv a, struct tm *tm) {
  memset(tm, 0, sizeof(*tm));
  TO_TM_FIELD(tm->tm_year, a, 0);
  tm->tm_year -= 1900;
  TO_TM_FIELD(tm->tm_mon,  a, 1);
  TO_TM_FIELD(tm->tm_mday, a, 2);
  TO_TM_FIELD(tm->tm_hour, a, 3);
  TO_TM_FIELD(tm->tm_min,  a, 4);
  TO_TM_FIELD(tm->tm_sec,  a, 5);
  TO_TM_FIELD(tm->tm_wday, a, 6);
  TO_TM_FIELD(tm->tm_yday, a, 7);
  jv_free(a);

  // We use UTC everywhere (gettimeofday, gmtime) and UTC does not do DST.
  // Setting tm_isdst to 0 is done by the memset.
  // tm->tm_isdst = 0;

  // The standard permits the tm structure to contain additional members. We
  // hope it is okay to initialize them to zero, because the standard does not
  // provide an alternative.

  return 1;
}

#undef TO_TM_FIELD

static jv f_mktime(jq_state *jq, jv a) {
  if (jv_get_kind(a) != JV_KIND_ARRAY)
    return jv_invalid_with_msg(jv_string("mktime requires array inputs"));
  if (jv_array_length(jv_copy(a)) < 6)
    return jv_invalid_with_msg(jv_string("mktime requires parsed datetime inputs"));
  struct tm tm;
  if (!jv2tm(a, &tm))
    return jv_invalid_with_msg(jv_string("mktime requires parsed datetime inputs"));
  time_t t = my_mktime(&tm);
  if (t == (time_t)-1)
    return jv_invalid_with_msg(jv_string("invalid gmtime representation"));
  if (t == (time_t)-2)
    return jv_invalid_with_msg(jv_string("mktime not supported on this platform"));
  return jv_number(t);
}

#ifdef HAVE_GMTIME_R
static jv f_gmtime(jq_state *jq, jv a) {
  if (jv_get_kind(a) != JV_KIND_NUMBER)
    return jv_invalid_with_msg(jv_string("gmtime() requires numeric inputs"));
  struct tm tm, *tmp;
  memset(&tm, 0, sizeof(tm));
  double fsecs = jv_number_value(a);
  time_t secs = fsecs;
  jv_free(a);
  tmp = gmtime_r(&secs, &tm);
  if (tmp == NULL)
    return jv_invalid_with_msg(jv_string("errror converting number of seconds since epoch to datetime"));
  a = tm2jv(tmp);
  return jv_array_set(a, 5, jv_number(jv_number_value(jv_array_get(jv_copy(a), 5)) + (fsecs - floor(fsecs))));
}
#elif defined HAVE_GMTIME
static jv f_gmtime(jq_state *jq, jv a) {
  if (jv_get_kind(a) != JV_KIND_NUMBER)
    return jv_invalid_with_msg(jv_string("gmtime requires numeric inputs"));
  struct tm tm, *tmp;
  memset(&tm, 0, sizeof(tm));
  double fsecs = jv_number_value(a);
  time_t secs = fsecs;
  jv_free(a);
  tmp = gmtime(&secs);
  if (tmp == NULL)
    return jv_invalid_with_msg(jv_string("errror converting number of seconds since epoch to datetime"));
  a = tm2jv(tmp);
  return jv_array_set(a, 5, jv_number(jv_number_value(jv_array_get(jv_copy(a), 5)) + (fsecs - floor(fsecs))));
}
#else
static jv f_gmtime(jq_state *jq, jv a) {
  jv_free(a);
  return jv_invalid_with_msg(jv_string("gmtime not implemented on this platform"));
}
#endif

#ifdef HAVE_STRFTIME
static jv f_strftime(jq_state *jq, jv a, jv b) {
  if (jv_get_kind(a) == JV_KIND_NUMBER) {
    a = f_gmtime(jq, a);
  } else if (jv_get_kind(a) != JV_KIND_ARRAY) {
    return jv_invalid_with_msg(jv_string("strftime/1 requires parsed datetime inputs"));
  }
  struct tm tm;
  if (!jv2tm(a, &tm))
    return jv_invalid_with_msg(jv_string("strftime/1 requires parsed datetime inputs")); \
  const char *fmt = jv_string_value(b);
  size_t alloced = strlen(fmt) + 100;
  char *buf = alloca(alloced);
  size_t n = strftime(buf, alloced, fmt, &tm);
  jv_free(b);
  /* POSIX doesn't provide errno values for strftime() failures; weird */
  if (n == 0 || n > alloced)
    return jv_invalid_with_msg(jv_string("strftime/1: unknown system failure"));
  return jv_string(buf);
}
#else
static jv f_strftime(jq_state *jq, jv a) {
  jv_free(a);
  jv_free(b);
  return jv_invalid_with_msg(jv_string("strftime/1 not implemented on this platform"));
}
#endif

#ifdef HAVE_GETTIMEOFDAY
static jv f_now(jq_state *jq, jv a) {
  jv_free(a);
  struct timeval tv;
  if (gettimeofday(&tv, NULL) == -1)
    return jv_number(time(NULL));
  return jv_number(tv.tv_sec + tv.tv_usec / 1000000.0);
}
#else
static jv f_now(jq_state *jq, jv a) {
  jv_free(a);
  return jv_number(time(NULL));
}
#endif

static jv f_current_filename(jq_state *jq) {
  jv r = jq_util_input_get_current_filename(jq);
  if (jv_is_valid(r))
    return r;
  jv_free(r);
  return jv_null();
}
static jv f_current_line(jq_state *jq) {
  return jq_util_input_get_current_line(jq);
}

#define LIBM_DD(name) \
  {(cfunction_ptr)f_ ## name, "_" #name, 1},
#define LIBM_DD_NO(name)

#define LIBM_DDD(name) \
  {(cfunction_ptr)f_ ## name, "_" #name, 3},
#define LIBM_DDD_NO(name)

static const struct cfunction function_list[] = {
#include "libm.h"
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
  {(cfunction_ptr)f_keys_unsorted, "keys_unsorted", 1},
  {(cfunction_ptr)f_startswith, "startswith", 2},
  {(cfunction_ptr)f_endswith, "endswith", 2},
  {(cfunction_ptr)f_ltrimstr, "ltrimstr", 2},
  {(cfunction_ptr)f_rtrimstr, "rtrimstr", 2},
  {(cfunction_ptr)f_string_split, "split", 2},
  {(cfunction_ptr)f_string_explode, "explode", 1},
  {(cfunction_ptr)f_string_implode, "implode", 1},
  {(cfunction_ptr)f_string_indexes, "_strindices", 2},
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
  {(cfunction_ptr)f_isinfinite, "isinfinite", 1},
  {(cfunction_ptr)f_isnan, "isnan", 1},
  {(cfunction_ptr)f_isnormal, "isnormal", 1},
  {(cfunction_ptr)f_infinite, "infinite", 1},
  {(cfunction_ptr)f_nan, "nan", 1},
  {(cfunction_ptr)f_sort, "sort", 1},
  {(cfunction_ptr)f_sort_by_impl, "_sort_by_impl", 2},
  {(cfunction_ptr)f_group_by_impl, "_group_by_impl", 2},
  {(cfunction_ptr)f_min, "min", 1},
  {(cfunction_ptr)f_max, "max", 1},
  {(cfunction_ptr)f_min_by_impl, "_min_by_impl", 2},
  {(cfunction_ptr)f_max_by_impl, "_max_by_impl", 2},
  {(cfunction_ptr)f_error, "error", 2},
  {(cfunction_ptr)f_format, "format", 2},
  {(cfunction_ptr)f_env, "env", 1},
  {(cfunction_ptr)f_get_search_list, "get_search_list", 1},
  {(cfunction_ptr)f_get_prog_origin, "get_prog_origin", 1},
  {(cfunction_ptr)f_get_jq_origin, "get_jq_origin", 1},
  {(cfunction_ptr)f_match, "_match_impl", 4},
  {(cfunction_ptr)f_modulemeta, "modulemeta", 1},
  {(cfunction_ptr)f_input, "_input", 1},
  {(cfunction_ptr)f_debug, "debug", 1},
  {(cfunction_ptr)f_stderr, "stderr", 1},
  {(cfunction_ptr)f_strptime, "strptime", 2},
  {(cfunction_ptr)f_strftime, "strftime", 2},
  {(cfunction_ptr)f_mktime, "mktime", 1},
  {(cfunction_ptr)f_gmtime, "gmtime", 1},
  {(cfunction_ptr)f_now, "now", 1},
  {(cfunction_ptr)f_current_filename, "input_filename", 1},
  {(cfunction_ptr)f_current_line, "input_line_number", 1},
};
#undef LIBM_DDD_NO
#undef LIBM_DD_NO
#undef LIBM_DDD
#undef LIBM_DD

struct bytecoded_builtin { const char* name; block code; };
static block bind_bytecoded_builtins(block b) {
  block builtins = gen_noop();
  {
    struct bytecoded_builtin builtin_defs[] = {
      {"empty", gen_op_simple(BACKTRACK)},
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
    // Note that we can now define `range` as a jq-coded function
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

#define LIBM_DD(name) "def " #name ": _" #name ";",
#define LIBM_DDD(name) "def " #name "(a;b): _" #name "(a;b);",
#define LIBM_DD_NO(name) "def " #name ": \"Error: " #name "() not found at build time\"|error;",
#define LIBM_DDD_NO(name) "def " #name "(a;b): \"Error: " #name "() not found at build time\"|error;",

static const char* const jq_builtins[] = {
  "def error: error(.);",
  "def map(f): [.[] | f];",
  "def select(f): if f then . else empty end;",
  "def sort_by(f): _sort_by_impl(map([f]));",
  "def group_by(f): _group_by_impl(map([f]));",
  "def unique: group_by(.) | map(.[0]);",
  "def unique_by(f): group_by(f) | map(.[0]);",
  "def max_by(f): _max_by_impl(map([f]));",
  "def min_by(f): _min_by_impl(map([f]));",
#include "libm.h"
  "def add: reduce .[] as $x (null; . + $x);",
  "def del(f): delpaths([path(f)]);",
  "def _assign(paths; value): value as $v | reduce path(paths) as $p (.; setpath($p; $v));",
  "def _modify(paths; update): reduce path(paths) as $p (.; setpath($p; getpath($p) | update));",
  "def map_values(f): .[] |= f;",

  // recurse
  "def recurse(f): def r: ., (f | select(. != null) | r); r;",
  "def recurse(f; cond): def r: ., (f | select(cond) | r); r;",
  "def recurse: recurse(.[]?);",
  "def recurse_down: recurse;",

  "def to_entries: [keys_unsorted[] as $k | {key: $k, value: .[$k]}];",
  "def from_entries: map({(.key // .Key // .Name): (if has(\"value\") then .value else .Value end)}) | add | .//={};",
  "def with_entries(f): to_entries | map(f) | from_entries;",
  "def reverse: [.[length - 1 - range(0;length)]];",
  "def indices($i): if type == \"array\" and ($i|type) == \"array\" then .[$i]"
  "  elif type == \"array\" then .[[$i]]"
  "  elif type == \"string\" and ($i|type) == \"string\" then _strindices($i)"
  "  else .[$i] end;",
  "def index($i):   indices($i) | .[0];",       // TODO: optimize
  "def rindex($i):  indices($i) | .[-1:][0];",  // TODO: optimize
  "def paths: path(recurse(if (type|. == \"array\" or . == \"object\") then .[] else empty end))|select(length > 0);",
  "def paths(node_filter): . as $dot|paths|select(. as $p|$dot|getpath($p)|node_filter);",
  "def any(generator; condition):"
  "        [label $out | foreach generator as $i"
  "                 (false;"
  "                  if . then break $out elif $i | condition then true else . end;"
  "                  if . then . else empty end)] | length == 1;",
  "def any(condition): any(.[]; condition);",
  "def any: any(.);",
  "def all(generator; condition): "
  "        [label $out | foreach generator as $i"
  "                 (true;"
  "                  if .|not then break $out elif $i | condition then . else false end;"
  "                  if .|not then . else empty end)] | length == 0;",
  "def all(condition): all(.[]; condition);",
  "def all: all(.);",
  "def isfinite: type == \"number\" and (isinfinite | not);",
  "def arrays: select(type == \"array\");",
  "def objects: select(type == \"object\");",
  "def iterables: arrays, objects;",
  "def booleans: select(type == \"boolean\");",
  "def numbers: select(type == \"number\");",
  "def normals: select(isnormal);",
  "def finites: select(isfinite);",
  "def strings: select(type == \"string\");",
  "def nulls: select(type == \"null\");",
  "def values: select(. != null);",
  "def scalars: select(. == null or . == true or . == false or type == \"number\" or type == \"string\");",
  "def scalars_or_empty: select(. == null or . == true or . == false or type == \"number\" or type == \"string\" or ((type==\"array\" or type==\"object\") and length==0));",
  "def leaf_paths: paths(scalars);",
  "def join($x): reduce .[] as $i (null; (.//\"\") + (if . == null then $i else $x + $i end))//\"\";",
  "def _flatten($x): reduce .[] as $i ([]; if $i | type == \"array\" and $x != 0 then . + ($i | _flatten($x-1)) else . + [$i] end);",
  "def flatten($x): if $x < 0 then error(\"flatten depth must not be negative\") else _flatten($x) end;",
  "def flatten: _flatten(-1);",
  "def range($x): range(0;$x);",
  "def fromdateiso8601: strptime(\"%Y-%m-%dT%H:%M:%SZ\")|mktime;",
  "def todateiso8601: strftime(\"%Y-%m-%dT%H:%M:%SZ\");",
  "def fromdate: fromdateiso8601;",
  "def todate: todateiso8601;",
  "def match(re; mode): _match_impl(re; mode; false)|.[];",
  "def match($val): ($val|type) as $vt | if $vt == \"string\" then match($val; null)"
  "   elif $vt == \"array\" and ($val | length) > 1 then match($val[0]; $val[1])"
  "   elif $vt == \"array\" and ($val | length) > 0 then match($val[0]; null)"
  "   else error( $vt + \" not a string or array\") end;",
  "def test(re; mode): _match_impl(re; mode; true);",
  "def test($val): ($val|type) as $vt | if $vt == \"string\" then test($val; null)"
  "   elif $vt == \"array\" and ($val | length) > 1 then test($val[0]; $val[1])"
  "   elif $vt == \"array\" and ($val | length) > 0 then test($val[0]; null)"
  "   else error( $vt + \" not a string or array\") end;",
  "def capture(re; mods): match(re; mods) | reduce ( .captures | .[] | select(.name != null) | { (.name) : .string } ) as $pair ({}; . + $pair);",
  "def capture($val): ($val|type) as $vt | if $vt == \"string\" then capture($val; null)"
  "   elif $vt == \"array\" and ($val | length) > 1 then capture($val[0]; $val[1])"
  "   elif $vt == \"array\" and ($val | length) > 0 then capture($val[0]; null)"
  "   else error( $vt + \" not a string or array\") end;",
  "def scan(re):"
  "  match(re; \"g\")"
  "  |  if (.captures|length > 0)"
  "      then [ .captures | .[] | .string ]"
  "      else .string"
  "      end ;",
  //
  // If input is an array, then emit a stream of successive subarrays of length n (or less),
  // and similarly for strings.
  "def _nwise(a; $n): if a|length <= $n then a else a[0:$n] , _nwise(a[$n:]; $n) end;",
  "def _nwise($n): _nwise(.; $n);",
  //
  // splits/1 produces a stream; split/1 is retained for backward compatibility.
  "def splits($re; flags): . as $s"
     //  # multiple occurrences of "g" are acceptable
  "  | [ match($re; \"g\" + flags) | (.offset, .offset + .length) ]"
  "  | [0] + . +[$s|length]"
  "  | _nwise(2)"
  "  | $s[.[0]:.[1] ] ;",
  "def splits($re): splits($re; null);",
  //
  // split emits an array for backward compatibility
  "def split($re; flags): [ splits($re; flags) ];",
  //
  // If s contains capture variables, then create a capture object and pipe it to s
  "def sub($re; s):"
  "  . as $in"
  "  | [match($re)]"
  "  | if length == 0 then $in"
  "    else .[0]"
  "    | . as $r"
       //  # create the \"capture\" object:
  "    | reduce ( $r | .captures | .[] | select(.name != null) | { (.name) : .string } ) as $pair"
  "        ({}; . + $pair)"
  "    | $in[0:$r.offset] + s + $in[$r.offset+$r.length:]"
  "    end ;",
  //
  // If s contains capture variables, then create a capture object and pipe it to s
  "def sub($re; s; flags):"
  "  def subg: explode | select(. != 103) | implode;"
  // # "fla" should be flags with all occurrences of g removed; gs should be non-nil if flags has a g
  "  def sub1(fla; gs):"
  "    def mysub:"
  "      . as $in"
  "      | [match($re; fla)]"
  "      | if length == 0 then $in"
  "        else .[0] as $edit"
  "        | ($edit | .offset + .length) as $len"
           //  # create the "capture" object:
  "        | reduce ( $edit | .captures | .[] | select(.name != null) | { (.name) : .string } ) as $pair"
  "            ({}; . + $pair)"
  "        | $in[0:$edit.offset]"
  "          + s"
  "          + ($in[$len:] | if gs then mysub else . end)"
  "        end ;"
  "    mysub ;"
  "    (flags | index(\"g\")) as $gs"
  "    | (flags | if $gs then subg else . end) as $fla"
  "    | sub1($fla; $gs);",
  //
  "def sub($re; s): sub($re; s; \"\");",
  // repeated substitution of re (which may contain named captures)
  "def gsub($re; s; flags): sub($re; s; flags + \"g\");",
  "def gsub($re; s): sub($re; s; \"g\");",

  //#######################################################################
  // range/3, with a `by` expression argument
  "def range($init; $upto; $by): "
  "    def _range: "
  "        if ($by > 0 and . < $upto) or ($by < 0 and . > $upto) then ., ((.+$by)|_range) else . end; "
  "    if $by == 0 then $init else $init|_range end | select(($by > 0 and . < $upto) or ($by < 0 and . > $upto));",
  // generic iterator/generator
  "def while(cond; update): "
  "     def _while: "
  "         if cond then ., (update | _while) else empty end; "
  "     _while;",
  "def until(cond; next): "
  "     def _until: "
  "         if cond then . else (next|_until) end;"
  "     _until;",
  "def limit($n; exp): if $n < 0 then exp else label $out | foreach exp as $item ([$n, null]; if .[0] < 1 then break $out else [.[0] -1, $item] end; .[1]) end;",
  "def first(g): label $out | foreach g as $item ([false, null]; if .[0]==true then break $out else [true, $item] end; .[1]);",
  "def last(g): reduce g as $item (null; $item);",
  "def nth($n; g): if $n < 0 then error(\"nth doesn't support negative indices\") else last(limit($n + 1; g)) end;",
  "def first: .[0];",
  "def last: .[-1];",
  "def nth($n): .[$n];",
  "def combinations:"
  "    if length == 0 then [] else"
  "        .[0][] as $x"
  "          | (.[1:] | combinations) as $y"
  "          | [$x] + $y"
  "    end;",
  "def combinations(n):"
  "    . as $dot"
  "      | [range(n) | $dot]"
  "      | combinations;",
  // # transpose a possibly jagged matrix, quickly;
  // # rows are padded with nulls so the result is always rectangular.
  "def transpose:"
  "  if . == [] then []"
  "  else . as $in"
  "  | (map(length) | max) as $max"
  "  | length as $length"
  "  | reduce range(0; $max) as $j"
  "      ([]; . + [reduce range(0;$length) as $i ([]; . + [ $in[$i][$j] ] )] )"
	      "  end;",
  "def in(xs): . as $x | xs | has($x);",
  "def inside(xs): . as $x | xs | contains($x);",
  "def input: _input;",
  "def repeat(exp): "
  "     def _repeat: "
  "         exp, _repeat;"
  "     _repeat;",
  "def inputs: try repeat(_input) catch if .==\"break\" then empty else .|error end;",
  // # like ruby's downcase - only characters A to Z are affected
  "def ascii_downcase:"
  "  explode | map( if 65 <= . and . <= 90 then . + 32  else . end) | implode;",
  // # like ruby's upcase - only characters a to z are affected
  "def ascii_upcase:"
  "  explode | map( if 97 <= . and . <= 122 then . - 32  else . end) | implode;",

  // Streaming utilities
  "def truncate_stream(stream):"
  "  . as $n | null | stream | . as $input | if (.[0]|length) > $n then setpath([0];$input[0][1:]) else empty end;",
  "def fromstream(i):"
  "  foreach i as $item ("
  "    [null,false,null,false];"
  "    if ($item[0]|length) == 0 then [null,false,.[2],.[3]]"
  "    elif ($item|length) == 1 and ($item[0]|length) < 2 then [null,false,.[0],.[1]]"
  "    else . end |"
  "    . as $state |"
  "    if ($item|length) > 1 and ($item[0]|length) > 0 then"
  "      [.[0]|setpath(($item|.[0]); ($item|.[1])),    "
  "      true,                                         "
  "      $state[2],                                    "
  "      $state[3]]                                    "
  "    else ."
  "    end;"
  "    if ($item[0]|length) == 1 and ($item|length == 1) and .[3] then .[2] else empty end,"
  "    if ($item[0]|length) == 0 then $item[1] else empty end"
  "    );",
  "def tostream:\n"
  "  {string:true,number:true,boolean:true,null:true} as $leaf_types |\n"
  "  . as $dot |\n"
  "  if $leaf_types[$dot|type] or length==0 then [[],$dot]\n"
  "  else\n"
  "    # We really need a _streaming_ form of `keys`.\n"
  "    # We can use `range` for arrays, but not for objects.\n"
  "    keys as $keys |\n"
  "    $keys[-1] as $last|\n"
  "    ((# for each key\n"
  "      $keys[] | . as $key |\n"
  "      $dot[$key] | . as $dot |\n"
  "      # recurse on each key/value\n"
  "      tostream|.[0]|=[$key]+.),\n"
  "     # then add the closing marker\n"
  "     [[$last]])\n"
  "  end;",


  // # Assuming the input array is sorted, bsearch/1 returns
  // # the index of the target if the target is in the input array; and otherwise
  // #  (-1 - ix), where ix is the insertion point that would leave the array sorted.
  // # If the input is not sorted, bsearch will terminate but with irrelevant results.
  "def bsearch(target):"
  "  if length == 0 then -1"
  "  elif length == 1 then"
  "     if target == .[0] then 0 elif target < .[0] then -1 else -2 end"
  "  else . as $in"
  ""   // # state variable: [start, end, answer]
  ""   // # where start and end are the upper and lower offsets to use.
  "    | [0, length-1, null]"
  "    | until( .[0] > .[1] ;"
  "             if .[2] != null then (.[1] = -1)"              // # i.e. break
  "             else"
  "               ( ( (.[1] + .[0]) / 2 ) | floor ) as $mid"
  "               | $in[$mid] as $monkey"
  "               | if $monkey == target  then (.[2] = $mid)"  // # success
  "                 elif .[0] == .[1]     then (.[1] = -1)"    // # failure
  "                 elif $monkey < target then (.[0] = ($mid + 1))"
  "                 else (.[1] = ($mid - 1))"
  "                 end"
  "             end )"
  "    | if .[2] == null then"         // # compute the insertion point
  "         if $in[ .[0] ] < target then (-2 -.[0])"
  "         else (-1 -.[0])"
  "         end"
  "      else .[2]"
  "      end"
  "  end;",
};
#undef LIBM_DDD_NO
#undef LIBM_DD_NO
#undef LIBM_DDD
#undef LIBM_DD


static int builtins_bind_one(jq_state *jq, block* bb, const char* code) {
  struct locfile* src;
  src = locfile_init(jq, "<builtin>", code, strlen(code));
  block funcs;
  int nerrors = jq_parse_library(src, &funcs);
  if (nerrors == 0) {
    *bb = block_bind_referenced(funcs, *bb, OP_IS_CALL_PSEUDO);
  }
  locfile_free(src);
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
