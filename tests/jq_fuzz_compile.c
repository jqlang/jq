#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "jq.h"

int LLVMFuzzerTestOneInput(uint8_t *data, size_t size) {
  // Creat null-terminated string
  char *null_terminated = (char *)malloc(size + 1);
  memcpy(null_terminated, (char *)data, size);
  null_terminated[size] = '\0';

  // Fuzzer entrypoint
  jq_state *jq = NULL;
  jq = jq_init();
  if (jq != NULL) {
    if (jq_compile(jq, null_terminated)) {
      jq_dump_disassembly(jq, 2);
    }
  }
  jq_teardown(&jq);

  // Free the null-terminated string
  free(null_terminated);

  return 0;
}
