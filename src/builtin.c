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
#else
#include <errno.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
#include "builtin.h"
#include "compile.h"
#include "jq_parser.h"
#include "bytecode.h"
#include "linker.h"
#include "locfile.h"
#include "jv_unicode.h"
#include "jv_alloc.h"
#include "jv_dtoa.h"
#include "jv_dtoa_tsd.h"
#include "jv_private.h"
#include "util.h"


#define BINOP(name) \
static jv f_ ## name(jq_state *jq, jv input, jv a, jv b) { \
  jv_free(input); \
  return binop_ ## name(a, b); \
}
BINOPS
#undef BINOP


static jv type_error(jv bad, const char* msg) {
  char errbuf[15];
  const char *badkind = jv_kind_name(jv_get_kind(bad));
  jv err = jv_invalid_with_msg(jv_string_fmt("%s (%s) %s", badkind,
                                             jv_dump_string_trunc(bad, errbuf, sizeof(errbuf)),
                                             msg));
  return err;
}

static jv type_error2(jv bad1, jv bad2, const char* msg) {
  char errbuf1[15],errbuf2[15];
  const char *badkind1 = jv_kind_name(jv_get_kind(bad1));
  const char *badkind2 = jv_kind_name(jv_get_kind(bad2));
  jv err = jv_invalid_with_msg(jv_string_fmt("%s (%s) and %s (%s) %s",
                                             badkind1,
                                             jv_dump_string_trunc(bad1, errbuf1, sizeof(errbuf1)),
                                             badkind2,
                                             jv_dump_string_trunc(bad2, errbuf2, sizeof(errbuf2)),
                                             msg));
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

jv binop_plus(jv a, jv b) {
  if (jv_get_kind(a) == JV_KIND_NULL) {
    jv_free(a);
    return b;
  } else if (jv_get_kind(b) == JV_KIND_NULL) {
    jv_free(b);
    return a;
  } else if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    jv r = jv_number(jv_number_value(a) +
                     jv_number_value(b));
    jv_free(a);
    jv_free(b);
    return r;
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

#ifdef __APPLE__
// macOS has a bunch of libm deprecation warnings, so let's clean those up
#ifdef HAVE_TGAMMA
#define HAVE_GAMMA
#define gamma tgamma
#endif
#ifdef HAVE___EXP10
#define HAVE_EXP10
#define exp10 __exp10
#endif
#ifdef HAVE_REMAINDER
#define HAVE_DREM
#define drem remainder
#endif

// We replace significand with our own, since there's not a rename-replacement
#ifdef HAVE_FREXP
static double __jq_significand(double x) {
  int z;
  return 2*frexp(x, &z);
}
#define HAVE_SIGNIFICAND
#define significand __jq_significand
#elif defined(HAVE_SCALBN) && defined(HAVE_ILOGB)
static double __jq_significand(double x) {
  return scalbn(x, -ilogb(x));
}
#define HAVE_SIGNIFICAND
#define significand __jq_significand
#endif

#endif // ifdef __APPLE__

#define LIBM_DD(name) \
static jv f_ ## name(jq_state *jq, jv input) { \
  if (jv_get_kind(input) != JV_KIND_NUMBER) { \
    return type_error(input, "number required"); \
  } \
  jv ret = jv_number(name(jv_number_value(input))); \
  jv_free(input); \
  return ret; \
}
#define LIBM_DD_NO(name) \
static jv f_ ## name(jq_state *jq, jv input) { \
  jv error = jv_string("Error: " #name "/0 not found at build time"); \
  return ret_error(input, error); \
}

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
#define LIBM_DDD_NO(name) \
static jv f_ ## name(jq_state *jq, jv input, jv a, jv b) { \
  jv_free(b); \
  jv error = jv_string("Error: " #name "/2 not found at build time"); \
  return ret_error2(input, a, error); \
}

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
#define LIBM_DDDD_NO(name) \
static jv f_ ## name(jq_state *jq, jv input, jv a, jv b, jv c) { \
  jv_free(c); \
  jv_free(b); \
  jv error = jv_string("Error: " #name "/3 not found at build time"); \
  return ret_error2(input, a, error); \
}

#define LIBM_DA(name, type) \
static jv f_ ## name(jq_state *jq, jv input) { \
  if (jv_get_kind(input) != JV_KIND_NUMBER) { \
    return type_error(input, "number required"); \
  } \
  type value; \
  double d = name(jv_number_value(input), &value); \
  jv ret = JV_ARRAY(jv_number(d), jv_number(value)); \
  jv_free(input); \
  return ret; \
}
#define LIBM_DA_NO(name, type) \
static jv f_ ## name(jq_state *jq, jv input) { \
  jv error = jv_string("Error: " #name "/0 not found at build time"); \
  return ret_error(input, error); \
}

