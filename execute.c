#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "execute.h"

#include "opcode.h"
#include "bytecode.h"

#include "forkable_stack.h"
#include "frame_layout.h"

#include "jv_alloc.h"
#include "jq_parser.h"
#include "locfile.h"
#include "jv.h"
#include "jv_aux.h"
#include "parser.h"
#include "builtin.h"

struct jq_state {
  struct forkable_stack data_stk;
  struct forkable_stack frame_stk;
  struct forkable_stack fork_stk;
  jv path;
  int subexp_nest;
  int debug_trace_enabled;
  int initial_execution;
};

typedef struct {
  FORKABLE_STACK_HEADER;
  jv val;
} data_stk_elem;

void stack_push(jq_state *jq, jv val) {
  assert(jv_is_valid(val));
  data_stk_elem* s = forkable_stack_push(&jq->data_stk, sizeof(data_stk_elem));
  s->val = val;
}

jv stack_pop(jq_state *jq) {
  data_stk_elem* s = forkable_stack_peek(&jq->data_stk);
  jv val = s->val;
  if (!forkable_stack_pop_will_free(&jq->data_stk)) {
    val = jv_copy(val);
  }
  forkable_stack_pop(&jq->data_stk);
  assert(jv_is_valid(val));
  return val;
}


struct forkpoint {
  FORKABLE_STACK_HEADER;
  struct forkable_stack_state saved_data_stack;
  struct forkable_stack_state saved_call_stack;
  int path_len, subexp_nest;
  uint16_t* return_address;
};


void stack_save(jq_state *jq, uint16_t* retaddr){
  struct forkpoint* fork = forkable_stack_push(&jq->fork_stk, sizeof(struct forkpoint));
  forkable_stack_save(&jq->data_stk, &fork->saved_data_stack);
  forkable_stack_save(&jq->frame_stk, &fork->saved_call_stack);
  fork->path_len = 
    jv_get_kind(jq->path) == JV_KIND_ARRAY ? jv_array_length(jv_copy(jq->path)) : 0;
  fork->subexp_nest = jq->subexp_nest;
  fork->return_address = retaddr;
}

void stack_switch(jq_state *jq) {
  struct forkpoint* fork = forkable_stack_peek(&jq->fork_stk);
  forkable_stack_switch(&jq->data_stk, &fork->saved_data_stack);
  forkable_stack_switch(&jq->frame_stk, &fork->saved_call_stack);
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
  while (!forkable_stack_empty(&jq->data_stk) && 
         forkable_stack_pop_will_free(&jq->data_stk)) {
    jv_free(stack_pop(jq));
  }
  while (!forkable_stack_empty(&jq->frame_stk) && 
         forkable_stack_pop_will_free(&jq->frame_stk)) {
    frame_pop(&jq->frame_stk);
  }

  if (forkable_stack_empty(&jq->fork_stk)) {
    return 0;
  }

  struct forkpoint* fork = forkable_stack_peek(&jq->fork_stk);
  uint16_t* retaddr = fork->return_address;
  forkable_stack_restore(&jq->data_stk, &fork->saved_data_stack);
  forkable_stack_restore(&jq->frame_stk, &fork->saved_call_stack);
  int path_len = fork->path_len;
  if (jv_get_kind(jq->path) == JV_KIND_ARRAY) {
    assert(path_len >= 0);
    jq->path = jv_array_slice(jq->path, 0, path_len);
  } else {
    assert(path_len == 0);
  }
  jq->subexp_nest = fork->subexp_nest;
  forkable_stack_pop(&jq->fork_stk);
  return retaddr;
}


static struct closure make_closure(struct forkable_stack* stk, frame_ptr fr, uint16_t* pc) {
  uint16_t level = *pc++;
  uint16_t idx = *pc++;
  fr = frame_get_level(stk, fr, level);
  if (idx & ARG_NEWCLOSURE) {
    int subfn_idx = idx & ~ARG_NEWCLOSURE;
    assert(subfn_idx < frame_self(fr)->bc->nsubfunctions);
    struct closure cl = {frame_self(fr)->bc->subfunctions[subfn_idx],
                         forkable_stack_to_idx(stk, fr)};
    return cl;
  } else {
    return *frame_closure_arg(fr, idx);
  }
}

