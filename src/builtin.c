#define _BSD_SOURCE
#define _GNU_SOURCE
#ifndef __sun__
# define _XOPEN_SOURCE
# define _XOPEN_SOURCE_EXTENDED 1
#else
# define _XPG6
# define __EXTENSIONS__
#endif
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
#ifdef HAVE_LIBONIG
#include <oniguruma.h>
#endif
#include <string.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
#endif
#include "builtin.h"
#include "compile.h"
#include "jq_parser.h"
#include "bytecode.h"
#include "linker.h"
#include "locfile.h"
#include "jv_unicode.h"
#include "jv_alloc.h"


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

static inline jv ret_error(jv bad, jv msg) {
  jv_free(bad);
  return jv_invalid_with_msg(msg);
}

static inline jv ret_error2(jv bad1, jv bad2, jv msg) {
  jv_free(bad1);
  jv_free(bad2);
  return jv_invalid_with_msg(msg);
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
  jv_free(input); \
  if (jv_get_kind(a) != JV_KIND_NUMBER) { \
    jv_free(b); \
    return type_error(a, "number required"); \
  } \
  if (jv_get_kind(b) != JV_KIND_NUMBER) { \
    jv_free(a); \
    return type_error(b, "number required"); \
  } \
  jv ret = jv_number(name(jv_number_value(a), jv_number_value(b))); \
  jv_free(a); \
  jv_free(b); \
  return ret; \
}
#define LIBM_DDD_NO(name)

#define LIBM_DDDD(name) \
static jv f_ ## name(jq_state *jq, jv input, jv a, jv b, jv c) { \
  jv_free(input); \
  if (jv_get_kind(a) != JV_KIND_NUMBER) { \
    jv_free(b); \
    jv_free(c); \
    return type_error(a, "number required"); \
  } \
  if (jv_get_kind(b) != JV_KIND_NUMBER) { \
    jv_free(a); \
    jv_free(c); \
    return type_error(b, "number required"); \
  } \
  if (jv_get_kind(c) != JV_KIND_NUMBER) { \
    jv_free(a); \
    jv_free(b); \
    return type_error(c, "number required"); \
  } \
  jv ret = jv_number(name(jv_number_value(a), jv_number_value(b), jv_number_value(c))); \
  jv_free(a); \
  jv_free(b); \
  jv_free(c); \
  return ret; \
}
#define LIBM_DDDD_NO(name)
#include "libm.h"
#undef LIBM_DDDD_NO
#undef LIBM_DDD_NO
#undef LIBM_DD_NO
#undef LIBM_DDDD
#undef LIBM_DDD
#undef LIBM_DD

#ifdef HAVE_FREXP
static jv f_frexp(jq_state *jq, jv input) {
  if (jv_get_kind(input) != JV_KIND_NUMBER) {
    return type_error(input, "number required");
  }
  int exp;
  double d = frexp(jv_number_value(input), &exp);
  jv ret = JV_ARRAY(jv_number(d), jv_number(exp));
  jv_free(input);
  return ret;
}
#endif
#ifdef HAVE_MODF
static jv f_modf(jq_state *jq, jv input) {
  if (jv_get_kind(input) != JV_KIND_NUMBER) {
    return type_error(input, "number required");
  }
  double i;
  jv ret = JV_ARRAY(jv_number(modf(jv_number_value(input), &i)));
  jv_free(input);
  return jv_array_append(ret, jv_number(i));
}
#endif
#ifdef HAVE_LGAMMA_R
static jv f_lgamma_r(jq_state *jq, jv input) {
  if (jv_get_kind(input) != JV_KIND_NUMBER) {
    return type_error(input, "number required");
  }
  int sign;
  jv ret = JV_ARRAY(jv_number(lgamma_r(jv_number_value(input), &sign)));
  jv_free(input);
  return jv_array_append(ret, jv_number(sign));
}
#endif

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
    return ret_error2(a, b, jv_string("startswith() requires string inputs"));
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
    return ret_error2(a, b, jv_string("endswith() requires string inputs"));
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

static jv f_utf8bytelength(jq_state *jq, jv input) {
  if (jv_get_kind(input) != JV_KIND_STRING)
    return type_error(input, "only strings have UTF-8 byte length");
  return jv_number(jv_string_length_bytes(input));
}

#define CHARS_ALPHANUM "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"

