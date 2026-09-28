#include "format_table.h"
#include "jv.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_COLS 64
#define MAX_LINE_LEN 4096
#define MAX_CELL_LEN 1024

// --- Utility helpers ---

static int max(int a, int b) { return a > b ? a : b; }

static void repeat_utf8_string(char *buf, const char *utf8, int count) {
  buf[0] = '\0';
  for (int i = 0; i < count; ++i) strcat(buf, utf8);
}

// Return a string (malloc'd) with value padded to width with spaces on the right
static char* pad_and_copy(const char *src, int width) {
  size_t len = strlen(src);
  char *buf = malloc(width + 1);
  strcpy(buf, src);
  if (width > (int)len) {
    memset(buf + len, ' ', width - len);
  }
  buf[width] = '\0';
  return buf;
}

// Join an array of strings with a separator and return as jv string
static jv join_lines(jv arr, const char *sep) {
  if (jv_get_kind(arr) != JV_KIND_ARRAY) {
    jv_free(arr);
    return jv_invalid_with_msg(jv_string("input to join must be array"));
  }
  int n = jv_array_length(jv_copy(arr));
  size_t total = 1;
  for (int i = 0; i < n; ++i) {
    jv s = jv_array_get(jv_copy(arr), i);
    total += strlen(jv_string_value(s));
    jv_free(s);
    if (i < n - 1) total += strlen(sep);
  }
  char *buf = malloc(total);
  buf[0] = '\0';
  for (int i = 0; i < n; ++i) {
    if (i > 0) strcat(buf, sep);
    jv s = jv_array_get(jv_copy(arr), i);
    strcat(buf, jv_string_value(s));
    jv_free(s);
  }
  jv res = jv_string(buf);
  free(buf);
  jv_free(arr);
  return res;
}

// --- Table helpers ---

// Parse input and extract table headings and rows
static int parse_input(jv input, jv *headings_out, jv *rows_out, int *show_headings, int *have_separator) {
  *headings_out = jv_null();
  *rows_out = jv_null();
  *show_headings = 0;
  *have_separator = 0;

  if (jv_get_kind(input) == JV_KIND_OBJECT) {
    jv h = jv_object_get(jv_copy(input), jv_string("headings"));
    jv r = jv_object_get(jv_copy(input), jv_string("rows"));
    if (jv_get_kind(h) == JV_KIND_ARRAY && jv_get_kind(r) == JV_KIND_ARRAY) {
      *headings_out = h;
      *rows_out = r;
      *show_headings = 1;
      *have_separator = 1;
      return 1;
    } else {
      jv_free(h); jv_free(r);
      return 0;
    }
  } else if (jv_get_kind(input) == JV_KIND_ARRAY) {
    int n = jv_array_length(jv_copy(input));
    if (n == 0) return 1; // table will be empty
    jv first = jv_array_get(jv_copy(input), 0);
    if (jv_get_kind(first) == JV_KIND_ARRAY) {
      *headings_out = jv_array_get(jv_copy(input), 0);
      *rows_out = jv_array_slice(jv_copy(input), 1, n);
      *show_headings = 1;
      *have_separator = 0;
      jv_free(first);
      return 1;
    } else {
      *headings_out = jv_null();
      *rows_out = jv_copy(input);
      *show_headings = 0;
      *have_separator = 0;
      jv_free(first);
      return 1;
    }
  }
  return 0;
}

