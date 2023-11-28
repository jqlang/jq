#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "jv.h"

int LLVMFuzzerTestOneInput(uint8_t *data, size_t size) {
  if (size < 8) {
    return 0;
  }

  int fuzz_flags = *(int*)data;
  data += 4;
  size -= 4;
  int dump_flags = *(int*)data;
  data += 4;
  size -= 4;

  // Creat null-terminated string
  char *null_terminated = (char *)malloc(size + 1);
  memcpy(null_terminated, (char *)data, size);
  null_terminated[size] = '\0';

  // Fuzzer entrypoint
  jv res = jv_parse_custom_flags(null_terminated, fuzz_flags);
  if (jv_is_valid(res)) {
    jv_dump(res, dump_flags);
  } else {
    jv_free(res);
  }

  // Free the null-terminated string
  free(null_terminated);

  return 0;
}
