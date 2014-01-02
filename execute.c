#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "exec_stack.h"
#include "bytecode.h"

#include "jv_alloc.h"
#include "jq_parser.h"
#include "locfile.h"
#include "jv.h"
#include "jq.h"
#include "parser.h"
#include "builtin.h"

typedef struct jq_handle {
  void *handle;
  void (*destructor)(void *);
  jv type;
} jq_handle;

struct jq_state {
  void (*nomem_handler)(void *);
  void *nomem_handler_data;
  struct bytecode* bc;

  jq_err_cb err_cb;
  void *err_cb_data;

  struct stack stk;
  stack_ptr curr_frame;
  stack_ptr stk_top;
  stack_ptr fork_top;

  jv path;
  int subexp_nest;
  int debug_trace_enabled;
  int initial_execution;

  jq_handle *handles;
  int nhandles;

  jq_runtime_flags flags;
};

int jq_handle_create(jq_state *jq,
                     int desired_handle,
                     const char *type,
                     void *handle,
                     void (*destructor)(void *)) {
  int i;

  errno = EINVAL;
  if (type == NULL || *type == '\0')
    return -1;

  if (desired_handle > -1) {
    i = desired_handle;
    if (i < jq->nhandles)
      jq_handle_delete(jq, i);
  } else {
    for (i = 0; i < jq->nhandles; i++) {
      if (jq->handles[i].handle == NULL)
        break;
    }
  }

  /* FIXME This should be a #define in some header or a tunable */
  if (i > 128)
    return -1;

  if (i >= jq->nhandles) {
    jq->handles = jv_mem_realloc(jq->handles, sizeof(*jq->handles) * (i + 1));
    /* FIXME: don't use memset() for this */
    memset(&jq->handles[jq->nhandles], 0, sizeof(*jq->handles) * (i - jq->nhandles));
    jq->nhandles = i + 1;
  }

  jq->handles[i].type = jv_string(type);
  jq->handles[i].handle = handle;
  jq->handles[i].destructor = destructor;

  return i;
}

static void destroy_stdio_handle(void *data) {
  struct jq_stdio_handle *h = data;
  if (h->f != NULL && h->is_pipe)
    pclose(h->f);
  else if (h->f != NULL && h->close_it)
    fclose(h->f);
  if (h->p)
      jv_parser_free(h->p);
  jv_free(h->s);
  jv_mem_free(h);
}

int jq_handle_create_null(jq_state *jq, int desired_handle) {
  return jq_handle_create(jq, desired_handle, "null", &jq, NULL); // Any "handle" address will do
}

static void destroy_buffer_handle(void *data) {
  jv_free(*(jv *)data);
  jv_mem_free(data);
}

int jq_handle_create_buffer(jq_state *jq, int desired_handle) {
  jv *v = jv_mem_alloc(sizeof(*v));
  *v = jv_null();

  int hdl = jq_handle_create(jq, desired_handle, "buffer", v, destroy_buffer_handle);
  if (hdl < 0)
    jv_free(*v);
  return hdl;
}

int jq_handle_create_stdio(jq_state *jq,
                           int desired_handle,
                           FILE *f,
                           int close_it,
                           int is_pipe,
                           int raw,
                           int slurp) {
  struct jq_stdio_handle *h = jv_mem_alloc(sizeof(*h));

  h->buf[0] = '\0';
  h->f = f;
  h->p = NULL;
  h->s = jv_invalid();
  h->close_it = close_it;
  h->is_pipe = is_pipe;
  if (!raw)
    h->p = jv_parser_new(0);
  if (slurp && raw)
    h->s = jv_string("");
  else if (slurp)
    h->s = jv_array();

  int hdl = jq_handle_create(jq, desired_handle, "FILE", h, destroy_stdio_handle);
  if (hdl < 0)
    destroy_stdio_handle(h);
  return hdl;
}

