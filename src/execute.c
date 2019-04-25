#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#include "exec_stack.h"
#include "bytecode.h"

#include "jv_alloc.h"
#include "jq_parser.h"
#include "locfile.h"
#include "jv.h"
#include "jq.h"
#include "parser.h"
#include "builtin.h"
#include "util.h"
#include "linker.h"

struct jq_state {
  void (*nomem_handler)(void *);
  void *nomem_handler_data;
  struct bytecode* bc;

  jq_msg_cb err_cb;
  void *err_cb_data;
  jv error;

  struct stack stk;
  stack_ptr curr_frame;
  stack_ptr stk_top;
  stack_ptr fork_top;

  jv path;
  jv value_at_path;
  int subexp_nest;
  int debug_trace_enabled;
  int initial_execution;
  unsigned next_label;

  int halted;
  jv exit_code;
  jv error_message;

  jv attrs;
  jq_input_cb input_cb;
  void *input_cb_data;
  jq_msg_cb debug_cb;
  void *debug_cb_data;
};

struct closure {
  struct bytecode* bc;  // jq bytecode
  stack_ptr env;        // jq stack address of closed frame
};

// locals for any function called: either a closure or a local variable
union frame_entry {
  struct closure closure;
  jv localvar;
};

// jq function call frame
struct frame {
  struct bytecode* bc;      // jq bytecode for callee
  stack_ptr env;            // jq stack address of frame to return to
  stack_ptr retdata;        // jq stack address to unwind to on RET
  uint16_t* retaddr;        // jq bytecode return address
  union frame_entry entries[]; // nclosures + nlocals
};

static int frame_size(struct bytecode* bc) {
  return sizeof(struct frame) + sizeof(union frame_entry) * (bc->nclosures + bc->nlocals);
}

static struct frame* frame_current(struct jq_state* jq) {
  struct frame* fp = stack_block(&jq->stk, jq->curr_frame);

  stack_ptr next = *stack_block_next(&jq->stk, jq->curr_frame);
  if (next) {
    struct frame* fpnext = stack_block(&jq->stk, next);
    struct bytecode* bc = fpnext->bc;
    assert(fp->retaddr >= bc->code && fp->retaddr < bc->code + bc->codelen);
  } else {
    assert(fp->retaddr == 0);
  }
  return fp;
}

static stack_ptr frame_get_level(struct jq_state* jq, int level) {
  stack_ptr fr = jq->curr_frame;
  for (int i=0; i<level; i++) {
    struct frame* fp = stack_block(&jq->stk, fr);
    fr = fp->env;
  }
  return fr;
}

static jv* frame_local_var(struct jq_state* jq, int var, int level) {
  struct frame* fr = stack_block(&jq->stk, frame_get_level(jq, level));
  assert(var >= 0);
  assert(var < fr->bc->nlocals);
  return &fr->entries[fr->bc->nclosures + var].localvar;
}

static struct closure make_closure(struct jq_state* jq, uint16_t* pc) {
  uint16_t level = *pc++;
  uint16_t idx = *pc++;
  stack_ptr fridx = frame_get_level(jq, level);
  struct frame* fr = stack_block(&jq->stk, fridx);
  if (idx & ARG_NEWCLOSURE) {
    // A new closure closing the frame identified by level, and with
    // the bytecode body of the idx'th subfunction of that frame
    int subfn_idx = idx & ~ARG_NEWCLOSURE;
    assert(subfn_idx < fr->bc->nsubfunctions);
    struct closure cl = {fr->bc->subfunctions[subfn_idx],
                         fridx};
    return cl;
  } else {
    // A reference to a closure from the frame identified by level; copy
    // it as-is
    int closure = idx;
    assert(closure >= 0);
    assert(closure < fr->bc->nclosures);
    return fr->entries[closure].closure;
  }
}

static struct frame* frame_push(struct jq_state* jq, struct closure callee,
                                uint16_t* argdef, int nargs) {
  stack_ptr new_frame_idx = stack_push_block(&jq->stk, jq->curr_frame, frame_size(callee.bc));
  struct frame* new_frame = stack_block(&jq->stk, new_frame_idx);
  new_frame->bc = callee.bc;
  new_frame->env = callee.env;
  assert(nargs == new_frame->bc->nclosures);
  union frame_entry* entries = new_frame->entries;
  for (int i=0; i<nargs; i++) {
    entries->closure = make_closure(jq, argdef + i * 2);
    entries++;
  }
  for (int i=0; i<callee.bc->nlocals; i++) {
    entries->localvar = jv_invalid();
    entries++;
  }
  jq->curr_frame = new_frame_idx;
  return new_frame;
}

static void frame_pop(struct jq_state* jq) {
  assert(jq->curr_frame);
  struct frame* fp = frame_current(jq);
  if (stack_pop_will_free(&jq->stk, jq->curr_frame)) {
    int nlocals = fp->bc->nlocals;
    for (int i=0; i<nlocals; i++) {
      jv_free(*frame_local_var(jq, i, 0));
    }
  }
  jq->curr_frame = stack_pop_block(&jq->stk, jq->curr_frame, frame_size(fp->bc));
}

