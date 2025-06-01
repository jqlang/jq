#include "jv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLS 64

// Helper: repeat a UTF-8 string (like "─") count times
static void repeat_utf8_string(char *buf, const char *utf8, int count) {
    buf[0] = '\0';
    for (int i = 0; i < count; ++i) strcat(buf, utf8);
}

// Helper: join JV array of strings with separator (returns a jv string)
static jv my_jv_join(jv input, const char *sep) {
    if (jv_get_kind(input) != JV_KIND_ARRAY) {
        jv_free(input);
        return jv_invalid_with_msg(jv_string("input to join must be array"));
    }
    int n = jv_array_length(jv_copy(input));
    size_t total = 1; // for trailing '\0'
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

// Main formatter: takes jv input (array of arrays)
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

    // Find max width for each column
    for (int i = 0; i < rows; ++i) {
        jv row = jv_array_get(jv_copy(input), i);
        if (jv_get_kind(row) != JV_KIND_ARRAY) { jv_free(row); continue; }
        int row_cols = jv_array_length(jv_copy(row));
        if (row_cols > cols) cols = row_cols;
        for (int j = 0; j < row_cols && j < MAX_COLS; ++j) {
            jv val = jv_array_get(jv_copy(row), j);
            jv sval = jv_dump_string(jv_copy(val), 0);
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

    // Header row
    {
        jv header = jv_array_get(jv_copy(input), 0);
        char buf[4096] = "│";
        int n = jv_array_length(jv_copy(header));
        for (int j = 0; j < cols; ++j) {
            jv val = (j < n) ? jv_array_get(jv_copy(header), j) : jv_string("");
            jv sval = jv_dump_string(jv_copy(val), 0);
            const char* s = jv_string_value(sval);
            int pad = col_widths[j] - (int)strlen(s);
            strcat(buf, s);
            memset(buf + strlen(buf), ' ', pad); buf[strlen(buf) + pad] = '\0';
            strcat(buf, "│");
            jv_free(sval); jv_free(val);
        }
        lines = jv_array_append(lines, jv_string(buf));
        jv_free(header);
    }

    // Header separator
    {
        char buf[4096] = "├";
        for (int j = 0; j < cols; ++j) {
            char tmp[64];
            repeat_utf8_string(tmp, "─", col_widths[j]);
            strcat(buf, tmp);
            strcat(buf, (j == cols - 1) ? "┤" : "┼");
        }
        lines = jv_array_append(lines, jv_string(buf));
    }

    // Data rows
    for (int i = 1; i < rows; ++i) {
        jv row = jv_array_get(jv_copy(input), i);
        char buf[4096] = "│";
        int n = jv_array_length(jv_copy(row));
        for (int j = 0; j < cols; ++j) {
            jv val = (j < n) ? jv_array_get(jv_copy(row), j) : jv_string("");
            jv sval = jv_dump_string(jv_copy(val), 0);
            const char* s = jv_string_value(sval);
            int pad = col_widths[j] - (int)strlen(s);
            strcat(buf, s);
            memset(buf + strlen(buf), ' ', pad); buf[strlen(buf) + pad] = '\0';
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

    jv result = my_jv_join(lines, "\n");
    return result;
}