static int
handle_validate(jq_state *jq, const char *type, int handle) {
  errno = EBADF;
  if (handle >= jq->nhandles)
    return 0;

  if (jq->handles[handle].handle == NULL)
    return 0;

  errno = EINVAL;
  if (type == NULL || *type == '\0' ||
      !jv_is_valid(jq->handles[handle].type) ||
      strcmp(type, jv_string_value(jq->handles[handle].type)) != 0)
    return 0;
  return 1;
}

int jq_handle_get(jq_state *jq,
                  const char *type,
                  int hdl,
                  void **handle,
                  void (**destructor)(void *)) {
  if (handle != NULL)
    *handle = NULL;
  if (destructor != NULL)
    *destructor = NULL;

  if (!handle_validate(jq, type, hdl))
    return 0;

  if (handle != NULL)
    *handle = jq->handles[hdl].handle;
  if (destructor != NULL)
    *destructor = jq->handles[hdl].destructor;
  return 1;
}

void jq_handle_delete(jq_state *jq, int hdl) {
  if (hdl < 0 || hdl >= jq->nhandles || jq->handles[hdl].handle == NULL)
    return;

  if (jq->handles[hdl].destructor != NULL)
    jq->handles[hdl].destructor(jq->handles[hdl].handle);
  jv_free(jq->handles[hdl].type);
  jq->handles[hdl].type = jv_null();
  jq->handles[hdl].destructor = NULL;
  jq->handles[hdl].handle = NULL;
}


struct closure {
  struct bytecode* bc;
  stack_ptr env;
};

union frame_entry {
  struct closure closure;
  jv localvar;
};

struct frame {
  struct bytecode* bc;
  stack_ptr env;
  stack_ptr retdata;
  uint16_t* retaddr;
  /* bc->nclosures closures followed by bc->nlocals local variables */
  union frame_entry entries[0]; 
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
    int subfn_idx = idx & ~ARG_NEWCLOSURE;
    assert(subfn_idx < fr->bc->nsubfunctions);
    struct closure cl = {fr->bc->subfunctions[subfn_idx],
                         fridx};
    return cl;
  } else {
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


struct forkpoint {
  stack_ptr saved_data_stack;
  stack_ptr saved_curr_frame;
  int path_len, subexp_nest;
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
  fork->subexp_nest = jq->subexp_nest;
  fork->return_address = retaddr;
  jq->stk_top = sp.saved_data_stack;
  jq->curr_frame = sp.saved_curr_frame;
}

void path_append(jq_state* jq, jv component) {
  if (jq->subexp_nest == 0 && jv_get_kind(jq->path) == JV_KIND_ARRAY) {
    int n1 = jv_array_length(jv_copy(jq->path));
    jq->path = jv_array_append(jq->path, component);
    int n2 = jv_array_length(jv_copy(jq->path));
    assert(n2 == n1 + 1);
  } else {
    jv_free(component);
  }
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
    assert(path_len == 0);
  }
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

  if (jv_get_kind(jq->path) != JV_KIND_INVALID)
    jv_free(jq->path);
  jq->path = jv_null();
  jq->subexp_nest = 0;
}

static void print_error(jq_state *jq, jv value) {
  assert(!jv_is_valid(value));
  jv msg = jv_invalid_get_msg(value);
  jv msg2;
  if (jv_get_kind(msg) == JV_KIND_STRING)
    msg2 = jv_string_fmt("jq: error: %s", jv_string_value(msg));
  else
    msg2 = jv_string_fmt("jq: error: <unknown>");
  jv_free(msg);

  if (jq->err_cb)
    jq->err_cb(jq->err_cb_data, msg2);
  else if (jv_get_kind(msg2) == JV_KIND_STRING)
    fprintf(stderr, "%s\n", jv_string_value(msg2));
  else
    fprintf(stderr, "jq: error: %s\n", strerror(ENOMEM));
  jv_free(msg2);
}
#define ON_BACKTRACK(op) ((op)+NUM_OPCODES)