#include "libm.h"
#undef LIBM_DDDD_NO
#undef LIBM_DDD_NO
#undef LIBM_DD_NO
#undef LIBM_DA_NO
#undef LIBM_DDDD
#undef LIBM_DDD
#undef LIBM_DD
#undef LIBM_DA

#ifdef __APPLE__
#undef gamma
#undef drem
#undef significand
#undef exp10
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
  jv ret;

  if (alen < blen ||
     memcmp(astr + (alen - blen), bstr, blen) != 0)
    ret = jv_false();
  else
    ret = jv_true();
  jv_free(a);
  jv_free(b);
  return ret;
}

jv binop_minus(jv a, jv b) {
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    jv r = jv_number(jv_number_value(a) - jv_number_value(b));
    jv_free(a);
    jv_free(b);
    return r;
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

jv binop_multiply(jv a, jv b) {
  jv_kind ak = jv_get_kind(a);
  jv_kind bk = jv_get_kind(b);
  if (ak == JV_KIND_NUMBER && bk == JV_KIND_NUMBER) {
    jv r = jv_number(jv_number_value(a) * jv_number_value(b));
    jv_free(a);
    jv_free(b);
    return r;
  } else if ((ak == JV_KIND_STRING && bk == JV_KIND_NUMBER) ||
             (ak == JV_KIND_NUMBER && bk == JV_KIND_STRING)) {
    jv str = a;
    jv num = b;
    if (ak == JV_KIND_NUMBER) {
      str = b;
      num = a;
    }
    jv res;
    double d = jv_number_value(num);
    if (d < 0 || isnan(d)) {
      res = jv_null();
    } else {
      int n = d;
      size_t alen = jv_string_length_bytes(jv_copy(str));
      res = jv_string_empty(alen * n);
      for (; n > 0; n--) {
        res = jv_string_append_buf(res, jv_string_value(str), alen);
      }
    }
    jv_free(str);
    jv_free(num);
    return res;
  } else if (ak == JV_KIND_OBJECT && bk == JV_KIND_OBJECT) {
    return jv_object_merge_recursive(a, b);
  } else {
    return type_error2(a, b, "cannot be multiplied");
  }
}

jv binop_divide(jv a, jv b) {
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    if (jv_number_value(b) == 0.0)
      return type_error2(a, b, "cannot be divided because the divisor is zero");
    jv r = jv_number(jv_number_value(a) / jv_number_value(b));
    jv_free(a);
    jv_free(b);
    return r;
  } else if (jv_get_kind(a) == JV_KIND_STRING && jv_get_kind(b) == JV_KIND_STRING) {
    return jv_string_split(a, b);
  } else {
    return type_error2(a, b, "cannot be divided");
  }
}

#define dtoi(n) ((n) < INTMAX_MIN ? INTMAX_MIN : -(n) < INTMAX_MIN ? INTMAX_MAX : (intmax_t)(n))
jv binop_mod(jv a, jv b) {
  if (jv_get_kind(a) == JV_KIND_NUMBER && jv_get_kind(b) == JV_KIND_NUMBER) {
    double na = jv_number_value(a);
    double nb = jv_number_value(b);
    if (isnan(na) || isnan(nb)) {
      jv_free(a);
      jv_free(b);
      return jv_number(NAN);
    }
    intmax_t bi = dtoi(nb);
    if (bi == 0)
      return type_error2(a, b, "cannot be divided (remainder) because the divisor is zero");
    // Check if the divisor is -1 to avoid overflow when the dividend is INTMAX_MIN.
    jv r = jv_number(bi == -1 ? 0 : dtoi(na) % bi);
    jv_free(a);
    jv_free(b);
    return r;
  } else {
    return type_error2(a, b, "cannot be divided (remainder)");
  }
}
#undef dtoi

