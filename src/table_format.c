#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "jv.h"

#define MAX_COLS 64

static void repeat_utf8_string(char *buf, const char *utf8, int count) {
    buf[0] = '\0';
    for (int i = 0; i < count; ++i) strcat(buf, utf8);
}

static void word_to_wordboundary_regex(char *out, size_t outlen, const char *word) {
    snprintf(out, outlen, "(^|[^[:alnum:]_])(%s)([^[:alnum:]_]|$)", word);
}

// Colorize all matches of regexes from color_terms (regex array)
static void colorize_terms(char *buf, size_t buflen, const char *input, jv color_terms) {
    static const char *ansi_colors[] = {
        "1;31", "1;32", "1;33", "1;34", "1;35", "1;36",
        "1;41", "1;42", "1;43", "1;44", "1;45", "1;46",
        "1;91", "1;92", "1;93", "1;94", "1;95", "1;96",
        "1;101", "1;102", "1;103", "1;104", "1;105", "1;106"
    };
    int n_colors = sizeof(ansi_colors)/sizeof(ansi_colors[0]);
    int n_terms = jv_get_kind(color_terms) == JV_KIND_ARRAY ? jv_array_length(jv_copy(color_terms)) : 0;

    char temp[4096];
    strncpy(temp, input, sizeof(temp)-1);
    temp[sizeof(temp)-1] = '\0';

    if (n_terms == 0) {
        strncpy(buf, temp, buflen-1);
        buf[buflen-1] = '\0';
        return;
    }

    for (int t = 0; t < n_terms; ++t) {
        jv term = jv_array_get(jv_copy(color_terms), t);
        if (jv_get_kind(term) != JV_KIND_STRING) { jv_free(term); continue; }
        const char *pattern = jv_string_value(term);

        regex_t regex;
        if (regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE)) {
            jv_free(term);
            continue;
        }

        char colored[512];
        int color_idx = t % n_colors;
        char result[4096] = {0};
        char *src = temp;
        regmatch_t match;

        while (!regexec(&regex, src, 1, &match, 0)) {
            strncat(result, src, match.rm_so);

            regmatch_t subs[4];
            // The match is at src+match.rm_so..src+match.rm_eo
            // Try to find submatch 2 (the core word)
            if (regexec(&regex, src + match.rm_so, 4, subs, 0) == 0 && subs[2].rm_so != -1) {
                // Output: everything before submatch 2 (within the match), then the colored submatch 2, then everything after submatch 2 (within the match)
                if (subs[2].rm_so > 0)
                    strncat(result, src + match.rm_so, subs[2].rm_so);
                // Colorize core
                int mlen = subs[2].rm_eo - subs[2].rm_so;
                char matchbuf[512];
                strncpy(matchbuf, src + match.rm_so + subs[2].rm_so, mlen);
                matchbuf[mlen] = '\0';
                snprintf(colored, sizeof(colored), "\033[%sm%s\033[0m", ansi_colors[color_idx], matchbuf);
                strcat(result, colored);
                // Copy suffix (from end of submatch 2 to end of match)
                if (subs[2].rm_eo < match.rm_eo - match.rm_so)
                    strncat(result, src + match.rm_so + subs[2].rm_eo, match.rm_eo - match.rm_so - subs[2].rm_eo);
            } else {
                // fallback: color the whole match
                int n = match.rm_eo - match.rm_so;
                char matchbuf[512];
                strncpy(matchbuf, src + match.rm_so, n);
                matchbuf[n] = '\0';
                snprintf(colored, sizeof(colored), "\033[%sm%s\033[0m", ansi_colors[color_idx], matchbuf);
                strcat(result, colored);
            }
            src += match.rm_eo;
        }
        strcat(result, src);
        strncpy(temp, result, sizeof(temp)-1);
        temp[sizeof(temp)-1] = '\0';
        regfree(&regex);
        jv_free(term);
    }
    strncpy(buf, temp, buflen-1);
    buf[buflen-1] = '\0';
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

jv format_table(jv input) {
    if (jv_get_kind(input) == JV_KIND_NULL ||
        (jv_get_kind(input) == JV_KIND_STRING && strlen(jv_string_value(input)) == 0) ||
        (jv_get_kind(input) == JV_KIND_ARRAY && jv_array_length(jv_copy(input)) == 0)) {
        jv_free(input);
        return jv_string("");
    }

    jv headings = jv_null();
    jv rows = jv_null();
    int have_separator = 0;
    int show_headings = 0;

    // Build color_terms from color_words if present
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
            // fallback to legacy "color_terms" if present
            jv c = jv_object_get(jv_copy(input), jv_string("color_terms"));
            if (jv_get_kind(c) == JV_KIND_ARRAY)
                color_terms = c;
            else
                jv_free(c);
        }
    }
    if (jv_is_valid(color_terms) == 0)
        color_terms = jv_array();

    // Detect input mode
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

    int cols = 0, row_count = 0;
    int col_widths[MAX_COLS] = {0};

    // Determine column count and compute col_widths
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

    // Top border
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

    // Headings row (bold, never colored)
    if (show_headings) {
        char buf[4096] = "│";
        int ncols = jv_array_length(jv_copy(headings));
        for (int j = 0; j < cols; ++j) {
            jv val = (j < ncols) ? jv_array_get(jv_copy(headings), j) : jv_string("");
            jv sval = (jv_get_kind(val) == JV_KIND_STRING) ? jv_copy(val) : jv_dump_string(jv_copy(val), 0);

            char cell[2560];
            snprintf(cell, sizeof(cell), "\033[1m%s\033[0m", jv_string_value(sval)); // bold only

            int pad = col_widths[j] - (int)strlen(jv_string_value(sval));
            strcat(buf, cell);
            memset(buf + strlen(buf), ' ', pad);
            buf[strlen(buf) + pad] = '\0';

            strcat(buf, "│");
            jv_free(sval); jv_free(val);
        }
        lines = jv_array_append(lines, jv_string(buf));
    }

    // Header separator
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

    // Data rows (colorized)
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

    // Bottom border
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