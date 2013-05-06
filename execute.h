#ifndef EXECUTE_H
#define EXECUTE_H
#include "bytecode.h"



struct bytecode* jq_compile(const char* str);

/* args must be an array of the form [{name:"foo", value:"thing"}, {name:"bar",value:3}] */
struct bytecode* jq_compile_args(const char* str, jv args);

typedef struct jq_state jq_state;
enum {JQ_DEBUG_TRACE = 1};

void jq_init(struct bytecode* bc, jv value, jq_state **, int flags);
jv jq_next(jq_state *);
void jq_teardown(jq_state **);


#endif
