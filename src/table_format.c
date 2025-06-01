#include "jv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLS 64

static void repeat_utf8_string(char *buf, const char *utf8, int count) {
    buf[0] = '\0';
    for (int i = 0; i < count; ++i) strcat(buf, utf8);
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
    jv headings = jv_null();
    jv rows = jv_null();
    int have_separator = 0;
    int show_headings = 0;

    // Detect input mode
    if (jv_get_kind(input) == JV_KIND_OBJECT) {
        jv h = jv_object_get(jv_copy(input), jv_string("headings"));
        jv r = jv_object_get(jv_copy(input), jv_string("rows"));
        if (jv_get_kind(r) != JV_KIND_ARRAY) {
            jv_free(h); jv_free(r); jv_free(input);
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
            jv_free(input);
            return jv_string("");
        }
        headings = jv_array_get(jv_copy(input), 0);
        rows = jv_array_slice(jv_copy(input), 1, n);
        show_headings = 1;
        have_separator = 0;
    } else {
        jv_free(input);
        return jv_invalid_with_msg(jv_string("Input to @table must be array-of-arrays or {headings?, rows}"));
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
        if (ncols > cols) cols = ncols; // expand if data row is wider
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

    // Headings row
    if (show_headings) {
        char buf[4096] = "│";
        int ncols = jv_array_length(jv_copy(headings));
        for (int j = 0; j < cols; ++j) {
            jv val = (j < ncols) ? jv_array_get(jv_copy(headings), j) : jv_string("");
            jv sval = (jv_get_kind(val) == JV_KIND_STRING) ? jv_copy(val) : jv_dump_string(jv_copy(val), 0);
            const char* s = jv_string_value(sval);
            int pad = col_widths[j] - (int)strlen(s);
            strcat(buf, s);
            memset(buf + strlen(buf), ' ', pad);
            buf[strlen(buf) + pad] = '\0';
            strcat(buf, "│");
            jv_free(sval); jv_free(val);
        }
        lines = jv_array_append(lines, jv_string(buf));
    }

    // Header separator (object input with headings only)
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

    // Data rows
    for (int i = 0; i < row_count; ++i) {
        jv row = jv_array_get(jv_copy(rows), i);
        int ncols = jv_array_length(jv_copy(row));
        char buf[4096] = "│";
        for (int j = 0; j < cols; ++j) {
            jv val = (j < ncols) ? jv_array_get(jv_copy(row), j) : jv_string("");
            jv sval = (jv_get_kind(val) == JV_KIND_STRING) ? jv_copy(val) : jv_dump_string(jv_copy(val), 0);
            const char* s = jv_string_value(sval);
            int pad = col_widths[j] - (int)strlen(s);
            strcat(buf, s);
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
    return my_jv_join(lines, "\n");
}