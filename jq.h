#ifndef _JQ_H_
#define _JQ_H_

#include <stdio.h>
#include <jv.h>

enum {JQ_DEBUG_TRACE = 1};

typedef struct jq_state jq_state;
typedef void (*jq_err_cb)(void *, jv);

jq_state *jq_init(void);
void jq_set_error_cb(jq_state *, jq_err_cb, void *);
void jq_get_error_cb(jq_state *, jq_err_cb *, void **);
void jq_set_nomem_handler(jq_state *, void (*)(void *), void *);
jv jq_format_error(jv msg);
void jq_report_error(jq_state *, jv);
int jq_compile(jq_state *, const char* str);
int jq_compile_args(jq_state *, const char* str, jv args);
void jq_dump_disassembly(jq_state *, int);
void jq_start(jq_state *, jv value, int flags);
jv jq_next(jq_state *);
void jq_teardown(jq_state **);

void jq_set_lib_origin(jq_state *, jv origin);
jv jq_get_lib_origin(jq_state *);
void jq_set_lib_dirs(jq_state *, jv dirs);
jv jq_get_lib_dirs(jq_state *);
void jq_set_attr(jq_state *, jv attr, jv val);
jv jq_get_attr(jq_state *, jv attr);
#endif /* !_JQ_H_ */