jv binop_equal(jv a, jv b) {
  return jv_bool(jv_equal(a, b));
}

jv binop_notequal(jv a, jv b) {
  return jv_bool(!jv_equal(a, b));
}

enum cmp_op {
  CMP_OP_LESS,
  CMP_OP_GREATER,
  CMP_OP_LESSEQ,
  CMP_OP_GREATEREQ
};

static jv order_cmp(jv a, jv b, enum cmp_op op) {
  int r = jv_cmp(a, b);
  return jv_bool((op == CMP_OP_LESS && r < 0) ||
                 (op == CMP_OP_LESSEQ && r <= 0) ||
                 (op == CMP_OP_GREATEREQ && r >= 0) ||
                 (op == CMP_OP_GREATER && r > 0));
}

jv binop_less(jv a, jv b) {
  return order_cmp(a, b, CMP_OP_LESS);
}

jv binop_greater(jv a, jv b) {
  return order_cmp(a, b, CMP_OP_GREATER);
}

jv binop_lesseq(jv a, jv b) {
  return order_cmp(a, b, CMP_OP_LESSEQ);
}

jv binop_greatereq(jv a, jv b) {
  return order_cmp(a, b, CMP_OP_GREATEREQ);
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
    const char* s = jv_string_value(input);
#ifdef USE_DECNUM
    jv number = jv_number_with_literal(s);
    if (jv_get_kind(number) == JV_KIND_INVALID) {
      return type_error(input, "cannot be parsed as a number");
    }
#else
    char *end = 0;
    double d = jvp_strtod(tsd_dtoa_context_get(), s, &end);
    if (end == 0 || *end != 0) {
      return type_error(input, "cannot be parsed as a number");
    }
    jv number = jv_number(d);
#endif
    jv_free(input);
    return number;
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
    jv r = jv_number(fabs(jv_number_value(input)));
    jv_free(input);
    return r;
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
    const char* p = CHARS_ALPHANUM "-_.~";
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
    size_t decoded_len = (3 * (size_t)len) / 4; // 3 usable bytes for every 4 bytes of input
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
        jv captures = jv_array();
        for (int i = 1; i < region->num_regs; ++i) {
          jv cap = jv_object();
          cap = jv_object_set(cap, jv_string("offset"), jv_number(idx));
          cap = jv_object_set(cap, jv_string("string"), jv_string(""));
          cap = jv_object_set(cap, jv_string("length"), jv_number(0));
          cap = jv_object_set(cap, jv_string("name"), jv_null());
          captures = jv_array_append(captures, cap);
        }
        onig_foreach_name(reg, f_match_name_iter, &captures);
        match = jv_object_set(match, jv_string("captures"), captures);
        result = jv_array_append(result, match);
        // ensure '"qux" | match("(?=u)"; "g")' matches just once
        start = (const UChar*)(input_string+region->end[0]+1);
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
  } while (global && start <= end);
  onig_region_free(region,1);
  region = NULL;
  onig_free(reg);
  jv_free(input);
  jv_free(regex);
  return result;
}
#else /* !HAVE_LIBONIG */
static jv f_match(jq_state *jq, jv input, jv regex, jv modifiers, jv testmode) {
  jv_free(input);
  jv_free(regex);
  jv_free(modifiers);
  jv_free(testmode);
  return jv_invalid_with_msg(jv_string("jq was compiled without ONIGURUMA regex library. match/test/sub and related functions are not available."));
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

enum trim_op {
  TRIM_LEFT  = 1 << 0,
  TRIM_RIGHT = 1 << 1
};

static jv string_trim(jv a, int op) {
  if (jv_get_kind(a) != JV_KIND_STRING) {
    return ret_error(a, jv_string("trim input must be a string"));
  }

  int len = jv_string_length_bytes(jv_copy(a));
  const char *start = jv_string_value(a);
  const char *trim_start = start;
  const char *end = trim_start + len;
  const char *trim_end = end;
  int c;

  if (op & TRIM_LEFT) {
    for (;;) {
      const char *ns = jvp_utf8_next(trim_start, end, &c);
      if (!ns || !jvp_codepoint_is_whitespace(c))
        break;
      trim_start = ns;
    }
  }

  // make sure not empty string or start trim has trimmed everything
  if ((op & TRIM_RIGHT) && trim_end > trim_start) {
    for (;;) {
      const char *ns = jvp_utf8_backtrack(trim_end-1, trim_start, NULL);
      jvp_utf8_next(ns, trim_end, &c);
      if (!jvp_codepoint_is_whitespace(c))
        break;
      trim_end = ns;
      if (ns == trim_start)
        break;
    }
  }

  // no new string needed if there is nothing to trim
  if (trim_start == start && trim_end == end)
    return a;

  jv ts = jv_string_sized(trim_start, trim_end - trim_start);
  jv_free(a);
  return ts;
}

static jv f_string_trim(jq_state *jq, jv a)  { return string_trim(a, TRIM_LEFT | TRIM_RIGHT); }
static jv f_string_ltrim(jq_state *jq, jv a) { return string_trim(a, TRIM_LEFT); }
static jv f_string_rtrim(jq_state *jq, jv a) { return string_trim(a, TRIM_RIGHT); }

static jv f_string_implode(jq_state *jq, jv a) {
  if (jv_get_kind(a) != JV_KIND_ARRAY) {
    return ret_error(a, jv_string("implode input must be an array"));
  }

  int len = jv_array_length(jv_copy(a));
  jv s = jv_string_empty(len);

  for (int i = 0; i < len; i++) {
    jv n = jv_array_get(jv_copy(a), i);
    if (jv_get_kind(n) != JV_KIND_NUMBER || jvp_number_is_nan(n)) {
      jv_free(a);
      jv_free(s);
      return type_error(n, "can't be imploded, unicode codepoint needs to be numeric");
    }

    int nv = jv_number_value(n);
    jv_free(n);
    // outside codepoint range or in utf16 surrogate pair range
    if (nv < 0 || nv > 0x10FFFF || (nv >= 0xD800 && nv <= 0xDFFF))
      nv = 0xFFFD; // U+FFFD REPLACEMENT CHARACTER
    s = jv_string_append_codepoint(s, nv);
  }

  jv_free(a);
  return s;
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
  jq_msg_cb cb;
  void *data;
  jq_get_stderr_cb(jq, &cb, &data);
  if (cb != NULL)
    cb(data, jv_copy(input));
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
#elif defined(WIN32) || !defined(HAVE_STRPTIME)
  set_tm_wday(&tm);
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

static int jv2tm(jv a, struct tm *tm) {
  memset(tm, 0, sizeof(*tm));
  static const size_t offsets[] = {
    offsetof(struct tm, tm_year),
    offsetof(struct tm, tm_mon),
    offsetof(struct tm, tm_mday),
    offsetof(struct tm, tm_hour),
    offsetof(struct tm, tm_min),
    offsetof(struct tm, tm_sec),
    offsetof(struct tm, tm_wday),
    offsetof(struct tm, tm_yday),
  };

  for (size_t i = 0; i < (sizeof offsets / sizeof *offsets); ++i) {
    jv n = jv_array_get(jv_copy(a), i);
    if (!jv_is_valid(n))
      break;
    if (jv_get_kind(n) != JV_KIND_NUMBER || jvp_number_is_nan(n)) {
      jv_free(a);
      jv_free(n);
      return 0;
    }
    double d = jv_number_value(n);
    if (i == 0) /* year */
      d -= 1900;
    *(int *)((void *)tm + offsets[i]) = d < INT_MIN ? INT_MIN :
                                        d > INT_MAX ? INT_MAX : (int)d;
    jv_free(n);
  }

  // We use UTC everywhere (gettimeofday, gmtime) and UTC does not do DST.
  // Setting tm_isdst to 0 is done by the memset.
  // tm->tm_isdst = 0;

  // The standard permits the tm structure to contain additional members. We
  // hope it is okay to initialize them to zero, because the standard does not
  // provide an alternative.

  jv_free(a);
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
    return jv_invalid_with_msg(jv_string("error converting number of seconds since epoch to datetime"));
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
    return jv_invalid_with_msg(jv_string("error converting number of seconds since epoch to datetime"));
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
    if (!jv_is_valid(a)) {
      jv_free(b);
      return a;
    }
  } else if (jv_get_kind(a) != JV_KIND_ARRAY) {
    return ret_error2(a, b, jv_string("strftime/1 requires parsed datetime inputs"));
  }
  if (jv_get_kind(b) != JV_KIND_STRING)
    return ret_error2(a, b, jv_string("strftime/1 requires a string format"));
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
  }
  if (jv_get_kind(b) != JV_KIND_STRING)
    return ret_error2(a, b, jv_string("strflocaltime/1 requires a string format"));
  struct tm tm;
  if (!jv2tm(a, &tm))
    return ret_error(b, jv_string("strflocaltime/1 requires parsed datetime inputs"));
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

static jv f_have_decnum(jq_state *jq, jv a) {
  jv_free(a);
#ifdef USE_DECNUM
  return jv_true();
#else
  return jv_false();
#endif
}

#ifdef WIN32
static jv f_exec(jq_state *jq, jv input, jv path, jv args) {
	jv_free(input), jv_free(path), jv_free(args);
  return jv_invalid_with_msg(jv_string("exec not supported on this platform"));
}
#else
static jv f_exec(jq_state *jq, jv input, jv path, jv args) {
	int ret = 0;

	/* argument validation */
	if (jv_get_kind(path) != JV_KIND_STRING) {
		jv_free(input), jv_free(path), jv_free(args);
		return type_error(path, "exec/2 requires a string path");
	}

	// extract args into const char ** on the stack
	if (jv_get_kind(args) != JV_KIND_ARRAY) {
		jv_free(input), jv_free(path), jv_free(args);
		return type_error(args, "exec/2 requires an array of arguments");
	}

	// validate args array before using it to avoid having to clean up
	// a partially populated argv
	jv_array_foreach(args, i, s) {
		if (jv_get_kind(s) != JV_KIND_STRING) ret++;
		jv_free(s);
	}
	if (ret) {
		jv_free(input), jv_free(path), jv_free(args);
		return type_error(args, "exec/2 only supports string arguments");
	}

	const size_t argc = jv_array_length(jv_copy(args)) + 1;
	char * argv[argc + 1];
	jv_array_foreach(args, i, s) {
		argv[i + 1] = jv_mem_strdup(jv_string_value(s));
		jv_free(s);
	}
	argv[0] = jv_mem_strdup(jv_string_value(path));
	argv[argc] = 0;
	jv_free(path);

	/* setting up pipes */
	int fin[2] = {0, 0}, fout[2] = {0, 0}, ferr[2] = {0, 0};
	posix_spawn_file_actions_t fda;
	if ((ret = posix_spawn_file_actions_init(&fda))) {
		jv_free(args), jv_free(input);
		return jv_invalid_with_msg(jv_string("exec/2 could not initialize fd actions"));
	}

	/** stdin **/
	if ((ret = pipe(fin))) {
		jv_free(args), jv_free(input);
		switch (errno) {
			case EMFILE:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't pipe() because of a per-process limit"));
			case ENFILE:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't pipe() because of a system-wide limit"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't pipe()"));
		}
	}
	if ((ret = posix_spawn_file_actions_addclose(&fda, fin[1]))) {
		jv_free(args), jv_free(input);
		close(fin[0]), close(fin[1]);
		switch (errno) {
			case EBADF:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to close an invalid file descriptor"));
			case EINVAL:
				return jv_invalid_with_msg(jv_string("exec/2 tried to operate on an invalid file_actions object"));
			case ENOMEM:
				return jv_invalid_with_msg(jv_string("exec/2 ran out of memory while instructing the process to close a file descriptor"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to close a file descriptor"));
		}
	}
	if ((ret = posix_spawn_file_actions_adddup2(&fda, fin[0], 0))) {
		jv_free(args), jv_free(input);
		close(fin[0]), close(fin[1]);
		switch (errno) {
			case EBADF:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to dup2 an invalid file descriptor"));
			case ENOMEM:
				return jv_invalid_with_msg(jv_string("exec/2 ran out of memory while instructing the process to dup2 a file descriptor"));
			case EINVAL:
				return jv_invalid_with_msg(jv_string("exec/2 tried to operate on an invalid file_actions object"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to dup2 a file descriptor"));
		}
	}
	if ((ret = posix_spawn_file_actions_addclose(&fda, fin[0]))) {
		jv_free(args), jv_free(input);
		close(fin[0]), close(fin[1]);
		switch (errno) {
			case EBADF:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to close an invalid file descriptor"));
			case EINVAL:
				return jv_invalid_with_msg(jv_string("exec/2 tried to operate on an invalid file_actions object"));
			case ENOMEM:
				return jv_invalid_with_msg(jv_string("exec/2 ran out of memory while instructing the process to close a file descriptor"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to close a file descriptor"));
		}
	}

	/** stdout **/
	if ((ret = pipe(fout))) {
		jv_free(args), jv_free(input);
		close(fin[0]), close(fin[1]);
		switch (errno) {
			case EMFILE:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't pipe() because of a per-process limit"));
			case ENFILE:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't pipe() because of a system-wide limit"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't pipe()"));
		}
	}
	if ((ret = posix_spawn_file_actions_addclose(&fda, fout[0]))) {
		jv_free(args), jv_free(input);
		close(fin[0]), close(fin[1]);
		close(fout[0]), close(fout[1]);
		switch (errno) {
			case EBADF:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to close an invalid file descriptor"));
			case EINVAL:
				return jv_invalid_with_msg(jv_string("exec/2 tried to operate on an invalid file_actions object"));
			case ENOMEM:
				return jv_invalid_with_msg(jv_string("exec/2 ran out of memory while instructing the process to close a file descriptor"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to close a file descriptor"));
		}
	}
	if ((ret = posix_spawn_file_actions_adddup2(&fda, fout[1], 1))) {
		jv_free(args), jv_free(input);
		close(fin[0]), close(fin[1]);
		close(fout[0]), close(fout[1]);
		switch (errno) {
			case EBADF:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to dup2 an invalid file descriptor"));
			case ENOMEM:
				return jv_invalid_with_msg(jv_string("exec/2 ran out of memory while instructing the process to dup2 a file descriptor"));
			case EINVAL:
				return jv_invalid_with_msg(jv_string("exec/2 tried to operate on an invalid file_actions object"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to dup2 a file descriptor"));
		}
	}
	if ((ret = posix_spawn_file_actions_addclose(&fda, fout[1]))) {
		jv_free(args), jv_free(input);
		close(fin[0]), close(fin[1]);
		close(fout[0]), close(fout[1]);
		switch (errno) {
			case EBADF:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to close an invalid file descriptor"));
			case EINVAL:
				return jv_invalid_with_msg(jv_string("exec/2 tried to operate on an invalid file_actions object"));
			case ENOMEM:
				return jv_invalid_with_msg(jv_string("exec/2 ran out of memory while instructing the process to close a file descriptor"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to close a file descriptor"));
		}
	}

	/** stderr **/
	if ((ret = pipe(ferr))) {
		jv_free(args), jv_free(input);
		close(fin[0]), close(fin[1]);
		close(fout[0]), close(fout[1]);
		switch (errno) {
			case EMFILE:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't pipe() because of a per-process limit"));
			case ENFILE:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't pipe() because of a system-wide limit"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't pipe()"));
		}
	}
	if ((ret = posix_spawn_file_actions_addclose(&fda, ferr[0]))) {
		jv_free(args), jv_free(input);
		close(fin[0]), close(fin[1]);
		close(fout[0]), close(fout[1]);
		close(ferr[0]), close(ferr[1]);
		switch (errno) {
			case EBADF:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to close an invalid file descriptor"));
			case EINVAL:
				return jv_invalid_with_msg(jv_string("exec/2 tried to operate on an invalid file_actions object"));
			case ENOMEM:
				return jv_invalid_with_msg(jv_string("exec/2 ran out of memory while instructing the process to close a file descriptor"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to close a file descriptor"));
		}
	}
	if ((ret = posix_spawn_file_actions_adddup2(&fda, ferr[1], 2))) {
		jv_free(args), jv_free(input);
		close(fin[0]), close(fin[1]);
		close(fout[0]), close(fout[1]);
		close(ferr[0]), close(ferr[1]);
		switch (errno) {
			case EBADF:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to dup2 an invalid file descriptor"));
			case ENOMEM:
				return jv_invalid_with_msg(jv_string("exec/2 ran out of memory while instructing the process to dup2 a file descriptor"));
			case EINVAL:
				return jv_invalid_with_msg(jv_string("exec/2 tried to operate on an invalid file_actions object"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to dup2 a file descriptor"));
		}
	}
	if ((ret = posix_spawn_file_actions_addclose(&fda, ferr[1]))) {
		jv_free(args), jv_free(input);
		close(fin[0]), close(fin[1]);
		close(fout[0]), close(fout[1]);
		close(ferr[0]), close(ferr[1]);
		switch (errno) {
			case EBADF:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to close an invalid file descriptor"));
			case EINVAL:
				return jv_invalid_with_msg(jv_string("exec/2 tried to operate on an invalid file_actions object"));
			case ENOMEM:
				return jv_invalid_with_msg(jv_string("exec/2 ran out of memory while instructing the process to close a file descriptor"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 couldn't instruct the process to close a file descriptor"));
		}
	}

	/* execute */
	pid_t pid;
	// NOTE: the warning on argv should be fine, posix_spawnp doesn't mutate those to my knowledge
	if (posix_spawnp(&pid, argv[0], &fda,
				NULL, argv, NULL)) {
		close(fin[0]), close(fin[1]);
		close(fout[0]), close(fout[1]);
		close(ferr[0]), close(ferr[1]);
		jv_free(input);
		switch (errno) {
			case EINVAL:
				return jv_invalid_with_msg(jv_string("exec/2 failed to run posix_spawn due to an invalid file_actions object"));
			default:
				return jv_invalid_with_msg(jv_string("exec/2 failed to run posix_spawn"));
		}
	}
	for (size_t i = 0; i < argc; i++) {
		jv_mem_free(argv[i]);
	}
	close(fin[0]), close(fout[1]), close(ferr[1]);
	jv_free(args);
	if ((ret = posix_spawn_file_actions_destroy(&fda))) {
		// NOTE: posix_spawn_file_actions_destroy isn't checked for errors
		// because it's non-fatal
	}

	/* send and receive data */
	switch (jv_get_kind(input)) {
		case JV_KIND_INVALID:
		case JV_KIND_NULL:
			break; // do not pipe invalid / null
		case JV_KIND_STRING:
			// NOTE: write isn't checked for errors because it's non-fatal
			write(fin[1], jv_string_value(input), jv_string_length_bytes(jv_copy(input)));
			break;
		default: {
			jv s = jv_dump_string(jv_copy(input), 0);
			// NOTE: write isn't checked for errors because it's non-fatal
			write(fin[1], jv_string_value(s), jv_string_length_bytes(jv_copy(s)));
			jv_free(s);
			break;
		}
	}
	close(fin[1]);
	jv_free(input);

	static const size_t bufsize = 1024;
	jv sout = jv_string_empty(0),
	   serr = jv_string_empty(0);
	char *buf = jv_mem_alloc(bufsize);
	ssize_t bytes;
	while ((bytes = read(fout[0], buf, bufsize)) > 0) {
		sout = jv_string_append_buf(sout, buf, bytes);
	}
	// NOTE: if we want to check the read for failures, it'd be done here
	while ((bytes = read(ferr[0], buf, bufsize)) > 0) {
		serr = jv_string_append_buf(serr, buf, bytes);
	}
	// NOTE: if we want to check the read for failures, it'd be done here
	close(fout[0]), close(ferr[0]);
	jv_mem_free(buf);

	if (waitpid(pid, &ret, 0) == -1) {
		jv_free(sout), jv_free(serr);
		switch (errno) {
			case EINTR:
				return jv_invalid_with_msg(jv_string("exec/2 was interrupted by a signal while waiting on the child process"));
			case EINVAL:
				return jv_invalid_with_msg(jv_string("exec/2 passed invalid options to waitpid"));
			// we do not expect ECHILD here, so it's a generic failure
			case ECHILD:
			default:
				return jv_invalid_with_msg(jv_string("exec/2 failed in waitpid"));
		}
	}

	jv obj = jv_object();
	if (WIFEXITED(ret)) {
		obj = jv_object_set(obj, jv_string("status"), jv_number(WEXITSTATUS(ret)));
	} else {
		// POSIX guarantees that this is WIFSIGNALED(ret)
		obj = jv_object_set(obj, jv_string("status"), jv_number(-1));
		obj = jv_object_set(obj, jv_string("signal"), jv_number(WTERMSIG(ret)));
	}
	obj = jv_object_set(obj, jv_string("out"), sout);
	obj = jv_object_set(obj, jv_string("err"), serr);
	return obj;
}
#endif

#define LIBM_DD(name) \
  {f_ ## name, #name, 1},
#define LIBM_DD_NO(name) LIBM_DD(name)
#define LIBM_DA(name, type) LIBM_DD(name)
#define LIBM_DA_NO(name, type) LIBM_DD(name)

#define LIBM_DDD(name) \
  {f_ ## name, #name, 3},
#define LIBM_DDD_NO(name) LIBM_DDD(name)

#define LIBM_DDDD(name) \
  {f_ ## name, #name, 4},
#define LIBM_DDDD_NO(name) LIBM_DDDD(name)

static const struct cfunction function_list[] = {
#include "libm.h"
  {f_negate, "_negate", 1},
#define BINOP(name) {f_ ## name, "_" #name, 3},
BINOPS
#undef BINOP
  {f_dump, "tojson", 1},
  {f_json_parse, "fromjson", 1},
  {f_tonumber, "tonumber", 1},
  {f_tostring, "tostring", 1},
  {f_keys, "keys", 1},
  {f_keys_unsorted, "keys_unsorted", 1},
  {f_startswith, "startswith", 2},
  {f_endswith, "endswith", 2},
  {f_string_split, "split", 2},
  {f_string_explode, "explode", 1},
  {f_string_implode, "implode", 1},
  {f_string_indexes, "_strindices", 2},
  {f_string_trim, "trim", 1},
  {f_string_ltrim, "ltrim", 1},
  {f_string_rtrim, "rtrim", 1},
  {f_setpath, "setpath", 3}, // FIXME typechecking
  {f_getpath, "getpath", 2},
  {f_delpaths, "delpaths", 2},
  {f_has, "has", 2},
  {f_contains, "contains", 2},
  {f_length, "length", 1},
  {f_utf8bytelength, "utf8bytelength", 1},
  {f_type, "type", 1},
  {f_isinfinite, "isinfinite", 1},
  {f_isnan, "isnan", 1},
  {f_isnormal, "isnormal", 1},
  {f_infinite, "infinite", 1},
  {f_nan, "nan", 1},
  {f_sort, "sort", 1},
  {f_sort_by_impl, "_sort_by_impl", 2},
  {f_group_by_impl, "_group_by_impl", 2},
  {f_min, "min", 1},
  {f_max, "max", 1},
  {f_min_by_impl, "_min_by_impl", 2},
  {f_max_by_impl, "_max_by_impl", 2},
  {f_error, "error", 1},
  {f_format, "format", 2},
  {f_env, "env", 1},
  {f_halt, "halt", 1},
  {f_halt_error, "halt_error", 2},
  {f_get_search_list, "get_search_list", 1},
  {f_get_prog_origin, "get_prog_origin", 1},
  {f_get_jq_origin, "get_jq_origin", 1},
  {f_match, "_match_impl", 4},
  {f_modulemeta, "modulemeta", 1},
  {f_input, "input", 1},
  {f_debug, "debug", 1},
  {f_stderr, "stderr", 1},
  {f_strptime, "strptime", 2},
  {f_strftime, "strftime", 2},
  {f_strflocaltime, "strflocaltime", 2},
  {f_mktime, "mktime", 1},
  {f_gmtime, "gmtime", 1},
  {f_localtime, "localtime", 1},
  {f_now, "now", 1},
  {f_current_filename, "input_filename", 1},
  {f_current_line, "input_line_number", 1},
  {f_have_decnum, "have_decnum", 1},
  {f_have_decnum, "have_literal_numbers", 1},
  {f_exec, "exec", 3},
};
#undef LIBM_DDDD_NO
#undef LIBM_DDD_NO
#undef LIBM_DD_NO
#undef LIBM_DA_NO
#undef LIBM_DDDD
#undef LIBM_DDD
#undef LIBM_DD
#undef LIBM_DA

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

static const char jq_builtins[] = {
/* Include jq-coded builtins */
#include "src/builtin.inc"
  '\0',
};

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