void stack_push(jq_state *jq, jv val) {
  assert(jv_is_valid(val));
  jq->stk_top = stack_push_block(&jq->stk, jq->stk_top, sizeof(jv));
  jv* sval = stack_block(&jq->stk, jq->stk_top);
  *sval = val;
}

jv stack_pop(jq_state *jq) {
  jv* sval = stack_block(&jq->stk, jq->stk_top);
  jv val = *sval;
  if (!stack_pop_will_free(&jq->stk, jq->stk_top)) {
    val = jv_copy(val);
  }
  jq->stk_top = stack_pop_block(&jq->stk, jq->stk_top, sizeof(jv));
  assert(jv_is_valid(val));
  return val;
}

// Like stack_pop(), but assert !stack_pop_will_free() and replace with
// jv_null() on the stack.
jv stack_popn(jq_state *jq) {
  jv* sval = stack_block(&jq->stk, jq->stk_top);
  jv val = *sval;
  if (!stack_pop_will_free(&jq->stk, jq->stk_top)) {
    *sval = jv_null();
  }
  jq->stk_top = stack_pop_block(&jq->stk, jq->stk_top, sizeof(jv));
  assert(jv_is_valid(val));
  return val;
}


struct forkpoint {
  stack_ptr saved_data_stack;
  stack_ptr saved_curr_frame;
  int path_len, subexp_nest;
  jv value_at_path;
  uint16_t* return_address;
};

struct stack_pos {
  stack_ptr saved_data_stack, saved_curr_frame;
};

struct stack_pos stack_get_pos(jq_state* jq) {
  struct stack_pos sp = {jq->stk_top, jq->curr_frame};
  return sp;
}

void stack_save(jq_state *jq, uint16_t* retaddr, struct stack_pos sp){
  jq->fork_top = stack_push_block(&jq->stk, jq->fork_top, sizeof(struct forkpoint));
  struct forkpoint* fork = stack_block(&jq->stk, jq->fork_top);
  fork->saved_data_stack = jq->stk_top;
  fork->saved_curr_frame = jq->curr_frame;
  fork->path_len =
    jv_get_kind(jq->path) == JV_KIND_ARRAY ? jv_array_length(jv_copy(jq->path)) : 0;
  fork->value_at_path = jv_copy(jq->value_at_path);
  fork->subexp_nest = jq->subexp_nest;
  fork->return_address = retaddr;
  jq->stk_top = sp.saved_data_stack;
  jq->curr_frame = sp.saved_curr_frame;
}

static int path_intact(jq_state *jq, jv curr) {
  if (jq->subexp_nest == 0 && jv_get_kind(jq->path) == JV_KIND_ARRAY) {
    return jv_identical(curr, jv_copy(jq->value_at_path));
  } else {
    jv_free(curr);
    return 1;
  }
}

static void path_append(jq_state* jq, jv component, jv value_at_path) {
  if (jq->subexp_nest == 0 && jv_get_kind(jq->path) == JV_KIND_ARRAY) {
    int n1 = jv_array_length(jv_copy(jq->path));
    jq->path = jv_array_append(jq->path, component);
    int n2 = jv_array_length(jv_copy(jq->path));
    assert(n2 == n1 + 1);
    jv_free(jq->value_at_path);
    jq->value_at_path = value_at_path;
  } else {
    jv_free(component);
    jv_free(value_at_path);
  }
}

/* For f_getpath() */
jv
_jq_path_append(jq_state *jq, jv v, jv p, jv value_at_path) {
  if (jq->subexp_nest != 0 ||
      jv_get_kind(jq->path) != JV_KIND_ARRAY ||
      !jv_is_valid(value_at_path)) {
    jv_free(v);
    jv_free(p);
    return value_at_path;
  }
  if (!jv_identical(v, jv_copy(jq->value_at_path))) {
    jv_free(p);
    return value_at_path;
  }
  if (jv_get_kind(p) == JV_KIND_ARRAY)
    jq->path = jv_array_concat(jq->path, p);
  else
    jq->path = jv_array_append(jq->path, p);
  jv_free(jq->value_at_path);
  return jv_copy(jq->value_at_path = value_at_path);
}

uint16_t* stack_restore(jq_state *jq){
  while (!stack_pop_will_free(&jq->stk, jq->fork_top)) {
    if (stack_pop_will_free(&jq->stk, jq->stk_top)) {
      jv_free(stack_pop(jq));
    } else if (stack_pop_will_free(&jq->stk, jq->curr_frame)) {
      frame_pop(jq);
    } else {
      assert(0);
    }
  }

  if (jq->fork_top == 0) {
    return 0;
  }

  struct forkpoint* fork = stack_block(&jq->stk, jq->fork_top);
  uint16_t* retaddr = fork->return_address;
  jq->stk_top = fork->saved_data_stack;
  jq->curr_frame = fork->saved_curr_frame;
  int path_len = fork->path_len;
  if (jv_get_kind(jq->path) == JV_KIND_ARRAY) {
    assert(path_len >= 0);
    jq->path = jv_array_slice(jq->path, 0, path_len);
  } else {
    fork->path_len = 0;
  }
  jv_free(jq->value_at_path);
  jq->value_at_path = fork->value_at_path;
  jq->subexp_nest = fork->subexp_nest;
  jq->fork_top = stack_pop_block(&jq->stk, jq->fork_top, sizeof(struct forkpoint));
  return retaddr;
}

