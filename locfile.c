#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "jq.h"
#include "jv_alloc.h"
#include "locfile.h"


void locfile_init(struct locfile* l, jq_state *jq, const char* data, int length) {
  l->jq = jq;
  l->data = data;
  l->length = length;
  l->nlines = 1;
  for (int i=0; i<length; i++) {
    if (data[i] == '\n') l->nlines++;
  }
  l->linemap = jv_mem_alloc(sizeof(int) * (l->nlines + 1));
  l->linemap[0] = 0;
  int line = 1;
  for (int i=0; i<length; i++) {
    if (data[i] == '\n') {
      l->linemap[line] = i+1;   // at start of line, not of \n
      line++;
    }
  }
  l->linemap[l->nlines] = length+1;   // virtual last \n
}

void locfile_free(struct locfile* l) {
  jv_mem_free(l->linemap);
}

static int locfile_get_line(struct locfile* l, int pos) {
  assert(pos < l->length);
  int line = 1;
  while (l->linemap[line] <= pos) line++;   // == if pos at start (before, never ==, because pos never on \n)
  assert(line-1 < l->nlines);
  return line-1;
}

static int locfile_line_length(struct locfile* l, int line) {
  assert(line < l->nlines);
  return l->linemap[line+1] - l->linemap[line] -1;   // -1 to omit \n
}

void locfile_locate(struct locfile* l, location loc, const char* fmt, ...) {
  jq_err_cb cb;
  void *cb_data;
  va_list fmtargs;
  va_start(fmtargs, fmt);
  int startline;
  int offset;

  if (loc.start != -1) {
    startline = locfile_get_line(l, loc.start);
    offset = l->linemap[startline];
  }

  jq_get_error_cb(l->jq, &cb, &cb_data);

  jv m1 = jv_string_vfmt(fmt, fmtargs);
  if (!jv_is_valid(m1)) {
    jv_free(m1);
    goto enomem;
  }
  jv m2;
  if (loc.start == -1) {
    m2 = jv_string_fmt("%s\n<unknown location>", jv_string_value(m1));
    if (cb)
      cb(cb_data, m2);
    else
      fprintf(stderr, "%s", jv_string_value(m2));
    jv_free(m1);
    jv_free(m2);
    return;
  }
  m2 = jv_string_fmt("%s\n%.*s%*s", jv_string_value(m1),
                     locfile_line_length(l, startline), l->data + offset,
                     loc.start - offset, "");
  jv_free(m1);
  if (!jv_is_valid(m2)) {
    jv_free(m2);
    goto enomem;
  }
  if (cb)
    cb(cb_data, m2);
  else
    fprintf(stderr, "%s", jv_string_value(m2));
  jv_free(m2);
  return;

enomem:
  if (cb != NULL)
    cb(cb_data, jv_invalid());
  else if (errno == ENOMEM || errno == 0)
    fprintf(stderr, "Error formatting jq compilation error: %s", strerror(errno ? errno : ENOMEM));
  else
    fprintf(stderr, "Error formatting jq compilation error: %s", strerror(errno));
  return;
}
