#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "jv.h"

int LLVMFuzzerTestOneInput(uint8_t *data, size_t size) {
  // Create null-terminated string
  char *null_terminated = (char *)malloc(size + 1);
  memcpy(null_terminated, (char *)data, size);
  null_terminated[size] = '\0';

  // Use the iterative jv_parser API for streaming mode.
  // The single-shot jv_parse_custom_flags() returns "Unexpected extra JSON
  // values" for any compound JSON in streaming mode, because the streaming
  // parser produces multiple tokens but the single-shot API expects exactly
  // one value.
  jv_parser *parser = jv_parser_new(JV_PARSE_STREAMING);
  jv_parser_set_buf(parser, null_terminated, (int)size, 0);

  jv value;
  while (jv_is_valid(value = jv_parser_next(parser))) {
    jv_free(value);
  }
  jv_free(value);

  jv_parser_free(parser);

  // Free the null-terminated string
  free(null_terminated);

  return 0;
}