static void jq_reset(jq_state *jq) {
  while (stack_restore(jq)) {}

  assert(jq->stk_top == 0);
  assert(jq->fork_top == 0);
  assert(jq->curr_frame == 0);
  stack_reset(&jq->stk);
  jv_free(jq->error);
  jq->error = jv_null();

  jq->halted = 0;
  jv_free(jq->exit_code);
  jv_free(jq->error_message);
  if (jv_get_kind(jq->path) != JV_KIND_INVALID)
    jv_free(jq->path);
  jq->path = jv_null();
  jv_free(jq->value_at_path);
  jq->value_at_path = jv_null();
  jq->subexp_nest = 0;
}

void jq_report_error(jq_state *jq, jv value) {
  assert(jq->err_cb);
  // callback must jv_free() its jv argument
  jq->err_cb(jq->err_cb_data, value);
}

static void set_error(jq_state *jq, jv value) {
  // Record so try/catch can find it.
  jv_free(jq->error);
  jq->error = value;
}

#define ON_BACKTRACK(op) ((op)+NUM_OPCODES)

jv jq_next(jq_state *jq) {
  jv cfunc_input[MAX_CFUNCTION_ARGS];

  jv_nomem_handler(jq->nomem_handler, jq->nomem_handler_data);

  uint16_t* pc = stack_restore(jq);
  assert(pc);

  int raising;
  int backtracking = !jq->initial_execution;
  jq->initial_execution = 0;
  assert(jv_get_kind(jq->error) == JV_KIND_NULL);
  while (1) {
    if (jq->halted) {
      if (jq->debug_trace_enabled)
        printf("\t<halted>\n");
      return jv_invalid();
    }
    uint16_t opcode = *pc;
    raising = 0;

    if (jq->debug_trace_enabled) {
      dump_operation(frame_current(jq)->bc, pc);
      printf("\t");
      const struct opcode_description* opdesc = opcode_describe(opcode);
      stack_ptr param = 0;
      if (!backtracking) {
        int stack_in = opdesc->stack_in;
        if (stack_in == -1) stack_in = pc[1];
        param = jq->stk_top;
        for (int i=0; i<stack_in; i++) {
          if (i != 0) {
            printf(" | ");
            param = *stack_block_next(&jq->stk, param);
          }
          if (!param) break;
          jv_dump(jv_copy(*(jv*)stack_block(&jq->stk, param)), JV_PRINT_REFCOUNT);
          //printf("<%d>", jv_get_refcnt(param->val));
          //printf(" -- ");
          //jv_dump(jv_copy(jq->path), 0);
        }
        if (jq->debug_trace_enabled & JQ_DEBUG_TRACE_DETAIL) {
          while ((param = *stack_block_next(&jq->stk, param))) {
            printf(" || ");
            jv_dump(jv_copy(*(jv*)stack_block(&jq->stk, param)), JV_PRINT_REFCOUNT);
          }
        }
      } else {
        printf("\t<backtracking>");
      }

      printf("\n");
    }

    if (backtracking) {
      opcode = ON_BACKTRACK(opcode);
      backtracking = 0;
      raising = !jv_is_valid(jq->error);
    }
    pc++;

    switch (opcode) {
    default: assert(0 && "invalid instruction");

    case TOP: break;

    case LOADK: {
      jv v = jv_array_get(jv_copy(frame_current(jq)->bc->constants), *pc++);
      assert(jv_is_valid(v));
      jv_free(stack_pop(jq));
      stack_push(jq, v);
      break;
    }

    case GENLABEL: {
      stack_push(jq, JV_OBJECT(jv_string("__jq"), jv_number(jq->next_label++)));
      break;
    }

    case DUP: {
      jv v = stack_pop(jq);
      stack_push(jq, jv_copy(v));
      stack_push(jq, v);
      break;
    }

    case DUPN: {
      jv v = stack_popn(jq);
      stack_push(jq, jv_copy(v));
      stack_push(jq, v);
      break;
    }

    case DUP2: {
      jv keep = stack_pop(jq);
      jv v = stack_pop(jq);
      stack_push(jq, jv_copy(v));
      stack_push(jq, keep);
      stack_push(jq, v);
      break;
    }

    case SUBEXP_BEGIN: {
      jv v = stack_pop(jq);
      stack_push(jq, jv_copy(v));
      stack_push(jq, v);
      jq->subexp_nest++;
      break;
    }

    case SUBEXP_END: {
      assert(jq->subexp_nest > 0);
      jq->subexp_nest--;
      jv a = stack_pop(jq);
      jv b = stack_pop(jq);
      stack_push(jq, a);
      stack_push(jq, b);
      break;
    }

    case PUSHK_UNDER: {
      jv v = jv_array_get(jv_copy(frame_current(jq)->bc->constants), *pc++);
      assert(jv_is_valid(v));
      jv v2 = stack_pop(jq);
      stack_push(jq, v);
      stack_push(jq, v2);
      break;
    }

    case POP: {
      jv_free(stack_pop(jq));
      break;
    }

    case APPEND: {
      jv v = stack_pop(jq);
      uint16_t level = *pc++;
      uint16_t vidx = *pc++;
      jv* var = frame_local_var(jq, vidx, level);
      assert(jv_get_kind(*var) == JV_KIND_ARRAY);
      *var = jv_array_append(*var, v);
      break;
    }

    case INSERT: {
      jv stktop = stack_pop(jq);
      jv v = stack_pop(jq);
      jv k = stack_pop(jq);
      jv objv = stack_pop(jq);
      assert(jv_get_kind(objv) == JV_KIND_OBJECT);
      if (jv_get_kind(k) == JV_KIND_STRING) {
        stack_push(jq, jv_object_set(objv, k, v));
        stack_push(jq, stktop);
      } else {
        char errbuf[15];
        set_error(jq, jv_invalid_with_msg(jv_string_fmt("Cannot use %s (%s) as object key",
                                                        jv_kind_name(jv_get_kind(k)),
                                                        jv_dump_string_trunc(jv_copy(k), errbuf, sizeof(errbuf)))));
        jv_free(stktop);
        jv_free(v);
        jv_free(k);
        jv_free(objv);
        goto do_backtrack;
      }
      break;
    }

    case ON_BACKTRACK(RANGE):
    case RANGE: {
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      jv* var = frame_local_var(jq, v, level);
      jv max = stack_pop(jq);
      if (raising) goto do_backtrack;
      if (jv_get_kind(*var) != JV_KIND_NUMBER ||
          jv_get_kind(max) != JV_KIND_NUMBER) {
        set_error(jq, jv_invalid_with_msg(jv_string_fmt("Range bounds must be numeric")));
        jv_free(max);
        goto do_backtrack;
      } else if (jv_number_value(jv_copy(*var)) >= jv_number_value(jv_copy(max))) {
        /* finished iterating */
        goto do_backtrack;
      } else {
        jv curr = jv_copy(*var);
        *var = jv_number(jv_number_value(*var) + 1);

        struct stack_pos spos = stack_get_pos(jq);
        stack_push(jq, jv_copy(max));
        stack_save(jq, pc - 3, spos);

        stack_push(jq, curr);
      }
      break;
    }

      // FIXME: loadv/storev may do too much copying/freeing
    case LOADV: {
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      jv* var = frame_local_var(jq, v, level);
      if (jq->debug_trace_enabled) {
        printf("V%d = ", v);
        jv_dump(jv_copy(*var), 0);
        printf(" (%d)\n", jv_get_refcnt(*var));
      }
      jv_free(stack_pop(jq));
      stack_push(jq, jv_copy(*var));
      break;
    }

      // Does a load but replaces the variable with null
    case LOADVN: {
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      jv* var = frame_local_var(jq, v, level);
      if (jq->debug_trace_enabled) {
        printf("V%d = ", v);
        jv_dump(jv_copy(*var), 0);
        printf(" (%d)\n", jv_get_refcnt(*var));
      }
      jv_free(stack_popn(jq));
      stack_push(jq, *var);
      *var = jv_null();
      break;
    }

    case STOREVN:
        stack_save(jq, pc - 1, stack_get_pos(jq));
    case STOREV: {
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      jv* var = frame_local_var(jq, v, level);
      jv val = stack_pop(jq);
      if (jq->debug_trace_enabled) {
        printf("V%d = ", v);
        jv_dump(jv_copy(val), 0);
        printf(" (%d)\n", jv_get_refcnt(val));
      }
      jv_free(*var);
      *var = val;
      break;
    }

    case ON_BACKTRACK(STOREVN): {
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      jv* var = frame_local_var(jq, v, level);
      jv_free(*var);
      *var = jv_null();
      goto do_backtrack;
      break;
    }

    case STORE_GLOBAL: {
      // Get the constant
      jv val = jv_array_get(jv_copy(frame_current(jq)->bc->constants), *pc++);
      assert(jv_is_valid(val));

      // Store the var
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      jv* var = frame_local_var(jq, v, level);
      if (jq->debug_trace_enabled) {
        printf("V%d = ", v);
        jv_dump(jv_copy(val), 0);
        printf(" (%d)\n", jv_get_refcnt(val));
      }
      jv_free(*var);
      *var = val;
      break;
    }

    case PATH_BEGIN: {
      jv v = stack_pop(jq);
      stack_push(jq, jq->path);

      stack_save(jq, pc - 1, stack_get_pos(jq));

      stack_push(jq, jv_number(jq->subexp_nest));
      stack_push(jq, jq->value_at_path);
      stack_push(jq, jv_copy(v));

      jq->path = jv_array();
      jq->value_at_path = v; // next INDEX operation must index into v
      jq->subexp_nest = 0;
      break;
    }

    case PATH_END: {
      jv v = stack_pop(jq);
      // detect invalid path expression like path(.a | reverse)
      if (!path_intact(jq, jv_copy(v))) {
        char errbuf[30];
        jv msg = jv_string_fmt(
            "Invalid path expression with result %s",
            jv_dump_string_trunc(v, errbuf, sizeof(errbuf)));
        set_error(jq, jv_invalid_with_msg(msg));
        goto do_backtrack;
      }
      jv_free(v); // discard value, only keep path

      jv old_value_at_path = stack_pop(jq);
      int old_subexp_nest = (int)jv_number_value(stack_pop(jq));

      jv path = jq->path;
      jq->path = stack_pop(jq);

      struct stack_pos spos = stack_get_pos(jq);
      stack_push(jq, jv_copy(path));
      stack_save(jq, pc - 1, spos);

      stack_push(jq, path);
      jq->subexp_nest = old_subexp_nest;
      jv_free(jq->value_at_path);
      jq->value_at_path = old_value_at_path;
      break;
    }

    case ON_BACKTRACK(PATH_BEGIN):
    case ON_BACKTRACK(PATH_END): {
      jv_free(jq->path);
      jq->path = stack_pop(jq);
      goto do_backtrack;
    }

    case INDEX:
    case INDEX_OPT: {
      jv t = stack_pop(jq);
      jv k = stack_pop(jq);
      // detect invalid path expression like path(reverse | .a)
      if (!path_intact(jq, jv_copy(t))) {
        char keybuf[15];
        char objbuf[30];
        jv msg = jv_string_fmt(
            "Invalid path expression near attempt to access element %s of %s",
            jv_dump_string_trunc(k, keybuf, sizeof(keybuf)),
            jv_dump_string_trunc(t, objbuf, sizeof(objbuf)));
        set_error(jq, jv_invalid_with_msg(msg));
        goto do_backtrack;
      }
      jv v = jv_get(t, jv_copy(k));
      if (jv_is_valid(v)) {
        path_append(jq, k, jv_copy(v));
        stack_push(jq, v);
      } else {
        jv_free(k);
        if (opcode == INDEX)
          set_error(jq, v);
        else
          jv_free(v);
        goto do_backtrack;
      }
      break;
    }


    case JUMP: {
      uint16_t offset = *pc++;
      pc += offset;
      break;
    }

    case JUMP_F: {
      uint16_t offset = *pc++;
      jv t = stack_pop(jq);
      jv_kind kind = jv_get_kind(t);
      if (kind == JV_KIND_FALSE || kind == JV_KIND_NULL) {
        pc += offset;
      }
      stack_push(jq, t); // FIXME do this better
      break;
    }

    case EACH:
    case EACH_OPT: {
      jv container = stack_pop(jq);
      // detect invalid path expression like path(reverse | .[])
      if (!path_intact(jq, jv_copy(container))) {
        char errbuf[30];
        jv msg = jv_string_fmt(
            "Invalid path expression near attempt to iterate through %s",
            jv_dump_string_trunc(container, errbuf, sizeof(errbuf)));
        set_error(jq, jv_invalid_with_msg(msg));
        goto do_backtrack;
      }
      stack_push(jq, container);
      stack_push(jq, jv_number(-1));
      // fallthrough
    }
    case ON_BACKTRACK(EACH):
    case ON_BACKTRACK(EACH_OPT): {
      int idx = jv_number_value(stack_pop(jq));
      jv container = stack_pop(jq);

      int keep_going, is_last = 0;
      jv key, value;
      if (jv_get_kind(container) == JV_KIND_ARRAY) {
        if (opcode == EACH || opcode == EACH_OPT) idx = 0;
        else idx = idx + 1;
        int len = jv_array_length(jv_copy(container));
        keep_going = idx < len;
        is_last = idx == len - 1;
        if (keep_going) {
          key = jv_number(idx);
          value = jv_array_get(jv_copy(container), idx);
        }
      } else if (jv_get_kind(container) == JV_KIND_OBJECT) {
        if (opcode == EACH || opcode == EACH_OPT) idx = jv_object_iter(container);
        else idx = jv_object_iter_next(container, idx);
        keep_going = jv_object_iter_valid(container, idx);
        if (keep_going) {
          key = jv_object_iter_key(container, idx);
          value = jv_object_iter_value(container, idx);
        }
      } else {
        assert(opcode == EACH || opcode == EACH_OPT);
        if (opcode == EACH) {
          char errbuf[15];
          set_error(jq,
                    jv_invalid_with_msg(jv_string_fmt("Cannot iterate over %s (%s)",
                                                      jv_kind_name(jv_get_kind(container)),
                                                      jv_dump_string_trunc(jv_copy(container), errbuf, sizeof(errbuf)))));
        }
        keep_going = 0;
      }

      if (!keep_going || raising) {
        if (keep_going)
          jv_free(value);
        jv_free(container);
        goto do_backtrack;
      } else if (is_last) {
        // we don't need to make a backtrack point
        jv_free(container);
        path_append(jq, key, jv_copy(value));
        stack_push(jq, value);
      } else {
        struct stack_pos spos = stack_get_pos(jq);
        stack_push(jq, container);
        stack_push(jq, jv_number(idx));
        stack_save(jq, pc - 1, spos);
        path_append(jq, key, jv_copy(value));
        stack_push(jq, value);
      }
      break;
    }

    do_backtrack:
    case BACKTRACK: {
      pc = stack_restore(jq);
      if (!pc) {
        if (!jv_is_valid(jq->error)) {
          jv error = jq->error;
          jq->error = jv_null();
          return error;
        }
        return jv_invalid();
      }
      backtracking = 1;
      break;
    }

    case FORK_OPT:
    case DESTRUCTURE_ALT:
    case FORK: {
      stack_save(jq, pc - 1, stack_get_pos(jq));
      pc++; // skip offset this time
      break;
    }

    case ON_BACKTRACK(FORK_OPT):
    case ON_BACKTRACK(DESTRUCTURE_ALT): {
      if (jv_is_valid(jq->error)) {
        // `try EXP ...` backtracked here (no value, `empty`), so we backtrack more
        jv_free(stack_pop(jq));
        goto do_backtrack;
      }
      // `try EXP ...` exception caught in EXP
      // DESTRUCTURE_ALT doesn't want the error message on the stack,
      // as we would just want to throw it away anyway.
      if (opcode != ON_BACKTRACK(DESTRUCTURE_ALT)) {
        jv_free(stack_pop(jq)); // free the input
        stack_push(jq, jv_invalid_get_msg(jq->error));  // push the error's message
      } else {
        jv_free(jq->error);
      }
      jq->error = jv_null();
      uint16_t offset = *pc++;
      pc += offset;
      break;
    }
    case ON_BACKTRACK(FORK): {
      if (raising) goto do_backtrack;
      uint16_t offset = *pc++;
      pc += offset;
      break;
    }

    case CALL_BUILTIN: {
      int nargs = *pc++;
      jv top = stack_pop(jq);
      jv* in = cfunc_input;
      in[0] = top;
      for (int i = 1; i < nargs; i++) {
        in[i] = stack_pop(jq);
      }
      struct cfunction* function = &frame_current(jq)->bc->globals->cfunctions[*pc++];
      typedef jv (*func_1)(jq_state*,jv);
      typedef jv (*func_2)(jq_state*,jv,jv);
      typedef jv (*func_3)(jq_state*,jv,jv,jv);
      typedef jv (*func_4)(jq_state*,jv,jv,jv,jv);
      typedef jv (*func_5)(jq_state*,jv,jv,jv,jv,jv);
      switch (function->nargs) {
      case 1: top = ((func_1)function->fptr)(jq, in[0]); break;
      case 2: top = ((func_2)function->fptr)(jq, in[0], in[1]); break;
      case 3: top = ((func_3)function->fptr)(jq, in[0], in[1], in[2]); break;
      case 4: top = ((func_4)function->fptr)(jq, in[0], in[1], in[2], in[3]); break;
      case 5: top = ((func_5)function->fptr)(jq, in[0], in[1], in[2], in[3], in[4]); break;
      // FIXME: a) up to 7 arguments (input + 6), b) should assert
      // because the compiler should not generate this error.
      default: return jv_invalid_with_msg(jv_string("Function takes too many arguments"));
      }

      if (jv_is_valid(top)) {
        stack_push(jq, top);
      } else if (jv_invalid_has_msg(jv_copy(top))) {
        set_error(jq, top);
        goto do_backtrack;
      } else {
        // C-coded function returns invalid w/o msg? -> backtrack, as if
        // it had returned `empty`
        goto do_backtrack;
      }
      break;
    }

    case TAIL_CALL_JQ:
    case CALL_JQ: {
      /*
       * Bytecode layout here:
       *
       *  CALL_JQ
       *  <nclosures>                       (i.e., number of call arguments)
       *  <callee closure>                  (what we're calling)
       *  <nclosures' worth of closures>    (frame reference + code pointer)
       *
       *  <next instruction (to return to)>
       *
       * Each closure consists of two uint16_t values: a "level"
       * identifying the frame to be closed over, and an index.
       *
       * The level is a relative number of call frames reachable from
       * the currently one; 0 -> current frame, 1 -> previous frame, and
       * so on.
       *
       * The index is either an index of the closed frame's subfunctions
       * or of the closed frame's parameter closures.  If the latter,
       * that closure will be passed, else the closed frame's pointer
       * and the subfunction's code will form the closure to be passed.
       *
       * See make_closure() for more information.
       */
      jv input = stack_pop(jq);
      uint16_t nclosures = *pc++;
      uint16_t* retaddr = pc + 2 + nclosures*2;
      stack_ptr retdata = jq->stk_top;
      struct frame* new_frame;
      struct closure cl = make_closure(jq, pc);
      if (opcode == TAIL_CALL_JQ) {
        retaddr = frame_current(jq)->retaddr;
        retdata = frame_current(jq)->retdata;
        frame_pop(jq);
      }
      new_frame = frame_push(jq, cl, pc + 2, nclosures);
      new_frame->retdata = retdata;
      new_frame->retaddr = retaddr;
      pc = new_frame->bc->code;
      stack_push(jq, input);
      break;
    }

    case RET: {
      jv value = stack_pop(jq);
      assert(jq->stk_top == frame_current(jq)->retdata);
      uint16_t* retaddr = frame_current(jq)->retaddr;
      if (retaddr) {
        // function return
        pc = retaddr;
        frame_pop(jq);
      } else {
        // top-level return, yielding value
        struct stack_pos spos = stack_get_pos(jq);
        stack_push(jq, jv_null());
        stack_save(jq, pc - 1, spos);
        return value;
      }
      stack_push(jq, value);
      break;
    }
    case ON_BACKTRACK(RET): {
      // resumed after top-level return
      goto do_backtrack;
    }
    }
  }
}

