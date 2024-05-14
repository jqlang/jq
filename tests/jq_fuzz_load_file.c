#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "jv.h"

int LLVMFuzzerTestOneInput(uint8_t *data, size_t size) {
  // Create file with fuzzer data
  char filename[256];
  sprintf(filename, "/tmp/libfuzzer.%d", getpid());
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    return 0;
  }
  fwrite(data, size, 1, fp);
  fclose(fp);

  // Fuzz the two version of jv_load_file
  jv data1 = jv_load_file(filename, 1);
  jv_free(data1);
  jv data2 = jv_load_file(filename, 0);
  jv_free(data2);

  // Clean up fuzz file
  unlink(filename);

  return 0;
}