void print_error(jv value) {
  assert(!jv_is_valid(value));
  jv msg = jv_invalid_get_msg(value);
  if (jv_get_kind(msg) == JV_KIND_STRING) {
    fprintf(stderr, "jq: error: %s\n", jv_string_value(msg));
  }
  jv_free(msg);
}
#define ON_BACKTRACK(op) ((op)+NUM_OPCODES)

jv jq_next(jq_state *jq) {
  jv cfunc_input[MAX_CFUNCTION_ARGS];

  uint16_t* pc = stack_restore(jq);
  assert(pc);

  int backtracking = !jq->initial_execution;
  jq->initial_execution = 0;
  while (1) {
    uint16_t opcode = *pc;

    if (jq->debug_trace_enabled) {
      dump_operation(frame_current_bytecode(&jq->frame_stk), pc);
      printf("\t");
      const struct opcode_description* opdesc = opcode_describe(opcode);
      data_stk_elem* param = 0;
      int stack_in = opdesc->stack_in;
      if (stack_in == -1) stack_in = pc[1];
      for (int i=0; i<stack_in; i++) {
        if (i == 0) {
          param = forkable_stack_peek(&jq->data_stk);
        } else {
          printf(" | ");
          param = forkable_stack_peek_next(&jq->data_stk, param);
        }
        if (!param) break;
        jv_dump(jv_copy(param->val), 0);
        //printf("<%d>", jv_get_refcnt(param->val));
        //printf(" -- ");
        //jv_dump(jv_copy(jq->path), 0);
      }

      if (backtracking) printf("\t<backtracking>");

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
      jv v = jv_array_get(jv_copy(frame_current_bytecode(&jq->frame_stk)->constants), *pc++);
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
      frame_ptr fp = frame_get_level(&jq->frame_stk, frame_current(&jq->frame_stk), level);
      jv* var = frame_local_var(fp, vidx);
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
        print_error(jv_invalid_with_msg(jv_string_fmt("Cannot use %s as object key",
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
      frame_ptr fp = frame_get_level(&jq->frame_stk, frame_current(&jq->frame_stk), level);
      jv* var = frame_local_var(fp, v);
      jv max = stack_pop(jq);
      if (jv_get_kind(*var) != JV_KIND_NUMBER ||
          jv_get_kind(max) != JV_KIND_NUMBER) {
        print_error(jv_invalid_with_msg(jv_string_fmt("Range bounds must be numeric")));
        jv_free(max);
        goto do_backtrack;
      } else if (jv_number_value(jv_copy(*var)) >= jv_number_value(jv_copy(max))) {
        /* finished iterating */
        goto do_backtrack;
      } else {
        jv curr = jv_copy(*var);
        *var = jv_number(jv_number_value(*var) + 1);

        stack_save(jq, pc - 3);
        stack_push(jq, jv_copy(max));
        stack_switch(jq);
        stack_push(jq, curr);
      }
      break;
    }

      // FIXME: loadv/storev may do too much copying/freeing
    case LOADV: {
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      frame_ptr fp = frame_get_level(&jq->frame_stk, frame_current(&jq->frame_stk), level);
      jv* var = frame_local_var(fp, v);
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
      frame_ptr fp = frame_get_level(&jq->frame_stk, frame_current(&jq->frame_stk), level);
      jv* var = frame_local_var(fp, v);
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
      frame_ptr fp = frame_get_level(&jq->frame_stk, frame_current(&jq->frame_stk), level);
      jv* var = frame_local_var(fp, v);
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

      stack_save(jq, pc - 1);
      stack_switch(jq);

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

      stack_save(jq, pc - 1);
      stack_push(jq, jv_copy(path));
      stack_switch(jq);

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
        print_error(v);
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
        print_error(jv_invalid_with_msg(jv_string_fmt("Cannot iterate over %s",
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
        stack_save(jq, pc - 1);
        stack_push(jq, container);
        stack_push(jq, jv_number(idx));
        stack_switch(jq);
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
      stack_save(jq, pc - 1);
      stack_switch(jq);
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
      cfunc_input[0] = top;
      for (int i = 1; i < nargs; i++) {
        cfunc_input[i] = stack_pop(jq);
      }
      struct cfunction* func = &frame_current_bytecode(&jq->frame_stk)->globals->cfunctions[*pc++];
      top = cfunction_invoke(func, cfunc_input);
      if (jv_is_valid(top)) {
        stack_push(jq, top);
      } else {
        print_error(top);
        goto do_backtrack;
      }
      break;
    }

    case CALL_JQ: {
      uint16_t nclosures = *pc++;
      uint16_t* retaddr = pc + 2 + nclosures*2;
      frame_ptr new_frame = frame_push(&jq->frame_stk, 
                                       make_closure(&jq->frame_stk, frame_current(&jq->frame_stk), pc),
                                       retaddr);
      pc += 2;
      frame_ptr old_frame = forkable_stack_peek_next(&jq->frame_stk, new_frame);
      assert(nclosures == frame_self(new_frame)->bc->nclosures);
      for (int i=0; i<nclosures; i++) {
        *frame_closure_arg(new_frame, i) = make_closure(&jq->frame_stk, old_frame, pc);
        pc += 2;
      }

      pc = frame_current_bytecode(&jq->frame_stk)->code;
      break;
    }

    case RET: {
      uint16_t* retaddr = *frame_current_retaddr(&jq->frame_stk);
      if (retaddr) {
        // function return
        pc = retaddr;
        frame_pop(&jq->frame_stk);
      } else {
        // top-level return, yielding value
        jv value = stack_pop(jq);
        stack_save(jq, pc - 1);
        stack_push(jq, jv_null());
        stack_switch(jq);
        return value;
      }
      break;
    }
    case ON_BACKTRACK(RET): {
      // resumed after top-level return
      goto do_backtrack;
    }
    }
  }
}


void jq_init(struct bytecode* bc, jv input, jq_state **jq, int flags) {
  jq_state *new_jq;
  new_jq = jv_mem_alloc(sizeof(*new_jq));
  memset(new_jq, 0, sizeof(*new_jq));
  new_jq->path = jv_null();
  forkable_stack_init(&new_jq->data_stk, sizeof(data_stk_elem) * 100);
  forkable_stack_init(&new_jq->frame_stk, 1024);
  forkable_stack_init(&new_jq->fork_stk, 1024);
  
  stack_push(new_jq, input);
  struct closure top = {bc, -1};
  frame_push(&new_jq->frame_stk, top, 0);
  stack_save(new_jq, bc->code);
  stack_switch(new_jq);
  if (flags & JQ_DEBUG_TRACE) {
    new_jq->debug_trace_enabled = 1;
  } else {
    new_jq->debug_trace_enabled = 0;
  }
  new_jq->initial_execution = 1;
  *jq = new_jq;
}

void jq_teardown(jq_state **jq) {
  jq_state *old_jq = *jq;
  if (old_jq == NULL)
    return;
  *jq = NULL;

  while (stack_restore(old_jq)) {}

  assert(forkable_stack_empty(&old_jq->fork_stk));
  assert(forkable_stack_empty(&old_jq->data_stk));
  assert(forkable_stack_empty(&old_jq->frame_stk));
  forkable_stack_free(&old_jq->fork_stk);
  forkable_stack_free(&old_jq->data_stk);
  forkable_stack_free(&old_jq->frame_stk);

  jv_free(old_jq->path);
  jv_mem_free(old_jq);
}

struct bytecode* jq_compile_args(const char* str, jv args) {
  assert(jv_get_kind(args) == JV_KIND_ARRAY);
  struct locfile locations;
  locfile_init(&locations, str, strlen(str));
  block program;
  struct bytecode* bc = 0;
  int nerrors = jq_parse(&locations, &program);
  if (nerrors == 0) {
    for (int i=0; i<jv_array_length(jv_copy(args)); i++) {
      jv arg = jv_array_get(jv_copy(args), i);
      jv name = jv_object_get(jv_copy(arg), jv_string("name"));
      jv value = jv_object_get(arg, jv_string("value"));
      program = gen_var_binding(gen_const(value), jv_string_value(name), program);
      jv_free(name);
    }
    jv_free(args);
    program = builtins_bind(program);
    nerrors = block_compile(program, &locations, &bc);
  }
  if (nerrors) {
    fprintf(stderr, "%d compile %s\n", nerrors, nerrors > 1 ? "errors" : "error");
  }
  locfile_free(&locations);
  return bc;
}

struct bytecode* jq_compile(const char* str) {
  return jq_compile_args(str, jv_array());
}
