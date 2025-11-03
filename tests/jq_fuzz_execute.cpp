#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#include "jq.h"
#include "jv.h"

// Fuzzer inspired by /src/jq_test.c
// The goal is to have the fuzzer execute the functions:
// jq_compile -> jv_parse -> jq_next.
extern "C" int LLVMFuzzerTestOneInput(uint8_t *data, size_t size) {
  FuzzedDataProvider fdp(data, size);
  std::string prog_payload = fdp.ConsumeRandomLengthString();
  std::string parse_payload1 = fdp.ConsumeRandomLengthString();
  std::string parse_payload2 = fdp.ConsumeRandomLengthString();

  jq_state *jq = NULL;
  jq = jq_init();
  if (jq != NULL) {
    jq_set_attr(jq, jv_string("JQ_ORIGIN"), jv_string("/tmp/"));

    if (jq_compile(jq, prog_payload.c_str())) {
      // Process to jv_parse and then jv_next
      jv input = jv_parse(parse_payload1.c_str());
      if (jv_is_valid(input)) {
        jq_start(jq, input, 0);
        jv next = jv_parse(parse_payload2.c_str());
        if (jv_is_valid(next)) {
          jv actual = jq_next(jq);
          jv_free(actual);
        }
        jv_free(next);
      } else {
        // Only free if input is invalid as otherwise jq_teardown
        // frees it.
        jv_free(input);
      }
    }
  }
  jq_teardown(&jq);

  return 0;
}
