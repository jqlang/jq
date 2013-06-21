#ifndef _JQ_H_
#define _JQ_H_

#include <jv.h>

enum {JQ_DEBUG_TRACE = 1};

typedef struct jq_state jq_state;
jq_state *jq_init(void);
void jq_set_nomem_handler(jq_state *, void (*)(void *), void *);
int jq_compile(jq_state *, const char* str);
int jq_compile_args(jq_state *, const char* str, jv args);
void jq_dump_disassembly(jq_state *, int);
void jq_start(jq_state *, jv value, int flags);
jv jq_next(jq_state *);
void jq_teardown(jq_state **);

#endif /* !_JQ_H_ */
