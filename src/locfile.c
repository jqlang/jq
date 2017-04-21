#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "jq.h"
#include "jv_alloc.h"
#include "locfile.h"


struct locfile* locfile_init(jq_state *jq, const char *fname, const char* data, int length) {
  struct locfile* l = jv_mem_alloc(sizeof(struct locfile));
  l->jq = jq;
  l->fname = jv_string(fname);
  l->data = jv_mem_alloc(length);
  memcpy((char*)l->data,data,length);
  l->length = length;
  l->nlines = 1;
  l->refct = 1;
  for (int i=0; i<length; i++) {
    if (data[i] == '\n') l->nlines++;
  }
  l->linemap = jv_mem_calloc(sizeof(int), (l->nlines + 1));
  l->linemap[0] = 0;
  int line = 1;
  for (int i=0; i<length; i++) {
    if (data[i] == '\n') {
      l->linemap[line] = i+1;   // at start of line, not of \n
      line++;
    }
  }
  l->linemap[l->nlines] = length+1;   // virtual last \n
  return l;
}

struct locfile* locfile_retain(struct locfile* l) {
  l->refct++;
  return l;
}
void locfile_free(struct locfile* l) {
  if (--(l->refct) == 0) {
    jv_free(l->fname);
    jv_mem_free(l->linemap);
    jv_mem_free((char*)l->data);
    jv_mem_free(l);
  }
}

int locfile_get_line(struct locfile* l, int pos) {
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
  va_list fmtargs;
  va_start(fmtargs, fmt);
  int startline;
  int offset;

  if (loc.start != -1) {
    startline = locfile_get_line(l, loc.start);
    offset = l->linemap[startline];
  }

  jv m1 = jv_string_vfmt(fmt, fmtargs);
  if (!jv_is_valid(m1)) {
    jq_report_error(l->jq, m1);
    return;
  }
  if (loc.start == -1) {
    jq_report_error(l->jq, jv_string_fmt("jq: error: %s\n<unknown location>", jv_string_value(m1)));
    jv_free(m1);
    return;
  }
  jv m2 = jv_string_fmt("%s at %s, line %d:\n%.*s%*s", jv_string_value(m1),
                        jv_string_value(l->fname), startline + 1,
                        locfile_line_length(l, startline), l->data + offset,
                        loc.start - offset, "");
  jv_free(m1);
  jq_report_error(l->jq, m2);
  return;
}
