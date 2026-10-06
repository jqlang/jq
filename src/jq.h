#ifndef JQ_H
#define JQ_H

#include <stdio.h>
#include "jv.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
  JQ_DEBUG_TRACE = 1,
  JQ_DEBUG_TRACE_DETAIL = 2,
  JQ_DEBUG_TRACE_ALL = JQ_DEBUG_TRACE | JQ_DEBUG_TRACE_DETAIL,
};

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

void jq_halt(jq_state *, jv, jv);
int jq_halted(jq_state *);
jv jq_get_exit_code(jq_state *);
jv jq_get_error_message(jq_state *);

typedef jv (*jq_input_cb)(jq_state *, void *);
void jq_set_input_cb(jq_state *, jq_input_cb, void *);
void jq_get_input_cb(jq_state *, jq_input_cb *, void **);

/*
 * Optional providers for input_filename and input_line_number. Each callback
 * returns an owned jv: a string for filename, a number for line, or an invalid
 * value when unavailable. Other valid types produce an error. An invalid
 * filename becomes null; an invalid line preserves its error message, or
 * reports "Unknown input line number" if it has none.
 *
 * Callbacks only observe the current input; they must not consume input or
 * re-enter this jq_state. A host that caches a filename must return a jv_copy()
 * of it. The host also supplies metadata for values passed to jq_start(). No
 * metadata is automatically changed at EOF or on an input error.
 *
 * data is borrowed, independent of the value input callback's data, and must
 * remain valid until the registration is replaced, cleared, or jq_teardown()
 * is called. Registration survives jq_start(), recompilation, and changes to
 * the value input callback. libjq does not free data.
 *
 * A NULL callback uses the existing jq_util input metadata for that field.
 * Pass NULL for both callbacks and data to clear the registration. The getter
 * borrows the registered data and requires all three output pointers.
 *
 * For example, with callbacks returning jv_copy(source->filename) and
 * jv_number(source->line), register the host's current source with:
 *   jq_set_input_meta_cb(jq, filename_cb, line_cb, &source);
 * Before destroying source, unregister it with:
 *   jq_set_input_meta_cb(jq, NULL, NULL, NULL);
 */
typedef jv (*jq_input_meta_cb)(jq_state *, void *);
void jq_set_input_meta_cb(jq_state *, jq_input_meta_cb, jq_input_meta_cb, void *);
void jq_get_input_meta_cb(jq_state *, jq_input_meta_cb *, jq_input_meta_cb *, void **);

void jq_set_debug_cb(jq_state *, jq_msg_cb, void *);
void jq_get_debug_cb(jq_state *, jq_msg_cb *, void **);
void jq_set_stderr_cb(jq_state *, jq_msg_cb, void *);
void jq_get_stderr_cb(jq_state *, jq_msg_cb *, void **);

void jq_set_attrs(jq_state *, jv);
jv jq_get_attrs(jq_state *);
jv jq_get_jq_origin(jq_state *);
jv jq_get_prog_origin(jq_state *);
jv jq_get_lib_dirs(jq_state *);
void jq_set_attr(jq_state *, jv, jv);
jv jq_get_attr(jq_state *, jv);

/*
 * We use char * instead of jf for filenames here because filenames
 * should be in the process' locale's codeset, which may not be UTF-8,
 * whereas jv string values must be in UTF-8.  This way the caller
 * doesn't have to perform any codeset conversions.
 */
typedef struct jq_util_input_state jq_util_input_state;
typedef void (*jq_util_msg_cb)(void *, const char *);

jq_util_input_state *jq_util_input_init(jq_util_msg_cb, void *);
void jq_util_input_set_parser(jq_util_input_state *, jv_parser *, int);
void jq_util_input_free(jq_util_input_state **);
void jq_util_input_add_input(jq_util_input_state *, const char *);
int jq_util_input_errors(jq_util_input_state *);
jv jq_util_input_next_input(jq_util_input_state *);
jv jq_util_input_next_input_cb(jq_state *, void *);
jv jq_util_input_get_position(jq_state*);
jv jq_util_input_get_current_filename(jq_state*);
jv jq_util_input_get_current_line(jq_state*);

int jq_set_colors(const char *);

#ifdef __cplusplus
}
#endif

#endif /* !JQ_H */
