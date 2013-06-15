#ifndef EXECUTE_H
#define EXECUTE_H
#include "bytecode.h"

typedef struct jq jq;

int jq_compile(jq *, const char* str);

/* args must be an array of the form [{name:"foo", value:"thing"}, {name:"bar",value:3}] */
int jq_compile_args(jq *, const char* str, jv args);

enum {JQ_DEBUG_TRACE = 1};

jq *jq_init(void);
void jq_set_nomem_handler(jq *, void (*)(void *), void *);
void jq_start(jq *, jv value, int flags);
jv jq_next(jq *);
void jq_teardown(jq **);
void jq_dump_disassembly(jq *, int);

#endif
