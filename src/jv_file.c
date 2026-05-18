#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "jv.h"
#include "jv_unicode.h"

static int jv_strtofd(const char* decimal) {
  if(!('0' <= *decimal && *decimal <= '9')) return -1;
  intmax_t parsed = strtoimax(decimal, (char**)&decimal, 10);
  int fd = parsed;
  if(*decimal || fd != parsed) return -1;
  return fd;
}

jv jv_load_file(const char* source, int raw, int is_fd) {
  struct stat sb;
  int fd;
  if(is_fd) {
    fd = jv_strtofd(source);
    if(fd == -1) {
      return jv_invalid_with_msg(jv_string_fmt("Not a file descriptor: %s",
                                               source));
    }
  } else {
    fd = open(source, O_RDONLY);
    if (fd == -1) {
      return jv_invalid_with_msg(jv_string_fmt("Could not open %s: %s",
                                               source,
                                               strerror(errno)));
    }
  }
  if (fstat(fd, &sb) == -1) {
    int staterr = errno;
    if(!is_fd) close(fd);
    return jv_invalid_with_msg(jv_string_fmt("Cannot stat %s%s: %s",
                                             is_fd ? "FD " : "", source,
                                             strerror(staterr)));
  }
  if (S_ISDIR(sb.st_mode)) {
    if(!is_fd) close(fd);
    return jv_invalid_with_msg(jv_string_fmt("Cannot read %s%s: %s",
                                             is_fd ? "FD " : "", source,
                                             "It's a directory"));
  }

  FILE* file = fdopen(fd, "r");
  struct jv_parser* parser = NULL;
  jv data;
  if (!file) {
    if(!is_fd) close(fd);
    return jv_invalid_with_msg(jv_string_fmt("Could not open %s%s: %s",
                                             is_fd ? "FD " : "", source,
                                             strerror(errno)));
  }
  if (raw) {
    data = jv_string("");
  } else {
    data = jv_array();
    parser = jv_parser_new(0);
  }

  // To avoid mangling UTF-8 multi-byte sequences that cross the end of our read
  // buffer, we need to be able to read the remainder of a sequence and add that
  // before appending.
  char buf[4096 + 4];
  while (!feof(file) && !ferror(file)) {
    size_t n = fread(buf, 1, sizeof(buf) - 4, file);
    if (n == 0)
      continue;

    char *end = buf + n;
    int len = 0;
    if (jvp_utf8_backtrack(end - 1, buf, &len) && len > 0 &&
        !feof(file) && !ferror(file)) {
      n += fread(end, 1, len, file);
    }

    if (raw) {
      data = jv_string_append_buf(data, buf, n);
    } else {
      jv_parser_set_buf(parser, buf, n, !feof(file));
      jv value;
      while (jv_is_valid((value = jv_parser_next(parser))))
        data = jv_array_append(data, value);
      if (jv_invalid_has_msg(jv_copy(value))) {
        jv_free(data);
        data = value;
        break;
      }
      jv_free(value);
    }
  }
  if (!raw)
      jv_parser_free(parser);
  int badread = ferror(file);
  if (fclose(file) != 0 || badread) {
    jv_free(data);
    return jv_invalid_with_msg(jv_string_fmt("Error reading from %s%s",
                                             is_fd ? "FD " : "", source));
  }
  return data;
}
