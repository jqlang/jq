#ifdef HAVE_LIBONIG
#include <oniguruma.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "jv.h"

#define MAX_COLS 64

void repeat_utf8_string(char *buf, const char *utf8, int count) {
  buf[0] = '\0';
  for (int i = 0; i < count; ++i) strcat(buf, utf8);
}

void word_to_wordboundary_regex(char *out, size_t outlen, const char *word) {
  snprintf(out, outlen, "(^|[^[:alnum:]_])(%s)([^[:alnum:]_]|$)", word);
}

typedef struct {
  int start;
  int end;
  int pattern_index;
} Match;

static int cmp_match(const void* a, const void* b) {
  const Match* ma = (const Match*)a;
  const Match* mb = (const Match*)b;
  if (ma->start != mb->start) return ma->start - mb->start;
  return mb->end - ma->end;
}

#ifdef HAVE_LIBONIG
void colorize_terms(char *buf, size_t buflen, const char *input, jv color_terms) {
  static const char *ansi_colors[] = {
    "1;31", "1;32", "1;33", "1;34", "1;35", "1;36",
    "1;41", "1;42", "1;43", "1;44", "1;45", "1;46",
    "1;91", "1;92", "1;93", "1;94", "1;95", "1;96",
    "1;101", "1;102", "1;103", "1;104", "1;105", "1;106"
  };
  int n_colors = sizeof(ansi_colors) / sizeof(ansi_colors[0]);
  int n_terms = jv_get_kind(color_terms) == JV_KIND_ARRAY ? jv_array_length(jv_copy(color_terms)) : 0;

  if (n_terms == 0) {
    strncpy(buf, input, buflen-1);
    buf[buflen-1] = '\0';
    return;
  }

  Match matches[128];
  int match_count = 0;
  for (int t = 0; t < n_terms; ++t) {
    jv term = jv_array_get(jv_copy(color_terms), t);
    if (jv_get_kind(term) != JV_KIND_STRING) { jv_free(term); continue; }
    const char *pattern = jv_string_value(term);

    OnigRegex reg;
    OnigErrorInfo einfo;
    int r = onig_new(&reg,
                     (const UChar*)pattern, (const UChar*)(pattern + strlen(pattern)),
                     ONIG_OPTION_IGNORECASE | ONIG_OPTION_EXTEND,
                     ONIG_ENCODING_UTF8, ONIG_SYNTAX_DEFAULT, &einfo);
    if (r != ONIG_NORMAL) { jv_free(term); continue; }

    const UChar* s = (const UChar*)input;
    const UChar* end = s + strlen(input);
    int offset = 0;
    OnigRegion* region = onig_region_new();
    while (onig_search(reg, s, end, s, end, region, 0) >= 0) {
      if (region->num_regs > 2 && region->beg[2] >= 0 && match_count < 128) {
        Match mm = {
          .start = offset + region->beg[2],
          .end = offset + region->end[2],
          .pattern_index = t
        };
        matches[match_count++] = mm;
      }
      int adv = region->end[0] > 0 ? region->end[0] : 1;
      s += adv;
      offset += adv;
      if (s >= end) break;
    }
    onig_region_free(region, 1);
    onig_free(reg);
    jv_free(term);
  }

  qsort(matches, match_count, sizeof(Match), cmp_match);

  bool taken[4096] = {0};
  for (int i = 0; i < match_count; ++i) {
    for (int j = matches[i].start; j < matches[i].end; ++j) {
      if (taken[j]) { matches[i].start = matches[i].end = -1; break; }
    }
    if (matches[i].start != -1) {
      for (int j = matches[i].start; j < matches[i].end; ++j) taken[j] = true;
    }
  }

  int pos = 0, outpos = 0;
  for (int i = 0; i < match_count; ++i) {
    if (matches[i].start == -1) continue;
    if ((size_t)(outpos + (matches[i].start - pos)) < buflen) {
      int n = matches[i].start - pos;
      memcpy(buf + outpos, input + pos, n);
      outpos += n;
    }
    int n = matches[i].end - matches[i].start;
    if ((size_t)(outpos + 32) < buflen) {
      outpos += snprintf(buf + outpos, buflen - outpos, "\033[%sm", ansi_colors[matches[i].pattern_index % n_colors]);
    }
    if ((size_t)(outpos + n) < buflen) {
      memcpy(buf + outpos, input + matches[i].start, n);
      outpos += n;
    }
    if ((size_t)(outpos + 5) < buflen) {
      memcpy(buf + outpos, "\033[0m", 4);
      outpos += 4;
      buf[outpos] = '\0';
    }
    pos = matches[i].end;
  }
  int inputlen = strlen(input);
  if ((size_t)(outpos + (inputlen-pos)) < buflen) {
    memcpy(buf + outpos, input + pos, inputlen - pos);
    outpos += inputlen - pos;
    buf[outpos] = '\0';
  } else {
    buf[outpos] = '\0';
  }
}
#else
void colorize_terms(char *buf, size_t buflen, const char *input, jv color_terms) {
  // Oniguruma not available; just copy input
  strncpy(buf, input, buflen-1);
  buf[buflen-1] = '\0';
}
#endif

jv my_jv_join(jv input, const char *sep) {
  if (jv_get_kind(input) != JV_KIND_ARRAY) {
    jv_free(input);
    return jv_invalid_with_msg(jv_string("input to join must be array"));
  }
  int n = jv_array_length(jv_copy(input));
  size_t total = 1;
  for (int i = 0; i < n; ++i) {
    jv s = jv_array_get(jv_copy(input), i);
    const char* str = jv_string_value(s);
    total += strlen(str);
    jv_free(s);
    if (i < n - 1) total += strlen(sep);
  }
  char *buf = malloc(total);
  buf[0] = '\0';
  for (int i = 0; i < n; ++i) {
    if (i > 0) strcat(buf, sep);
    jv s = jv_array_get(jv_copy(input), i);
    strcat(buf, jv_string_value(s));
    jv_free(s);
  }
  jv res = jv_string(buf);
  free(buf);
  jv_free(input);
  return res;
}