jv jq_format_error(jv msg) {
  if (jv_get_kind(msg) == JV_KIND_NULL ||
      (jv_get_kind(msg) == JV_KIND_INVALID && !jv_invalid_has_msg(jv_copy(msg)))) {
    jv_free(msg);
    fprintf(stderr, "jq: error: out of memory\n");
    return jv_null();
  }

  if (jv_get_kind(msg) == JV_KIND_STRING)
    return msg;                         // expected to already be formatted

  if (jv_get_kind(msg) == JV_KIND_INVALID)
    msg = jv_invalid_get_msg(msg);

  if (jv_get_kind(msg) == JV_KIND_NULL)
    return jq_format_error(msg);        // ENOMEM

  // Invalid with msg; prefix with "jq: error: "

  if (jv_get_kind(msg) != JV_KIND_INVALID) {
    if (jv_get_kind(msg) == JV_KIND_STRING)
      return jv_string_fmt("jq: error: %s", jv_string_value(msg));

    msg = jv_dump_string(msg, JV_PRINT_INVALID);
    if (jv_get_kind(msg) == JV_KIND_STRING)
      return jv_string_fmt("jq: error: %s", jv_string_value(msg));
    return jq_format_error(jv_null());  // ENOMEM
  }

  // An invalid inside an invalid!
  return jq_format_error(jv_invalid_get_msg(msg));
}

