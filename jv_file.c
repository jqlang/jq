
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jv.h"
#include "jv_aux.h"
#include "jv_parse.h"

static FILE *my_fopen_re(const char *filename)
{
#if defined(O_CLOEXEC) || defined(FD_CLOEXEC)
  int fd;

#ifdef O_CLOEXEC
  fd = open(filename, O_RDONLY | O_CLOEXEC);
  if (fd == -1)
    return NULL;
#else
  /*
   * This is not MT-safe, but hopefully any thread doing
   * [v]fork()+exec*() will closefrom() or equivalent.
   */
  fd = open(filename, O_RDONLY);
  if (fd == -1)
    return NULL;
  long flags = fcntl(fd, F_GETFD) | FD_CLOEXEC;
  fcntl(fd, FD_SETFD, flags);
#endif /* O_CLOEXEC */

  f = fdopen(fd, "r");
  if (!f)
    close(fd);
  return f;
#else
  FILE *f = fopen(filename, "re");

  if (f)
      return f;

  return fopen(filename, "r");
#endif
}


jv jv_load_file(const char* filename, int raw) {
  FILE* file = my_fopen_re(filename);
  struct jv_parser parser;
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
    jv_parser_init(&parser);
  }
  while (!feof(file) && !ferror(file)) {
    char buf[4096];
    size_t n = fread(buf, 1, sizeof(buf), file);
    if (raw) {
      data = jv_string_concat(data, jv_string_sized(buf, (int)n));
    } else {
      jv_parser_set_buf(&parser, buf, strlen(buf), !feof(file));
      jv value;
      while (jv_is_valid((value = jv_parser_next(&parser))))
        data = jv_array_append(data, value);
    }
  }
  int badread = ferror(file);
  fclose(file);
  if (badread) {
    jv_free(data);
    return jv_invalid_with_msg(jv_string_fmt("Error reading from %s",
                                             filename));
  }
  return data;
}