static const unsigned char BASE64_ENCODE_TABLE[64 + 1] = CHARS_ALPHANUM "+/";
static const unsigned char BASE64_INVALID_ENTRY = 0xFF;
static const unsigned char BASE64_DECODE_TABLE[255] = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  62, // +
  0xFF, 0xFF, 0xFF,
  63, // /
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // 0-9
  0xFF, 0xFF, 0xFF,
  99, // =
  0xFF, 0xFF, 0xFF,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // A-Z
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  // a-z
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};


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
        buf[j] = BASE64_ENCODE_TABLE[(code >> (18 - j*6)) & 0x3f];
      }
      if (n < 3) buf[3] = '=';
      if (n < 2) buf[2] = '=';
      line = jv_string_append_buf(line, buf, sizeof(buf));
    }
    jv_free(input);
    return line;
  } else if (!strcmp(fmt_s, "base64d")) {
    jv_free(fmt);
    input = f_tostring(jq, input);
    const unsigned char* data = (const unsigned char*)jv_string_value(input);
    int len = jv_string_length_bytes(jv_copy(input));
    size_t decoded_len = (3 * len) / 4; // 3 usable bytes for every 4 bytes of input
    char *result = jv_mem_calloc(decoded_len, sizeof(char));
    memset(result, 0, decoded_len * sizeof(char));
    uint32_t ri = 0;
    int input_bytes_read=0;
    uint32_t code = 0;
    for (int i=0; i<len && data[i] != '='; i++) {
      if (BASE64_DECODE_TABLE[data[i]] == BASE64_INVALID_ENTRY) {
        free(result);
        return type_error(input, "is not valid base64 data");
      }

      code <<= 6;
      code |= BASE64_DECODE_TABLE[data[i]];
      input_bytes_read++;

      if (input_bytes_read == 4) {
        result[ri++] = (code >> 16) & 0xFF;
        result[ri++] = (code >> 8) & 0xFF;
        result[ri++] = code & 0xFF;
        input_bytes_read = 0;
        code = 0;
      }
    }
    if (input_bytes_read == 3) {
      result[ri++] = (code >> 10) & 0xFF;
      result[ri++] = (code >> 2) & 0xFF;
    } else if (input_bytes_read == 2) {
      result[ri++] = (code >> 4) & 0xFF;
    } else if (input_bytes_read == 1) {
      free(result);
      return type_error(input, "trailing base64 byte found");
    }

    jv line = jv_string_sized(result, ri);
    jv_free(input);
    free(result);
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

#ifdef HAVE_LIBONIG
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
        for (idx = 0; fr < input_string+region->beg[0]; idx++) {
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
            for (idx = 0; fr < input_string+region->beg[i]; idx++) {
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
        for (idx = len = 0; fr < input_string+region->end[i]; len++) {
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
      onig_error_code_to_str(ebuf, onigret, &einfo);
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
#else /* !HAVE_LIBONIG */
static jv f_match(jq_state *jq, jv input, jv regex, jv modifiers, jv testmode) {
  return jv_invalid_with_msg(jv_string("jq was compiled without ONIGURUMA regex libary. match/test/sub and related functions are not available."));
}
#endif /* HAVE_LIBONIG */

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

static jv f_error(jq_state *jq, jv input) {
  return jv_invalid_with_msg(input);
}

// FIXME Should autoconf check for this!
#ifndef WIN32
extern char **environ;
#endif

static jv f_env(jq_state *jq, jv input) {
  jv_free(input);
  jv env = jv_object();
  const char *var, *val;
  for (char **e = environ; *e != NULL; e++) {
    var = e[0];
    val = strchr(e[0], '=');
    if (val == NULL)
      env = jv_object_set(env, jv_string(var), jv_null());
    else if (var - val < INT_MAX)
      env = jv_object_set(env, jv_string_sized(var, val - var), jv_string(val + 1));
  }
  return env;
}

static jv f_halt(jq_state *jq, jv input) {
  jv_free(input);
  jq_halt(jq, jv_invalid(), jv_invalid());
  return jv_true();
}

static jv f_halt_error(jq_state *jq, jv input, jv a) {
  if (jv_get_kind(a) != JV_KIND_NUMBER) {
    jv_free(a);
    return type_error(input, "halt_error/1: number required");
  }
  jq_halt(jq, a, input);
  return jv_true();
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
    return ret_error2(a, b, jv_string("split input and separator must be strings"));
  }
  return jv_string_split(a, b);
}

static jv f_string_explode(jq_state *jq, jv a) {
  if (jv_get_kind(a) != JV_KIND_STRING) {
    return ret_error(a, jv_string("explode input must be a string"));
  }
  return jv_string_explode(a);
}

static jv f_string_indexes(jq_state *jq, jv a, jv b) {
  return jv_string_indexes(a, b);
}

static jv f_string_implode(jq_state *jq, jv a) {
  if (jv_get_kind(a) != JV_KIND_ARRAY) {
    return ret_error(a, jv_string("implode input must be an array"));
  }
  return jv_string_implode(a);
}

static jv f_setpath(jq_state *jq, jv a, jv b, jv c) { return jv_setpath(a, b, c); }
extern jv _jq_path_append(jq_state *, jv, jv, jv);
static jv f_getpath(jq_state *jq, jv a, jv b) {
  return _jq_path_append(jq, a, b, jv_getpath(jv_copy(a), jv_copy(b)));
}
static jv f_delpaths(jq_state *jq, jv a, jv b) { return jv_delpaths(a, b); }
static jv f_has(jq_state *jq, jv a, jv b) { return jv_has(a, b); }

static jv f_modulemeta(jq_state *jq, jv a) {
  if (jv_get_kind(a) != JV_KIND_STRING) {
    return ret_error(a, jv_string("modulemeta input module name must be a string"));
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

#if defined(WIN32) && !defined(HAVE_SETENV)
static int setenv(const char *var, const char *val, int ovr)
{
  BOOL b;
  char c[2];
  if (!ovr)
  {
    DWORD d;
    d = GetEnvironmentVariableA (var, c, 2);
    if (0 != d && GetLastError () != ERROR_ENVVAR_NOT_FOUND) {
      return d;
    }
  }
  b = SetEnvironmentVariableA (var, val);
  if (b) {
    return 0;
  }
  return 1;
}
#endif

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
#elif HAVE_TM_TM_GMT_OFF

  time_t t = mktime(tm);
  if (t == (time_t)-1)
    return t;
  return t + tm->tm_gmtoff;
#elif HAVE_TM___TM_GMT_OFF
  time_t t = mktime(tm);
  if (t == (time_t)-1)
    return t;
  return t + tm->__tm_gmtoff;
#elif WIN32
  return _mkgmtime(tm);
#else
  char *tz;

  tz = (tz = getenv("TZ")) != NULL ? strdup(tz) : NULL;
  if (tz != NULL)
    setenv("TZ", "", 1);
  time_t t = mktime(tm);
  if (tz != NULL)
    setenv("TZ", tz, 1);
  return t;
#endif
}

/* Compute and set tm_wday */
static void set_tm_wday(struct tm *tm) {
  /*
   * https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week#Gauss.27s_algorithm
   * https://cs.uwaterloo.ca/~alopez-o/math-faq/node73.html
   *
   * Tested with dates from 1900-01-01 through 2100-01-01.  This
   * algorithm produces the wrong day-of-the-week number for dates in
   * the range 1900-01-01..1900-02-28, and for 2100-01-01..2100-02-28.
   * Since this is only needed on OS X and *BSD, we might just document
   * this.
   */
  int century = (1900 + tm->tm_year) / 100;
  int year = (1900 + tm->tm_year) % 100;
  if (tm->tm_mon < 2)
    year--;
  /*
   * The month value in the wday computation below is shifted so that
   * March is 1, April is 2, .., January is 11, and February is 12.
   */
  int mon = tm->tm_mon - 1;
  if (mon < 1)
    mon += 12;
  int wday =
    (tm->tm_mday + (int)floor((2.6 * mon - 0.2)) + year + (int)floor(year / 4.0) + (int)floor(century / 4.0) - 2 * century) % 7;
  if (wday < 0)
    wday += 7;
#if 0
  /* See commentary above */
  assert(wday == tm->tm_wday || tm->tm_wday == 8);
#endif
  tm->tm_wday = wday;
}
/*
 * Compute and set tm_yday.
 *
 */
static void set_tm_yday(struct tm *tm) {
  static const int d[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int mon = tm->tm_mon;
  int year = 1900 + tm->tm_year;
  int leap_day = 0;
  if (tm->tm_mon > 1 &&
      ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
    leap_day = 1;

  /* Bound check index into d[] */
  if (mon < 0)
    mon = -mon;
  if (mon > 11)
    mon %= 12;

  int yday = d[mon] + leap_day + tm->tm_mday - 1;
  assert(yday == tm->tm_yday || tm->tm_yday == 367);
  tm->tm_yday = yday;
}

#ifndef HAVE_STRPTIME
static char *strptime(const char *s, const char *format, struct tm *tm) {
  if (strcmp(format, "%Y-%m-%dT%H:%M:%SZ"))
    return NULL;

  int count, end;
  count = sscanf(s, "%d-%d-%dT%d:%d:%d%n",
                    &tm->tm_year, &tm->tm_mon, &tm->tm_mday,
                    &tm->tm_hour, &tm->tm_min, &tm->tm_sec,
                    &end );
  if (count == 6 && s[end] == 'Z') {
    tm->tm_year -= 1900;
    tm->tm_mon--;
    return (char*)s + end + 1;
  }
  return NULL;
}
#endif

static jv f_strptime(jq_state *jq, jv a, jv b) {
  if (jv_get_kind(a) != JV_KIND_STRING || jv_get_kind(b) != JV_KIND_STRING) {
    return ret_error2(a, b, jv_string("strptime/1 requires string inputs and arguments"));
  }

  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  tm.tm_wday = 8; // sentinel
  tm.tm_yday = 367; // sentinel
  const char *input = jv_string_value(a);
  const char *fmt = jv_string_value(b);

#ifndef HAVE_STRPTIME
  if (strcmp(fmt, "%Y-%m-%dT%H:%M:%SZ")) {
    return ret_error2(a, b, jv_string("strptime/1 only supports ISO 8601 on this platform"));
  }
#endif
  const char *end = strptime(input, fmt, &tm);
  if (end == NULL || (*end != '\0' && !isspace(*end))) {
    return ret_error2(a, b, jv_string_fmt("date \"%s\" does not match format \"%s\"", input, fmt));
  }
  jv_free(b);
  /*
   * This is OS X or some *BSD whose strptime() is just not that
   * helpful!
   *
   * We don't know that the format string did involve parsing a
   * year, or a month (if tm->tm_mon == 0).  But with our invalid
   * day-of-week and day-of-year sentinel checks above, the worst
   * this can do is produce garbage.
   */
#ifdef __APPLE__
  /*
   * Apple has made it worse, and different versions of the OS have different
   * behaviors. Some versions just don't touch the fields, but others do, and
   * sometimes provide wrong answers, at that! We can't tell at compile-time
   * which behavior the target system will have, so instead we always use our
   * functions to set these on OS X, and document that %u and %j are
   * unsupported on OS X.
   */
  set_tm_wday(&tm);
  set_tm_yday(&tm);
#else
  if (tm.tm_wday == 8 && tm.tm_mday != 0 && tm.tm_mon >= 0 && tm.tm_mon <= 11)
    set_tm_wday(&tm);
  if (tm.tm_yday == 367 && tm.tm_mday != 0 && tm.tm_mon >= 0 && tm.tm_mon <= 11)
    set_tm_yday(&tm);
#endif
  jv r = tm2jv(&tm);
  if (*end != '\0')
    r = jv_array_append(r, jv_string(end));
  jv_free(a); // must come after `*end` because `end` is a pointer into `a`'s string
  return r;
}

#define TO_TM_FIELD(t, j, i)                    \
    do {                                        \
      jv n = jv_array_get(jv_copy(j), (i));     \
      if (jv_get_kind(n) != (JV_KIND_NUMBER)) { \
        jv_free(j);                             \
        return 0;                               \
      }                                         \
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
    return ret_error(a, jv_string("mktime requires array inputs"));
  if (jv_array_length(jv_copy(a)) < 6)
    return ret_error(a, jv_string("mktime requires parsed datetime inputs"));
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
    return ret_error(a, jv_string("gmtime() requires numeric inputs"));
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
    return ret_error(a, jv_string("gmtime requires numeric inputs"));
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

#ifdef HAVE_LOCALTIME_R
static jv f_localtime(jq_state *jq, jv a) {
  if (jv_get_kind(a) != JV_KIND_NUMBER)
    return ret_error(a, jv_string("localtime() requires numeric inputs"));
  struct tm tm, *tmp;
  memset(&tm, 0, sizeof(tm));
  double fsecs = jv_number_value(a);
  time_t secs = fsecs;
  jv_free(a);
  tmp = localtime_r(&secs, &tm);
  if (tmp == NULL)
    return jv_invalid_with_msg(jv_string("error converting number of seconds since epoch to datetime"));
  a = tm2jv(tmp);
  return jv_array_set(a, 5, jv_number(jv_number_value(jv_array_get(jv_copy(a), 5)) + (fsecs - floor(fsecs))));
}
#elif defined HAVE_GMTIME
static jv f_localtime(jq_state *jq, jv a) {
  if (jv_get_kind(a) != JV_KIND_NUMBER)
    return ret_error(a, jv_string("localtime requires numeric inputs"));
  struct tm tm, *tmp;
  memset(&tm, 0, sizeof(tm));
  double fsecs = jv_number_value(a);
  time_t secs = fsecs;
  jv_free(a);
  tmp = localtime(&secs);
  if (tmp == NULL)
    return jv_invalid_with_msg(jv_string("error converting number of seconds since epoch to datetime"));
  a = tm2jv(tmp);
  return jv_array_set(a, 5, jv_number(jv_number_value(jv_array_get(jv_copy(a), 5)) + (fsecs - floor(fsecs))));
}
#else
static jv f_localtime(jq_state *jq, jv a) {
  jv_free(a);
  return jv_invalid_with_msg(jv_string("localtime not implemented on this platform"));
}
#endif

#ifdef HAVE_STRFTIME
static jv f_strftime(jq_state *jq, jv a, jv b) {
  if (jv_get_kind(a) == JV_KIND_NUMBER) {
    a = f_gmtime(jq, a);
  } else if (jv_get_kind(a) != JV_KIND_ARRAY) {
    return ret_error2(a, b, jv_string("strftime/1 requires parsed datetime inputs"));
  } else if (jv_get_kind(b) != JV_KIND_STRING) {
    return ret_error2(a, b, jv_string("strftime/1 requires a string format"));
  }
  struct tm tm;
  if (!jv2tm(a, &tm))
    return ret_error(b, jv_string("strftime/1 requires parsed datetime inputs"));

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
static jv f_strftime(jq_state *jq, jv a, jv b) {
  jv_free(a);
  jv_free(b);
  return jv_invalid_with_msg(jv_string("strftime/1 not implemented on this platform"));
}
#endif

#ifdef HAVE_STRFTIME
static jv f_strflocaltime(jq_state *jq, jv a, jv b) {
  if (jv_get_kind(a) == JV_KIND_NUMBER) {
    a = f_localtime(jq, a);
  } else if (jv_get_kind(a) != JV_KIND_ARRAY) {
    return ret_error2(a, b, jv_string("strflocaltime/1 requires parsed datetime inputs"));
  } else if (jv_get_kind(b) != JV_KIND_STRING) {
    return ret_error2(a, b, jv_string("strflocaltime/1 requires a string format"));
  }
  struct tm tm;
  if (!jv2tm(a, &tm))
    return jv_invalid_with_msg(jv_string("strflocaltime/1 requires parsed datetime inputs"));
  const char *fmt = jv_string_value(b);
  size_t alloced = strlen(fmt) + 100;
  char *buf = alloca(alloced);
  size_t n = strftime(buf, alloced, fmt, &tm);
  jv_free(b);
  /* POSIX doesn't provide errno values for strftime() failures; weird */
  if (n == 0 || n > alloced)
    return jv_invalid_with_msg(jv_string("strflocaltime/1: unknown system failure"));
  return jv_string(buf);
}
#else
static jv f_strflocaltime(jq_state *jq, jv a, jv b) {
  jv_free(a);
  jv_free(b);
  return jv_invalid_with_msg(jv_string("strflocaltime/1 not implemented on this platform"));
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

static jv f_current_filename(jq_state *jq, jv a) {
  jv_free(a);

  jv r = jq_util_input_get_current_filename(jq);
  if (jv_is_valid(r))
    return r;
  jv_free(r);
  return jv_null();
}
static jv f_current_line(jq_state *jq, jv a) {
  jv_free(a);
  return jq_util_input_get_current_line(jq);
}

#define LIBM_DD(name) \
  {(cfunction_ptr)f_ ## name,  #name, 1},
#define LIBM_DD_NO(name)

#define LIBM_DDD(name) \
  {(cfunction_ptr)f_ ## name, #name, 3},
#define LIBM_DDD_NO(name)

#define LIBM_DDDD(name) \
  {(cfunction_ptr)f_ ## name, #name, 4},
#define LIBM_DDDD_NO(name)

static const struct cfunction function_list[] = {
#include "libm.h"
#ifdef HAVE_FREXP
  {(cfunction_ptr)f_frexp,"frexp", 1},
#endif
#ifdef HAVE_MODF
  {(cfunction_ptr)f_modf,"modf", 1},
#endif
#ifdef HAVE_LGAMMA_R
  {(cfunction_ptr)f_lgamma_r,"lgamma_r", 1},
#endif
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
  {(cfunction_ptr)f_utf8bytelength, "utf8bytelength", 1},
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
  {(cfunction_ptr)f_error, "error", 1},
  {(cfunction_ptr)f_format, "format", 2},
  {(cfunction_ptr)f_env, "env", 1},
  {(cfunction_ptr)f_halt, "halt", 1},
  {(cfunction_ptr)f_halt_error, "halt_error", 2},
  {(cfunction_ptr)f_get_search_list, "get_search_list", 1},
  {(cfunction_ptr)f_get_prog_origin, "get_prog_origin", 1},
  {(cfunction_ptr)f_get_jq_origin, "get_jq_origin", 1},
  {(cfunction_ptr)f_match, "_match_impl", 4},
  {(cfunction_ptr)f_modulemeta, "modulemeta", 1},
  {(cfunction_ptr)f_input, "input", 1},
  {(cfunction_ptr)f_debug, "debug", 1},
  {(cfunction_ptr)f_stderr, "stderr", 1},
  {(cfunction_ptr)f_strptime, "strptime", 2},
  {(cfunction_ptr)f_strftime, "strftime", 2},
  {(cfunction_ptr)f_strflocaltime, "strflocaltime", 2},
  {(cfunction_ptr)f_mktime, "mktime", 1},
  {(cfunction_ptr)f_gmtime, "gmtime", 1},
  {(cfunction_ptr)f_localtime, "localtime", 1},
  {(cfunction_ptr)f_now, "now", 1},
  {(cfunction_ptr)f_current_filename, "input_filename", 1},
  {(cfunction_ptr)f_current_line, "input_line_number", 1},
};
#undef LIBM_DDDD_NO
#undef LIBM_DDD_NO
#undef LIBM_DD_NO
#undef LIBM_DDDD
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
    block rangestart = gen_op_var_fresh(STOREV, "rangestart");
    block range = BLOCK(gen_op_simple(DUP),
                        gen_call("start", gen_noop()),
                        rangestart,
                        gen_call("end", gen_noop()),
                        gen_op_simple(DUP),
                        gen_op_bound(LOADV, rangestart),
                        // Reset rangevar for every value generated by "end"
                        rangevar,
                        gen_op_bound(RANGE, rangevar));
    builtins = BLOCK(builtins, gen_function("range",
                                            BLOCK(gen_param("start"), gen_param("end")),
                                            range));
  }
  return BLOCK(builtins, b);
}

static const char jq_builtins[] =
/* Include jq-coded builtins */
#include "src/builtin.inc"

/* Include unsupported math functions next */
#define LIBM_DD(name)
#define LIBM_DDD(name)
#define LIBM_DDDD(name)
#define LIBM_DD_NO(name) "def " #name ": \"Error: " #name "/0 not found at build time\"|error;"
#define LIBM_DDD_NO(name) "def " #name "(a;b): \"Error: " #name "/2 not found at build time\"|error;"
#define LIBM_DDDD_NO(name) "def " #name "(a;b;c): \"Error: " #name "/3 not found at build time\"|error;"
#include "libm.h"
#ifndef HAVE_FREXP
  "def frexp: \"Error: frexp/0 not found found at build time\"|error;"
#endif
#ifndef HAVE_MODF
  "def modf: \"Error: modf/0 not found found at build time\"|error;"
#endif
#ifndef HAVE_LGAMMA_R
  "def lgamma_r: \"Error: lgamma_r/0 not found found at build time\"|error;"
#endif
;

#undef LIBM_DDDD_NO
#undef LIBM_DDD_NO
#undef LIBM_DD_NO
#undef LIBM_DDDD
#undef LIBM_DDD
#undef LIBM_DD


static block gen_builtin_list(block builtins) {
  jv list = jv_array_append(block_list_funcs(builtins, 1), jv_string("builtins/0"));
  return BLOCK(builtins, gen_function("builtins", gen_noop(), gen_const(list)));
}

int builtins_bind(jq_state *jq, block* bb) {
  block builtins;
  struct locfile* src = locfile_init(jq, "<builtin>", jq_builtins, sizeof(jq_builtins)-1);
  int nerrors = jq_parse_library(src, &builtins);
  assert(!nerrors);
  locfile_free(src);

  builtins = bind_bytecoded_builtins(builtins);
  builtins = gen_cbinding(function_list, sizeof(function_list)/sizeof(function_list[0]), builtins);
  builtins = gen_builtin_list(builtins);

  *bb = block_bind_referenced(builtins, *bb, OP_IS_CALL_PSEUDO);
  return nerrors;
}