// XXX Refactor into a utility function that returns a jv and one that
// uses it and then prints that jv's string as the complete error
// message.
static void default_err_cb(void *data, jv msg) {
  msg = jq_format_error(msg);
  fprintf((FILE *)data, "%s\n", jv_string_value(msg));
  jv_free(msg);
}

jq_state *jq_init(void) {
  jq_state *jq;
  jq = jv_mem_alloc_unguarded(sizeof(*jq));
  if (jq == NULL)
    return NULL;

  jq->bc = 0;
  jq->next_label = 0;

  stack_init(&jq->stk);
  jq->stk_top = 0;
  jq->fork_top = 0;
  jq->curr_frame = 0;
  jq->error = jv_null();

  jq->halted = 0;
  jq->exit_code = jv_invalid();
  jq->error_message = jv_invalid();

  jq->err_cb = default_err_cb;
  jq->err_cb_data = stderr;

  jq->attrs = jv_object();
  jq->path = jv_null();
  jq->value_at_path = jv_null();
  return jq;
}

void jq_set_error_cb(jq_state *jq, jq_msg_cb cb, void *data) {
  if (cb == NULL) {
    jq->err_cb = default_err_cb;
    jq->err_cb_data = stderr;
  } else {
    jq->err_cb = cb;
    jq->err_cb_data = data;
  }
}

