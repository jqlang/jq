#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "jv.h"

int LLVMFuzzerTestOneInput(uint8_t *data, size_t size) {
  // Creat null-terminated string
  char *null_terminated = (char *)malloc(size + 1);
  memcpy(null_terminated, (char *)data, size);
  null_terminated[size] = '\0';

  // Fuzzer entrypoint
  jv res = jv_parse(null_terminated);
  jv_free(res);

  // Free the null-terminated string
  free(null_terminated);

  return 0;
}
