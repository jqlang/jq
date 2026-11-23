# Embed jq in a C program

Link against libjq when an application needs to run a jq filter in process.
Install the libjq development headers and use pkg-config to find the compiler
and linker flags.

~~~sh
cc filter.c $(pkg-config --cflags --libs libjq) -o filter
~~~

This example compiles a filter once, gives it one input, and writes each result
as a separate JSON value. An exhausted filter and a runtime error both return
an invalid `jv`. A runtime error carries a message.

~~~c
#include <stdio.h>
#include <stdlib.h>

#include <jq.h>

int main(void) {
  int status = EXIT_SUCCESS;
  jq_state *jq = jq_init();
  if (jq == NULL)
    return EXIT_FAILURE;

  if (!jq_compile(jq, ".numbers[] | . * 2")) {
    jq_teardown(&jq);
    return EXIT_FAILURE;
  }

  jq_start(jq, jv_parse("{\"numbers\": [1, 2, 3]}"), 0);
  jv result = jq_next(jq);
  while (jv_is_valid(result)) {
    jv_dumpf(result, stdout, 0);
    fputc('\n', stdout);
    result = jq_next(jq);
  }

  if (jv_invalid_has_msg(jv_copy(result))) {
    fputs("jq runtime error: ", stderr);
    jv_dumpf(jv_invalid_get_msg(result), stderr, 0);
    fputc('\n', stderr);
    status = EXIT_FAILURE;
  } else {
    jv_free(result);
  }

  jq_teardown(&jq);
  return status;
}
~~~

The output is `2`, `4`, and `6` on separate lines.

`jq_start` consumes its input value. Each result from `jq_next` belongs to
the caller. In this example `jv_dumpf` consumes the valid results, and
`jv_invalid_get_msg` consumes the error result and passes its message to
`jv_dumpf`. `jv_invalid_has_msg` also consumes its argument, so use `jv_copy`
to check for a message before extracting it. Use `jv_free` for an invalid
result without a message. When accepting JSON from an external source, check
the result of `jv_parse` for a parse error before calling `jq_start`.

For further details, see the [libjq C API wiki page](https://github.com/jqlang/jq/wiki/C-API%3A-libjq) and the [guide to invoking jq programs from C](https://github.com/jqlang/jq/wiki/C-API%3A-jq-program-invocation).