void jq_get_error_cb(jq_state *jq, jq_msg_cb *cb, void **data) {
  *cb = jq->err_cb;
  *data = jq->err_cb_data;
}

void jq_set_nomem_handler(jq_state *jq, void (*nomem_handler)(void *), void *data) {
  jv_nomem_handler(nomem_handler, data);
  jq->nomem_handler = nomem_handler;
  jq->nomem_handler_data = data;
}


void jq_start(jq_state *jq, jv input, int flags) {
  jv_nomem_handler(jq->nomem_handler, jq->nomem_handler_data);
  jq_reset(jq);

  struct closure top = {jq->bc, -1};
  struct frame* top_frame = frame_push(jq, top, 0, 0);
  top_frame->retdata = 0;
  top_frame->retaddr = 0;

  stack_push(jq, input);
  stack_save(jq, jq->bc->code, stack_get_pos(jq));
  jq->debug_trace_enabled = flags & JQ_DEBUG_TRACE_ALL;
  jq->initial_execution = 1;
}

void jq_teardown(jq_state **jq) {
  jq_state *old_jq = *jq;
  if (old_jq == NULL)
    return;
  *jq = NULL;

  jq_reset(old_jq);
  bytecode_free(old_jq->bc);
  old_jq->bc = 0;
  jv_free(old_jq->attrs);

  jv_mem_free(old_jq);
}

