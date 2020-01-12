#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <src/jv.h>
#include <src/jq.h>
#ifndef JQ_PLUGIN
#error "JQ_PLUGIN not defined!"
#endif
#include <src/jq_plugin.h>

static jv f_add1(jq_state *jq, jv input) {
  if (jv_get_kind(input) == JV_KIND_NUMBER) {
    double n = jv_number_value(input);

    return jv_free(input), jv_number(n + 1);
  }
  if (jv_get_kind(input) == JV_KIND_STRING)
    return jv_string_concat(input, jv_string("1"));
  if (jv_get_kind(input) == JV_KIND_ARRAY)
    return jv_array_append(input, jv_number(1));
  return jv_invalid_with_msg(jv_string("add1 only works for numbers, strings, and arrays"));
}

static jv f_urandomn(jq_state *jq, jv input) {
  int fd = open("/dev/urandom", O_RDONLY);
  uint64_t n;
  jv r;

  jv_free(input);

  if (fd == -1)
    return jv_invalid_with_msg(jv_string_fmt("Could not open /dev/urandom: %s",
                                             strerror(errno)));

  if (read(fd, &n, sizeof(n)) != sizeof(n))
    return jv_invalid_with_msg(jv_string("Could not read enough bytes from /dev/urandom"));

  r = jv_number(n & ~((1ULL<<53)-1));
  (void) close(fd);
  return r;
}

struct cfunction my_cfuncs[] = {
  { (cfunction_ptr)f_add1, "_add1", 1, 1, 1},
  { (cfunction_ptr)f_urandomn, "_urandomn", 1, 0, 0},
};

int jq_plugin_init(int min_abi,
                   int max_abi,
                   struct jq_state *jq,
                   const char **err,
                   const char **contents,
                   size_t *contentssz,
                   struct cfunction **cf,
                   size_t *ncf)

{
  *contents = *err = 0;
  *ncf = *contentssz = 0;
  *cf = 0;

  if (JQ_CURRENT_ABI < min_abi || JQ_CURRENT_ABI > max_abi) {
    *err = "Wrong ABI version";
    return 1;
  }

  /*
   * Show this works too, allowing modules to have trivial .jq files
   * and all the jq code in .c files
   */
  *contents =
    "module {\"cfunctions\":\"somod\"};"
    "def add1: _add1;"
    "def urandn: _urandomn;"
    "def add2: add1 | add1;"
    "def saddr: . + (urandn | tostring);";
  *contentssz = strlen(*contents);
  *cf = my_cfuncs;
  *ncf = sizeof(my_cfuncs) / sizeof(my_cfuncs[0]);
  return 0;
}
