#ifndef _LOCFILE_H
#define _LOCFILE_H

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

void locfile_init(struct locfile* l, const char* data, int length);

void locfile_free(struct locfile* l);

void locfile_locate(struct locfile* l, location loc, const char* fmt, ...);

#endif
