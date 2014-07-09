#ifndef _LOCFILE_H
#define _LOCFILE_H

#include "jq.h"

typedef struct {
  int start, end;
} location;

static const location UNKNOWN_LOCATION = {-1, -1};

struct locfile {
  const char* data;
  int length;
  int* linemap;
  int nlines;
  char *error;
  jq_state *jq;
  int refct;
};

struct locfile* locfile_init(jq_state *jq, const char* data, int length);
struct locfile* locfile_retain(struct locfile* l);

void locfile_free(struct locfile* l);

void locfile_locate(struct locfile* l, location loc, const char* fmt, ...);

#endif
