#include "opcode.h"
#define MAX_CFUNCTION_PARAM 10
typedef struct {
  json_t* value;
  int pathidx;
} stackval;

typedef void (*cfunction_ptr)(stackval* input, stackval* output);

struct cfunction {
  cfunction_ptr fptr;
  const char* name;
  opcode callop;
};
