#include "jv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLS 64

static void repeat_utf8_string(char *buf, const char *utf8, int count) {
    buf[0] = '\0';
    for (int i = 0; i < count; ++i) strcat(buf, utf8);
}

// Join JV array of strings with separator (returns a jv string)
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
    if (jv_get_kind(input) != JV_KIND_ARRAY) {
        jv_free(input);
        return jv_invalid_with_msg(jv_string("Input to @table must be an array of arrays"));
    }
    int rows = jv_array_length(jv_copy(input));
    if (rows == 0) {
        jv_free(input);
        return jv_string("");
    }
    int cols = 0;
    int col_widths[MAX_COLS] = {0};

    // Validate all rows are arrays and compute column widths
    for (int i = 0; i < rows; ++i) {
        jv row = jv_array_get(jv_copy(input), i);
        if (jv_get_kind(row) != JV_KIND_ARRAY) {
            jv_free(row);
            jv_free(input);
            return jv_invalid_with_msg(jv_string("Each row to @table must be an array"));
        }
        int row_cols = jv_array_length(jv_copy(row));
        if (row_cols > cols) cols = row_cols;
        for (int j = 0; j < row_cols && j < MAX_COLS; ++j) {
            jv val = jv_array_get(jv_copy(row), j);
            jv sval;
            if (jv_get_kind(val) == JV_KIND_STRING) {
                sval = jv_copy(val);
            } else {
                sval = jv_dump_string(jv_copy(val), 0);
            }
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

    // Rows
    for (int i = 0; i < rows; ++i) {
        jv row = jv_array_get(jv_copy(input), i);
        char buf[4096] = "│";
        int n = jv_array_length(jv_copy(row));
        for (int j = 0; j < cols; ++j) {
            jv val = (j < n) ? jv_array_get(jv_copy(row), j) : jv_string("");
            jv sval;
            if (jv_get_kind(val) == JV_KIND_STRING) {
                sval = jv_copy(val);
            } else {
                sval = jv_dump_string(jv_copy(val), 0);
            }
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

        // Add row separator (header/data boundary) after first row
        if (i == 0 && rows > 1) {
            char sep[4096] = "├";
            for (int j = 0; j < cols; ++j) {
                char tmp[64];
                repeat_utf8_string(tmp, "─", col_widths[j]);
                strcat(sep, tmp);
                strcat(sep, (j == cols - 1) ? "┤" : "┼");
            }
            lines = jv_array_append(lines, jv_string(sep));
        }
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

    jv result = my_jv_join(lines, "\n");
    return result;
}