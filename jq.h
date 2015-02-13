#ifndef _JQ_H_
#define _JQ_H_

#include <stdio.h>
#include <jv.h>

enum {JQ_DEBUG_TRACE = 1};

typedef struct jq_state jq_state;
typedef void (*jq_msg_cb)(void *, jv);

jq_state *jq_init(void);
void jq_set_error_cb(jq_state *, jq_msg_cb, void *);
void jq_get_error_cb(jq_state *, jq_msg_cb *, void **);
void jq_set_nomem_handler(jq_state *, void (*)(void *), void *);
jv jq_format_error(jv msg);
void jq_report_error(jq_state *, jv);
int jq_compile(jq_state *, const char*);
int jq_compile_args(jq_state *, const char*, jv);
void jq_dump_disassembly(jq_state *, int);
void jq_start(jq_state *, jv value, int);
jv jq_next(jq_state *);
void jq_teardown(jq_state **);

typedef jv (*jq_input_cb)(jq_state *, void *);
void jq_set_input_cb(jq_state *, jq_input_cb, void *);
void jq_get_input_cb(jq_state *, jq_input_cb *, void **);

void jq_set_debug_cb(jq_state *, jq_msg_cb, void *);
void jq_get_debug_cb(jq_state *, jq_msg_cb *, void **);

void jq_set_attrs(jq_state *, jv);
jv jq_get_attrs(jq_state *);
jv jq_get_jq_origin(jq_state *);
jv jq_get_prog_origin(jq_state *);
jv jq_get_lib_dirs(jq_state *);
void jq_set_attr(jq_state *, jv, jv);
jv jq_get_attr(jq_state *, jv);

typedef struct jq_util_input_state *jq_util_input_state;

jq_util_input_state jq_util_input_init(jq_msg_cb, void *);
void jq_util_input_set_parser(jq_util_input_state, jv_parser *, int);
void jq_util_input_free(jq_util_input_state *);
void jq_util_input_add_input(jq_util_input_state, jv);
int jq_util_input_open_errors(jq_util_input_state);
int jq_util_input_read_more(jq_util_input_state);
jv jq_util_input_next_input(jq_util_input_state);
jv jq_util_input_next_input_cb(jq_state *, void *);

#endif /* !_JQ_H_ */
