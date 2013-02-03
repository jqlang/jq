#ifndef _LOCFILE_H
#define _LOCFILE_H
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include "jv_alloc.h"
typedef struct {
  int start, end;
} location;

static const location UNKNOWN_LOCATION = {-1, -1};

struct locfile {
  const char* data;
  int length;
  int* linemap;
  int nlines;
};

static void locfile_init(struct locfile* l, const char* data, int length) {
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
      l->linemap[line] = i;
      line++;
    }
  }
  l->linemap[l->nlines] = length;
}

static void locfile_free(struct locfile* l) {
  jv_mem_free(l->linemap);
}

static int locfile_get_line(struct locfile* l, int pos) {
  assert(pos < l->length);
  int line = 0;
  while (l->linemap[line+1] < pos) line++;
  assert(line < l->nlines);
  return line;
}

static int locfile_line_length(struct locfile* l, int line) {
  assert(line < l->nlines);
  return l->linemap[line+1] - l->linemap[line];
}

static void locfile_locate(struct locfile* l, location loc, const char* fmt, ...) {
  va_list fmtargs;
  va_start(fmtargs, fmt);
  vfprintf(stderr, fmt, fmtargs);
  va_end(fmtargs);
  fprintf(stderr, "\n");
  if (loc.start == -1) {
    fprintf(stderr, "<unknown location>\n");
    return;
  }
  int startline = locfile_get_line(l, loc.start);
  int offset = l->linemap[startline];
  fprintf(stderr, "%.*s\n", locfile_line_length(l, startline), l->data + offset);
  fprintf(stderr, "%*s", loc.start - offset, "");
  for (int i = loc.start; 
       i < loc.end && i < offset + locfile_line_length(l, startline);
       i++){
    fprintf(stderr, "^");
  }
  fprintf(stderr, "\n");
}

#endif
