#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include "jv_alloc.h"
#include "locfile.h"


void locfile_init(struct locfile* l, const char* data, int length) {
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
