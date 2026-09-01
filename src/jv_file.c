#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "jv.h"
#include "jv_unicode.h"

jv jv_load_from_fd(int fd, const char *fd_description, int raw) {
  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    int staterr = errno;
    close(fd);
    return jv_invalid_with_msg(jv_string_fmt("Cannot stat %s: %s",
                                             fd_description,
                                             strerror(staterr)));
  }
  if (S_ISDIR(sb.st_mode)) {
    close(fd);
    return jv_invalid_with_msg(jv_string_fmt("Cannot read %s: %s",
                                             fd_description,
                                             "It's a directory"));
  }

  FILE* file = fdopen(fd, "r");
  struct jv_parser* parser = NULL;
  jv data;
  if (!file) {
    close(fd);
    return jv_invalid_with_msg(jv_string_fmt("Could not open stream for %s: %s",
                                             fd_description,
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
      if (!jv_is_valid(data))
        break;
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
    return jv_invalid_with_msg(jv_string_fmt("Error reading from %s",
                                             fd_description));
  }
  return data;
}

jv jv_load_file(const char *filename, int raw) {
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    return jv_invalid_with_msg(jv_string_fmt("Could not open %s: %s",
                                             filename,
                                             strerror(errno)));
  }
  return jv_load_from_fd(fd, filename, raw);
}
