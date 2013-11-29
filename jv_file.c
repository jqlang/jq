
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jv.h"

jv jv_load_file(const char* filename, int raw) {
  FILE* file = fopen(filename, "r");
  struct jv_parser* parser;
  jv data;
  if (!file) {
    return jv_invalid_with_msg(jv_string_fmt("Could not open %s: %s",
                                             filename,
                                             strerror(errno)));
  }
  if (raw) {
    data = jv_string("");
  } else {
    data = jv_array();
    parser = jv_parser_new(0);
  }
  while (!feof(file) && !ferror(file)) {
    char buf[4096];
    size_t n = fread(buf, 1, sizeof(buf), file);
    if (raw) {
      data = jv_string_concat(data, jv_string_sized(buf, (int)n));
    } else {
      jv_parser_set_buf(parser, buf, n, !feof(file));
      jv value;
      while (jv_is_valid((value = jv_parser_next(parser))))
        data = jv_array_append(data, value);
      jv_free(value);
    }
  }
  if (!raw)
      jv_parser_free(parser);
  int badread = ferror(file);
  fclose(file);
  if (badread) {
    jv_free(data);
    return jv_invalid_with_msg(jv_string_fmt("Error reading from %s",
                                             filename));
  }
  return data;
}