// Compute column count and widths
static void compute_col_widths(jv headings, jv rows, int show_headings, int *cols, int col_widths[MAX_COLS]) {
  *cols = 0;
  memset(col_widths, 0, sizeof(int) * MAX_COLS);
  if (show_headings && jv_get_kind(headings) == JV_KIND_ARRAY) {
    *cols = jv_array_length(jv_copy(headings));
    for (int j = 0; j < *cols; ++j) {
      jv val = jv_array_get(jv_copy(headings), j);
      jv sval = (jv_get_kind(val) == JV_KIND_STRING) ? jv_copy(val) : jv_dump_string(jv_copy(val), 0);
      int len = (int)strlen(jv_string_value(sval));
      if (len > col_widths[j]) col_widths[j] = len;
      jv_free(sval); jv_free(val);
    }
  }
  int row_count = jv_array_length(jv_copy(rows));
  for (int i = 0; i < row_count; ++i) {
    jv row = jv_array_get(jv_copy(rows), i);
    int ncols = (jv_get_kind(row) == JV_KIND_ARRAY) ? jv_array_length(jv_copy(row)) : 1;
    if (ncols > *cols) *cols = ncols;
    for (int j = 0; j < ncols; ++j) {
      jv val = (jv_get_kind(row) == JV_KIND_ARRAY) ? jv_array_get(jv_copy(row), j) : jv_copy(row);
      jv sval = (jv_get_kind(val) == JV_KIND_STRING) ? jv_copy(val) : jv_dump_string(jv_copy(val), 0);
      int len = (int)strlen(jv_string_value(sval));
      if (len > col_widths[j]) col_widths[j] = len;
      jv_free(sval); jv_free(val);
      if (jv_get_kind(row) != JV_KIND_ARRAY) break;
    }
    jv_free(row);
  }
}

// Format a border (top, separator, bottom)
static jv make_border(int cols, int *col_widths, const char *left, const char *mid, const char *right) {
  char buf[MAX_LINE_LEN] = {0};
  strcat(buf, left);
  for (int j = 0; j < cols; ++j) {
    char tmp[64];
    repeat_utf8_string(tmp, "─", col_widths[j]);
    strcat(buf, tmp);
    strcat(buf, (j == cols - 1) ? right : mid);
  }
  return jv_string(buf);
}

// Format a row (either headings or data)
static jv make_row(jv row, int cols, int *col_widths) {
  char buf[MAX_LINE_LEN] = "│";
  int ncols = (jv_get_kind(row) == JV_KIND_ARRAY) ? jv_array_length(jv_copy(row)) : 1;
  for (int j = 0; j < cols; ++j) {
    jv val = (jv_get_kind(row) == JV_KIND_ARRAY) ?
               ((j < ncols) ? jv_array_get(jv_copy(row), j) : jv_string("")) :
               (j == 0 ? jv_copy(row) : jv_string(""));
    jv sval = (jv_get_kind(val) == JV_KIND_STRING) ? jv_copy(val) : jv_dump_string(jv_copy(val), 0);
    char *cell = pad_and_copy(jv_string_value(sval), col_widths[j]);
    strcat(buf, cell);
    strcat(buf, "│");
    free(cell);
    jv_free(sval); jv_free(val);
  }
  return jv_string(buf);
}

// --- Main entry point ---

jv format_table(jv input) {
  jv headings = jv_null(), rows = jv_null();
  int show_headings = 0, have_separator = 0;

  if (!parse_input(input, &headings, &rows, &show_headings, &have_separator)) {
    jv_free(input);
    return jv_invalid_with_msg(jv_string("Input to @table must be an array, or object with 'headings' and 'rows' arrays"));
  }

  int nrows = jv_array_length(jv_copy(rows));
  if ((!show_headings && nrows == 0) ||
      (show_headings && jv_get_kind(headings) == JV_KIND_ARRAY && jv_array_length(jv_copy(headings)) == 0)) {
    jv_free(headings); jv_free(rows); jv_free(input);
    return jv_string("");
  }

  // Compute columns and column widths
  int cols = 0, col_widths[MAX_COLS] = {0};
  compute_col_widths(headings, rows, show_headings, &cols, col_widths);

  jv lines = jv_array();

  // Top border
  lines = jv_array_append(lines, make_border(cols, col_widths, "┌", "┬", "┐"));

  // Headings
  if (show_headings && jv_get_kind(headings) == JV_KIND_ARRAY) {
    lines = jv_array_append(lines, make_row(headings, cols, col_widths));
  }

  // Separator
  if (have_separator) {
    lines = jv_array_append(lines, make_border(cols, col_widths, "├", "┼", "┤"));
  }

  // Rows
  int row_count = jv_array_length(jv_copy(rows));
  for (int i = 0; i < row_count; ++i) {
    jv row = jv_array_get(jv_copy(rows), i);
    lines = jv_array_append(lines, make_row(row, cols, col_widths));
    jv_free(row);
  }

  // Bottom border
  lines = jv_array_append(lines, make_border(cols, col_widths, "└", "┴", "┘"));

  jv_free(headings); jv_free(rows); jv_free(input);
  return join_lines(lines, "\n");
}