#ifndef LOCFILE_H
#define LOCFILE_H

#include "jq.h"

typedef struct {
  int start, end;
} location;

static const location UNKNOWN_LOCATION = {-1, -1};

struct locfile {
  jv fname;
  const char* data;
  int length;
  int* linemap;
  int nlines;
  char *error;
  jq_state *jq;
  int refct;
};

struct locfile* locfile_init(jq_state *, const char *, const char *, int);
struct locfile* locfile_retain(struct locfile *);
int locfile_get_line(struct locfile *, int);
void locfile_free(struct locfile *);
void locfile_locate(struct locfile *, location, const char *, ...);

#endif
