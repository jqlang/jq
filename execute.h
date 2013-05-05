#ifndef EXECUTE_H
#define EXECUTE_H
#include "bytecode.h"


struct bytecode* jq_compile(const char* str);

typedef struct jq_state jq_state;
enum {JQ_DEBUG_TRACE = 1};

void jq_init(struct bytecode* bc, jv value, jq_state **, int flags);
jv jq_next(jq_state *);
void jq_teardown(jq_state **);


#endif
