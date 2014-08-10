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
int jq_compile(jq_state *, const char*);
int jq_compile_args(jq_state *, const char*, jv);
void jq_dump_disassembly(jq_state *, int);
void jq_start(jq_state *, jv value, int);
jv jq_next(jq_state *);
void jq_teardown(jq_state **);

void jq_set_attrs(jq_state *, jv);
jv jq_get_attrs(jq_state *);
jv jq_get_lib_origin(jq_state *);
jv jq_get_lib_dirs(jq_state *);
jv jq_get_version_dir(jq_state *);
void jq_set_attr(jq_state *, jv, jv);
jv jq_get_attr(jq_state *, jv);
#endif /* !_JQ_H_ */
