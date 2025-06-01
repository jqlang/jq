#include "format_table.h"
#include "jv.h"
#include <string.h>

// Declarations for helpers in other files
jv convert_array_of_objects(jv arr);
jv normalize_rows_object(jv input);
void colorize_terms(char *buf, size_t buflen, const char *input, jv color_terms);
jv my_jv_join(jv input, const char *sep);
void repeat_utf8_string(char *buf, const char *utf8, int count);
void word_to_wordboundary_regex(char *out, size_t outlen, const char *word);

#define MAX_COLS 64

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