static int ret_follows(uint16_t *pc) {
  if (*pc == RET)
    return 1;
  if (*pc++ != JUMP)
    return 0;
  return ret_follows(pc + *pc + 1); // FIXME, might be ironic
}

/*
 * Look for tail calls that can be optimized: tail calls with no
 * references left to the current frame.
 *
 * We're staring at this bytecode layout:
 *
 *   CALL_JQ
 *   <nclosures>
 *   <callee closure>       (2 units)
 *   <nclosures closures>   (2 units each)
 *   <next instruction>
 *
 * A closure is:
 *
 *   <level>    (a relative frame count chased via the current frame's env)
 *   <index>    (an index of a subfunction or closure in that frame)
 *
 * We're looking for:
 *
 * a) the next instruction is a RET or a chain of unconditional JUMPs
 * that ends in a RET, and
 *
 * b) none of the closures -callee included- have level == 0.
 */
static uint16_t tail_call_analyze(uint16_t *pc) {
  assert(*pc == CALL_JQ);
  pc++;
  // + 1 for the callee closure
  for (uint16_t nclosures = *pc++ + 1; nclosures > 0; pc++, nclosures--) {
    if (*pc++ == 0)
      return CALL_JQ;
  }
  if (ret_follows(pc))
    return TAIL_CALL_JQ;
  return CALL_JQ;
}