jv jq_next(jq_state *jq) {
  jv cfunc_input[MAX_CFUNCTION_ARGS];

  jv_nomem_handler(jq->nomem_handler, jq->nomem_handler_data);

  uint16_t* pc = stack_restore(jq);
  assert(pc);

  int backtracking = !jq->initial_execution;
  jq->initial_execution = 0;
  while (1) {
    uint16_t opcode = *pc;

    if (jq->debug_trace_enabled) {
      dump_operation(frame_current(jq)->bc, pc);
      printf("\t");
      const struct opcode_description* opdesc = opcode_describe(opcode);
      stack_ptr param = 0;
      if (!backtracking) {
        int stack_in = opdesc->stack_in;
        if (stack_in == -1) stack_in = pc[1];
        for (int i=0; i<stack_in; i++) {
          if (i == 0) {
            param = jq->stk_top;
          } else {
            printf(" | ");
            param = *stack_block_next(&jq->stk, param);
          }
          if (!param) break;
          jv_dump(jv_copy(*(jv*)stack_block(&jq->stk, param)), 0);
          //printf("<%d>", jv_get_refcnt(param->val));
          //printf(" -- ");
          //jv_dump(jv_copy(jq->path), 0);
        }
      } else {
        printf("\t<backtracking>");
      }

      printf("\n");
    }

    if (backtracking) {
      opcode = ON_BACKTRACK(opcode);
      backtracking = 0;
    }
    pc++;

    switch (opcode) {
    default: assert(0 && "invalid instruction");

    case LOADK: {
      jv v = jv_array_get(jv_copy(frame_current(jq)->bc->constants), *pc++);
      assert(jv_is_valid(v));
      jv_free(stack_pop(jq));
      stack_push(jq, v);
      break;
    }

    case DUP: {
      jv v = stack_pop(jq);
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
        print_error(jq, jv_invalid_with_msg(jv_string_fmt("Cannot use %s as object key",
                                                          jv_kind_name(jv_get_kind(k)))));
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
      if (jv_get_kind(*var) != JV_KIND_NUMBER ||
          jv_get_kind(max) != JV_KIND_NUMBER) {
        print_error(jq, jv_invalid_with_msg(jv_string_fmt("Range bounds must be numeric")));
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
        printf("\n");
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
        printf("\n");
      }
      jv_free(stack_pop(jq));
      stack_push(jq, *var);
      *var = jv_null();
      break;
    }

    case STOREV: {
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      jv* var = frame_local_var(jq, v, level);
      jv val = stack_pop(jq);
      if (jq->debug_trace_enabled) {
        printf("V%d = ", v);
        jv_dump(jv_copy(val), 0);
        printf("\n");
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
      stack_push(jq, v);

      jq->path = jv_array();
      jq->subexp_nest = 0;
      break;
    }

    case PATH_END: {
      jv v = stack_pop(jq);
      jv_free(v); // discard value, only keep path

      int old_subexp_nest = (int)jv_number_value(stack_pop(jq));

      jv path = jq->path;
      jq->path = stack_pop(jq);

      struct stack_pos spos = stack_get_pos(jq);
      stack_push(jq, jv_copy(path));
      stack_save(jq, pc - 1, spos);

      stack_push(jq, path);
      jq->subexp_nest = old_subexp_nest;
      break;
    }

    case ON_BACKTRACK(PATH_BEGIN):
    case ON_BACKTRACK(PATH_END): {
      jv_free(jq->path);
      jq->path = stack_pop(jq);
      goto do_backtrack;
    }

    case INDEX: {
      jv t = stack_pop(jq);
      jv k = stack_pop(jq);
      path_append(jq, jv_copy(k));
      jv v = jv_get(t, k);
      if (jv_is_valid(v)) {
        stack_push(jq, v);
      } else {
        print_error(jq, v);
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
      stack_push(jq, jv_number(-1));
      // fallthrough
    case ON_BACKTRACK(EACH): {
      int idx = jv_number_value(stack_pop(jq));
      jv container = stack_pop(jq);

      int keep_going, is_last = 0;
      jv key, value;
      if (jv_get_kind(container) == JV_KIND_ARRAY) {
        if (opcode == EACH) idx = 0;
        else idx = idx + 1;
        int len = jv_array_length(jv_copy(container));
        keep_going = idx < len;
        is_last = idx == len - 1;
        if (keep_going) {
          key = jv_number(idx);
          value = jv_array_get(jv_copy(container), idx);
        }
      } else if (jv_get_kind(container) == JV_KIND_OBJECT) {
        if (opcode == EACH) idx = jv_object_iter(container);
        else idx = jv_object_iter_next(container, idx);
        keep_going = jv_object_iter_valid(container, idx);
        if (keep_going) {
          key = jv_object_iter_key(container, idx);
          value = jv_object_iter_value(container, idx);
        }
      } else {
        assert(opcode == EACH);
        print_error(jq, jv_invalid_with_msg(jv_string_fmt("Cannot iterate over %s",
                                                          jv_kind_name(jv_get_kind(container)))));
        keep_going = 0;
      }

      if (!keep_going) {
        jv_free(container);
        goto do_backtrack;
      } else if (is_last) {
        // we don't need to make a backtrack point
        jv_free(container);
        path_append(jq, key);
        stack_push(jq, value);
      } else {
        struct stack_pos spos = stack_get_pos(jq);
        stack_push(jq, container);
        stack_push(jq, jv_number(idx));
        stack_save(jq, pc - 1, spos);
        path_append(jq, key);
        stack_push(jq, value);
      }
      break;
    }

    do_backtrack:
    case BACKTRACK: {
      pc = stack_restore(jq);
      if (!pc) {
        return jv_invalid();
      }
      backtracking = 1;
      break;
    }

    case FORK: {
      stack_save(jq, pc - 1, stack_get_pos(jq));
      pc++; // skip offset this time
      break;
    }

    case ON_BACKTRACK(FORK): {
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
      typedef jv (*func_1)(jq_state *, jv);
      typedef jv (*func_2)(jq_state *, jv,jv);
      typedef jv (*func_3)(jq_state *, jv,jv,jv);
      typedef jv (*func_4)(jq_state *, jv,jv,jv,jv);
      typedef jv (*func_5)(jq_state *, jv,jv,jv,jv,jv);
      switch (function->nargs) {
      case 1: top = ((func_1)function->fptr)(jq, in[0]); break;
      case 2: top = ((func_2)function->fptr)(jq, in[0], in[1]); break;
      case 3: top = ((func_3)function->fptr)(jq, in[0], in[1], in[2]); break;
      case 4: top = ((func_4)function->fptr)(jq, in[0], in[1], in[2], in[3]); break;
      case 5: top = ((func_5)function->fptr)(jq, in[0], in[1], in[2], in[3], in[4]); break;
      default: return jv_invalid_with_msg(jv_string("Function takes too many arguments"));
      }
      
      if (jv_is_valid(top)) {
        stack_push(jq, top);
      } else {
        print_error(jq, top);
        goto do_backtrack;
      }
      break;
    }

    case CALL_JQ: {
      jv input = stack_pop(jq);
      uint16_t nclosures = *pc++;
      uint16_t* retaddr = pc + 2 + nclosures*2;
      struct frame* new_frame = frame_push(jq, make_closure(jq, pc),
                                           pc + 2, nclosures);
      new_frame->retdata = jq->stk_top;
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

jq_state *jq_init(void) {
  jq_state *jq;
  jq = jv_mem_alloc_unguarded(sizeof(*jq));
  if (jq == NULL)
    return NULL;

  jq->bc = 0;

  stack_init(&jq->stk);
  jq->stk_top = 0;
  jq->fork_top = 0;
  jq->curr_frame = 0;

  jq->err_cb = NULL;
  jq->err_cb_data = NULL;

  jq->handles = NULL;
  jq->nhandles = 0;

  jq->path = jv_null();
  return jq;
}

void jq_set_error_cb(jq_state *jq, jq_err_cb cb, void *data) {
  jq->err_cb = cb;
  jq->err_cb_data = data;
}

void jq_get_error_cb(jq_state *jq, jq_err_cb *cb, void **data) {
  *cb = jq->err_cb;
  *data = jq->err_cb_data;
}

void jq_set_nomem_handler(jq_state *jq, void (*nomem_handler)(void *), void *data) {
  jv_nomem_handler(nomem_handler, data);
  jq->nomem_handler = nomem_handler;
  jq->nomem_handler_data = data;
}


void jq_start(jq_state *jq, jv input, jq_runtime_flags flags) {
  jv_nomem_handler(jq->nomem_handler, jq->nomem_handler_data);
  jq_reset(jq);

  jq->flags = flags;
  
  struct closure top = {jq->bc, -1};
  struct frame* top_frame = frame_push(jq, top, 0, 0);
  top_frame->retdata = 0;
  top_frame->retaddr = 0;

  stack_push(jq, input);
  stack_save(jq, jq->bc->code, stack_get_pos(jq));
  if (flags & JQ_DEBUG_TRACE) {
    jq->debug_trace_enabled = 1;
  } else {
    jq->debug_trace_enabled = 0;
  }
  jq->initial_execution = 1;
}

jq_runtime_flags jq_flags(jq_state *jq) {
  return jq->flags;
}

void jq_teardown(jq_state **jq) {
  jq_state *old_jq = *jq;
  if (old_jq == NULL)
    return;
  *jq = NULL;

  jq_reset(old_jq);
  bytecode_free(old_jq->bc);
  old_jq->bc = 0;

  for (int i = 0; i < old_jq->nhandles; i++) {
    if (old_jq->handles[i].handle != NULL)
      jq_handle_delete(old_jq, i);
  }
  jv_mem_free(old_jq->handles);
  jv_mem_free(old_jq);
}

int jq_compile_args(jq_state *jq, const char* str, jv args) {
  jv_nomem_handler(jq->nomem_handler, jq->nomem_handler_data);
  assert(jv_get_kind(args) == JV_KIND_ARRAY);
  struct locfile locations;
  locfile_init(&locations, jq, str, strlen(str));
  block program;
  jq_reset(jq);
  if (jq->bc) {
    bytecode_free(jq->bc);
    jq->bc = 0;
  }
  int nerrors = jq_parse(&locations, &program);
  if (nerrors == 0) {
    for (int i=0; i<jv_array_length(jv_copy(args)); i++) {
      jv arg = jv_array_get(jv_copy(args), i);
      jv name = jv_object_get(jv_copy(arg), jv_string("name"));
      jv value = jv_object_get(arg, jv_string("value"));
      program = gen_var_binding(gen_const(value), jv_string_value(name), program);
      jv_free(name);
    }
    nerrors = builtins_bind(jq, &program);
    if (nerrors == 0) {
      nerrors = block_compile(program, &locations, &jq->bc);
    }
  }
  jv_free(args);
  if (nerrors) {
    jv s = jv_string_fmt("%d compile %s", nerrors,
                         nerrors > 1 ? "errors" : "error");
    if (jq->err_cb != NULL)
      jq->err_cb(jq->err_cb_data, s);
    else if (!jv_is_valid(s))
      fprintf(stderr, "Error formatting jq compilation errors: %s\n", strerror(errno));
    else
      fprintf(stderr, "%s\n", jv_string_value(s));
    jv_free(s);
  }
  locfile_free(&locations);
  return jq->bc != NULL;
}

int jq_compile(jq_state *jq, const char* str) {
  return jq_compile_args(jq, str, jv_array());
}

void jq_dump_disassembly(jq_state *jq, int indent) {
  dump_disassembly(indent, jq->bc);
}
