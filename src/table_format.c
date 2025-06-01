#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>
#include "jv.h"

#define MAX_COLS 64

static void repeat_utf8_string(char *buf, const char *utf8, int count) {
    buf[0] = '\0';
    for (int i = 0; i < count; ++i) strcat(buf, utf8);
}

static void word_to_wordboundary_regex(char *out, size_t outlen, const char *word) {
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

static void colorize_terms(char *buf, size_t buflen, const char *input, jv color_terms) {
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

        regex_t regex;
        if (regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE)) { jv_free(term); continue; }

        const char* s = input;
        int offset = 0;
        regmatch_t m[4];
        while (!regexec(&regex, s, 4, m, 0)) {
            if (m[2].rm_so != -1 && match_count < 128) {
                Match mm = {
                    .start = offset + m[2].rm_so,
                    .end = offset + m[2].rm_eo,
                    .pattern_index = t
                };
                matches[match_count++] = mm;
            }
            int adv = (m[0].rm_eo > 0) ? m[0].rm_eo : 1;
            s += adv;
            offset += adv;
        }
        regfree(&regex);
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

static jv my_jv_join(jv input, const char *sep) {
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

jv jv_keys(jv);

// Helper: Given an array of objects, produce {"headings": ..., "rows": ...}
// Preserves key order from the first object.
static jv convert_array_of_objects(jv arr) {
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

// If input is {rows: array of objects, color_words?: ...}, expand it to {headings, rows, color_words?}
static jv normalize_rows_object(jv input) {
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

jv format_table(jv input) {
    if (jv_get_kind(input) == JV_KIND_NULL ||
        (jv_get_kind(input) == JV_KIND_STRING && strlen(jv_string_value(input)) == 0) ||
        (jv_get_kind(input) == JV_KIND_ARRAY && jv_array_length(jv_copy(input)) == 0)) {
        jv_free(input);
        return jv_string("");
    }

    // Normalize: handle {rows: array of objects, color_words?: ...}
    if (jv_get_kind(input) == JV_KIND_OBJECT) {
        input = normalize_rows_object(input);
    }

    if (jv_get_kind(input) == JV_KIND_ARRAY && jv_array_length(jv_copy(input)) > 0) {
        jv first = jv_array_get(jv_copy(input), 0);
        if (jv_get_kind(first) == JV_KIND_OBJECT) {
            jv new_input = convert_array_of_objects(jv_copy(input));
            jv_free(first);
            jv_free(input);
            input = new_input;
        } else {
            jv_free(first);
        }
    }

    jv headings = jv_null();
    jv rows = jv_null();
    int have_separator = 0;
    int show_headings = 0;

    jv color_terms = jv_null();
    if (jv_get_kind(input) == JV_KIND_OBJECT) {
        jv words = jv_object_get(jv_copy(input), jv_string("color_words"));
        if (jv_get_kind(words) == JV_KIND_ARRAY) {
            int n = jv_array_length(jv_copy(words));
            color_terms = jv_array();
            for (int i = 0; i < n; ++i) {
                jv w = jv_array_get(jv_copy(words), i);
                if (jv_get_kind(w) == JV_KIND_STRING) {
                    const char* s = jv_string_value(w);
                    char regex[512];
                    word_to_wordboundary_regex(regex, sizeof(regex), s);
                    color_terms = jv_array_append(color_terms, jv_string(regex));
                }
                jv_free(w);
            }
            jv_free(words);
        } else {
            jv_free(words);
            jv c = jv_object_get(jv_copy(input), jv_string("color_terms"));
            if (jv_get_kind(c) == JV_KIND_ARRAY)
                color_terms = c;
            else
                jv_free(c);
        }
    }
    if (jv_is_valid(color_terms) == 0)
        color_terms = jv_array();

    if (jv_get_kind(input) == JV_KIND_OBJECT) {
        jv h = jv_object_get(jv_copy(input), jv_string("headings"));
        jv r = jv_object_get(jv_copy(input), jv_string("rows"));
        if (jv_get_kind(r) != JV_KIND_ARRAY) {
            jv_free(h); jv_free(r); jv_free(input); jv_free(color_terms);
            return jv_invalid_with_msg(jv_string("Object input to @table must have array 'rows'"));
        }
        rows = r;
        if (jv_get_kind(h) == JV_KIND_ARRAY) {
            headings = h;
            have_separator = 1;
            show_headings = 1;
        } else {
            jv_free(h);
            headings = jv_null();
            have_separator = 0;
            show_headings = 0;
        }
    } else if (jv_get_kind(input) == JV_KIND_ARRAY) {
        int n = jv_array_length(jv_copy(input));
        if (n == 0) {
            jv_free(input); jv_free(color_terms);
            return jv_string("");
        }
        headings = jv_array_get(jv_copy(input), 0);
        rows = jv_array_slice(jv_copy(input), 1, n);
        show_headings = 1;
        have_separator = 0;
    } else {
        jv_free(input); jv_free(color_terms);
        return jv_invalid_with_msg(jv_string("Input to @table must be array-of-arrays or {headings?, rows, color_words?}"));
    }

    // --- PATCH: convert rows (array of objects) to array of arrays if needed
    if (jv_get_kind(headings) == JV_KIND_ARRAY && jv_get_kind(rows) == JV_KIND_ARRAY) {
        int nrows = jv_array_length(jv_copy(rows));
        int ncols = jv_array_length(jv_copy(headings));
        int needs_conversion = 0;
        for (int i = 0; i < nrows; ++i) {
            jv r = jv_array_get(jv_copy(rows), i);
            if (jv_get_kind(r) == JV_KIND_OBJECT) {
                needs_conversion = 1;
                jv_free(r);
                break;
            }
            jv_free(r);
        }
        if (needs_conversion) {
            jv new_rows = jv_array();
            for (int i = 0; i < nrows; ++i) {
                jv obj = jv_array_get(jv_copy(rows), i);
                jv arr_row = jv_array();
                for (int j = 0; j < ncols; ++j) {
                    jv k = jv_array_get(jv_copy(headings), j);
                    jv v = jv_object_get(jv_copy(obj), jv_copy(k));
                    if (!jv_is_valid(v)) v = jv_null();
                    arr_row = jv_array_append(arr_row, v);
                    jv_free(k);
                }
                new_rows = jv_array_append(new_rows, arr_row);
                jv_free(obj);
            }
            jv_free(rows);
            rows = new_rows;
        }
    }
    // --- END PATCH

    int cols = 0, row_count = 0;
    int col_widths[MAX_COLS] = {0};

    if (show_headings) {
        cols = jv_array_length(jv_copy(headings));
        for (int j = 0; j < cols; ++j) {
            jv val = jv_array_get(jv_copy(headings), j);
            jv sval = (jv_get_kind(val) == JV_KIND_STRING) ? jv_copy(val) : jv_dump_string(jv_copy(val), 0);
            int len = (int)strlen(jv_string_value(sval));
            if (len > col_widths[j]) col_widths[j] = len;
            jv_free(sval); jv_free(val);
        }
    }
    row_count = jv_array_length(jv_copy(rows));
    for (int i = 0; i < row_count; ++i) {
        jv row = jv_array_get(jv_copy(rows), i);
        int ncols = jv_array_length(jv_copy(row));
        if (ncols > cols) cols = ncols;
        for (int j = 0; j < ncols; ++j) {
            jv val = jv_array_get(jv_copy(row), j);
            jv sval = (jv_get_kind(val) == JV_KIND_STRING) ? jv_copy(val) : jv_dump_string(jv_copy(val), 0);
            int len = (int)strlen(jv_string_value(sval));
            if (len > col_widths[j]) col_widths[j] = len;
            jv_free(sval); jv_free(val);
        }
        jv_free(row);
    }

    jv lines = jv_array();

    {
        char buf[4096] = "┌";
        for (int j = 0; j < cols; ++j) {
            char tmp[64];
            repeat_utf8_string(tmp, "─", col_widths[j]);
            strcat(buf, tmp);
            strcat(buf, (j == cols - 1) ? "┐" : "┬");
        }
        lines = jv_array_append(lines, jv_string(buf));
    }

    if (show_headings) {
        char buf[4096] = "│";
        int ncols = jv_array_length(jv_copy(headings));
        for (int j = 0; j < cols; ++j) {
            jv val = (j < ncols) ? jv_array_get(jv_copy(headings), j) : jv_string("");
            jv sval = (jv_get_kind(val) == JV_KIND_STRING) ? jv_copy(val) : jv_dump_string(jv_copy(val), 0);

            char cell[2560];
            snprintf(cell, sizeof(cell), "\033[1m%s\033[0m", jv_string_value(sval));

            int pad = col_widths[j] - (int)strlen(jv_string_value(sval));
            strcat(buf, cell);
            memset(buf + strlen(buf), ' ', pad);
            buf[strlen(buf) + pad] = '\0';

            strcat(buf, "│");
            jv_free(sval); jv_free(val);
        }
        lines = jv_array_append(lines, jv_string(buf));
    }

    if (have_separator) {
        char sep[4096] = "├";
        for (int j = 0; j < cols; ++j) {
            char tmp[64];
            repeat_utf8_string(tmp, "─", col_widths[j]);
            strcat(sep, tmp);
            strcat(sep, (j == cols - 1) ? "┤" : "┼");
        }
        lines = jv_array_append(lines, jv_string(sep));
    }

    for (int i = 0; i < row_count; ++i) {
        jv row = jv_array_get(jv_copy(rows), i);
        int ncols = jv_array_length(jv_copy(row));
        char buf[4096] = "│";
        for (int j = 0; j < cols; ++j) {
            jv val = (j < ncols) ? jv_array_get(jv_copy(row), j) : jv_string("");
            jv sval = (jv_get_kind(val) == JV_KIND_STRING) ? jv_copy(val) : jv_dump_string(jv_copy(val), 0);

            char colored[2048];
            colorize_terms(colored, sizeof(colored), jv_string_value(sval), color_terms);

            int pad = col_widths[j] - (int)strlen(jv_string_value(sval));
            strcat(buf, colored);
            memset(buf + strlen(buf), ' ', pad);
            buf[strlen(buf) + pad] = '\0';

            strcat(buf, "│");
            jv_free(sval); jv_free(val);
        }
        lines = jv_array_append(lines, jv_string(buf));
        jv_free(row);
    }

    {
        char buf[4096] = "└";
        for (int j = 0; j < cols; ++j) {
            char tmp[64];
            repeat_utf8_string(tmp, "─", col_widths[j]);
            strcat(buf, tmp);
            strcat(buf, (j == cols - 1) ? "┘" : "┴");
        }
        lines = jv_array_append(lines, jv_string(buf));
    }

    jv_free(headings);
    jv_free(rows);
    jv_free(input);
    jv_free(color_terms);
    return my_jv_join(lines, "\n");
}