static struct bytecode *optimize_code(struct bytecode *bc) {
  uint16_t *pc = bc->code;
  // FIXME: Don't mutate bc->code...
  while (pc < bc->code + bc->codelen) {
    switch (*pc) {
    case CALL_JQ:
      *pc = tail_call_analyze(pc);
      break;

    // Other bytecode optimizations here.  A peephole optimizer would
    // fit right in.
    default: break;
    }
    pc += bytecode_operation_length(pc);
  }
  return bc;
}

static struct bytecode *optimize(struct bytecode *bc) {
  for (int i=0; i<bc->nsubfunctions; i++) {
    bc->subfunctions[i] = optimize(bc->subfunctions[i]);
  }
  return optimize_code(bc);
}

static jv
args2obj(jv args)
{
  if (jv_get_kind(args) == JV_KIND_OBJECT)
    return args;
  assert(jv_get_kind(args) == JV_KIND_ARRAY);
  jv r = jv_object();
  jv kk = jv_string("name");
  jv vk = jv_string("value");
  jv_array_foreach(args, i, v)
    r = jv_object_set(r, jv_object_get(jv_copy(v), kk), jv_object_get(v, vk));
  jv_free(args);
  jv_free(kk);
  jv_free(vk);
  return r;
}

int jq_compile_args(jq_state *jq, const char* str, jv args) {
  jv_nomem_handler(jq->nomem_handler, jq->nomem_handler_data);
  assert(jv_get_kind(args) == JV_KIND_ARRAY || jv_get_kind(args) == JV_KIND_OBJECT);
  struct locfile* locations;
  locations = locfile_init(jq, "<top-level>", str, strlen(str));
  block program;
  jq_reset(jq);
  if (jq->bc) {
    bytecode_free(jq->bc);
    jq->bc = 0;
  }
  int nerrors = load_program(jq, locations, &program);
  if (nerrors == 0) {
    nerrors = builtins_bind(jq, &program);
    if (nerrors == 0) {
      nerrors = block_compile(program, &jq->bc, locations, args = args2obj(args));
    }
  } else
    jv_free(args);
  if (nerrors)
    jq_report_error(jq, jv_string_fmt("jq: %d compile %s", nerrors, nerrors > 1 ? "errors" : "error"));
  if (jq->bc)
    jq->bc = optimize(jq->bc);
  locfile_free(locations);
  return jq->bc != NULL;
}

int jq_compile(jq_state *jq, const char* str) {
  return jq_compile_args(jq, str, jv_object());
}

jv jq_get_jq_origin(jq_state *jq) {
  return jq_get_attr(jq, jv_string("JQ_ORIGIN"));
}

jv jq_get_prog_origin(jq_state *jq) {
  return jq_get_attr(jq, jv_string("PROGRAM_ORIGIN"));
}

jv jq_get_lib_dirs(jq_state *jq) {
  return jq_get_attr(jq, jv_string("JQ_LIBRARY_PATH"));
}

void jq_set_attrs(jq_state *jq, jv attrs) {
  assert(jv_get_kind(attrs) == JV_KIND_OBJECT);
  jv_free(jq->attrs);
  jq->attrs = attrs;
}

void jq_set_attr(jq_state *jq, jv attr, jv val) {
  jq->attrs = jv_object_set(jq->attrs, attr, val);
}

jv jq_get_attr(jq_state *jq, jv attr) {
  return jv_object_get(jv_copy(jq->attrs), attr);
}

void jq_dump_disassembly(jq_state *jq, int indent) {
  dump_disassembly(indent, jq->bc);
}

void jq_set_input_cb(jq_state *jq, jq_input_cb cb, void *data) {
  jq->input_cb = cb;
  jq->input_cb_data = data;
}

void jq_get_input_cb(jq_state *jq, jq_input_cb *cb, void **data) {
  *cb = jq->input_cb;
  *data = jq->input_cb_data;
}

void jq_set_debug_cb(jq_state *jq, jq_msg_cb cb, void *data) {
  jq->debug_cb = cb;
  jq->debug_cb_data = data;
}

void jq_get_debug_cb(jq_state *jq, jq_msg_cb *cb, void **data) {
  *cb = jq->debug_cb;
  *data = jq->debug_cb_data;
}

void
jq_halt(jq_state *jq, jv exit_code, jv error_message)
{
  assert(!jq->halted);
  jq->halted = 1;
  jq->exit_code = exit_code;
  jq->error_message = error_message;
}

int
jq_halted(jq_state *jq)
{
  return jq->halted;
}

jv jq_get_exit_code(jq_state *jq)
{
  return jv_copy(jq->exit_code);
}

jv jq_get_error_message(jq_state *jq)
{
  return jv_copy(jq->error_message);
}
