#ifndef EXECUTE_H
#define EXECUTE_H
#include "bytecode.h"

typedef struct jq_state jq_state;

int jq_compile(jq_state *, const char* str);

/* args must be an array of the form [{name:"foo", value:"thing"}, {name:"bar",value:3}] */
int jq_compile_args(jq_state *, const char* str, jv args);

enum {JQ_DEBUG_TRACE = 1};

jq_state *jq_init(void);
void jq_set_nomem_handler(jq_state *, void (*)(void *), void *);
void jq_start(jq_state *, jv value, int flags);
jv jq_next(jq_state *);
void jq_teardown(jq_state **);
void jq_dump_disassembly(jq_state *, int);

#endif
