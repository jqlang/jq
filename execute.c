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

typedef struct {
  jv value;
  int pathidx;
} stackval;

struct jq_state {
  struct forkable_stack data_stk;
  struct forkable_stack frame_stk;
  struct forkable_stack fork_stk;
  jv* pathbuf;
  int pathsize; // number of allocated elements
  int debug_trace_enabled;
};

int path_push(jq_state *jq, stackval sv, jv val) {
  int pos = sv.pathidx;
  assert(pos <= jq->pathsize);
  assert(pos >= 0);
  if (pos == jq->pathsize) {
    int oldpathsize = jq->pathsize;
    jq->pathsize = oldpathsize ? oldpathsize * 2 : 100;
    jq->pathbuf = jv_mem_realloc(jq->pathbuf, sizeof(jq->pathbuf[0]) * jq->pathsize);
    for (int i=oldpathsize; i<jq->pathsize; i++) {
      jq->pathbuf[i] = jv_invalid();
    }
  }
  jv_free(jq->pathbuf[pos]);
  jq->pathbuf[pos] = val;
  return pos + 1;
}

stackval stackval_replace(stackval value, jv newjs) {
  jv_free(value.value);
  stackval s = {newjs, value.pathidx};
  return s;
}


// Probably all uses of this function are bugs
stackval stackval_root(jv v) {
  stackval s = {v, 0};
  return s;
}

typedef struct {
  FORKABLE_STACK_HEADER;
  stackval sv;
} data_stk_elem;

void stack_push(jq_state *jq, stackval val) {
  assert(jv_is_valid(val.value));
  data_stk_elem* s = forkable_stack_push(&jq->data_stk, sizeof(data_stk_elem));
  s->sv = val;
}

stackval stack_pop(jq_state *jq) {
  data_stk_elem* s = forkable_stack_peek(&jq->data_stk);
  stackval sv = s->sv;
  if (!forkable_stack_pop_will_free(&jq->data_stk)) {
    sv.value = jv_copy(sv.value);
  }
  forkable_stack_pop(&jq->data_stk);
  assert(jv_is_valid(sv.value));
  return sv;
}



struct forkpoint {
  FORKABLE_STACK_HEADER;
  struct forkable_stack_state saved_data_stack;
  struct forkable_stack_state saved_call_stack;
};


void stack_save(jq_state *jq){
  struct forkpoint* fork = forkable_stack_push(&jq->fork_stk, sizeof(struct forkpoint));
  forkable_stack_save(&jq->data_stk, &fork->saved_data_stack);
  forkable_stack_save(&jq->frame_stk, &fork->saved_call_stack);
}

void stack_switch(jq_state *jq) {
  struct forkpoint* fork = forkable_stack_peek(&jq->fork_stk);
  forkable_stack_switch(&jq->data_stk, &fork->saved_data_stack);
  forkable_stack_switch(&jq->frame_stk, &fork->saved_call_stack);
}

int stack_restore(jq_state *jq){
  while (!forkable_stack_empty(&jq->data_stk) && 
         forkable_stack_pop_will_free(&jq->data_stk)) {
    jv_free(stack_pop(jq).value);
  }
  while (!forkable_stack_empty(&jq->frame_stk) && 
         forkable_stack_pop_will_free(&jq->frame_stk)) {
    frame_pop(&jq->frame_stk);
  }
  if (forkable_stack_empty(&jq->fork_stk)) {
    return 0;
  } else {
    struct forkpoint* fork = forkable_stack_peek(&jq->fork_stk);
    forkable_stack_restore(&jq->data_stk, &fork->saved_data_stack);
    forkable_stack_restore(&jq->frame_stk, &fork->saved_call_stack);
    forkable_stack_pop(&jq->fork_stk);
    return 1;
  }
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

  assert(!forkable_stack_empty(&jq->frame_stk));
  uint16_t* pc = *frame_current_retaddr(&jq->frame_stk);
  frame_pop(&jq->frame_stk);
  
  assert(!forkable_stack_empty(&jq->frame_stk));

  int backtracking = 0;
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
        jv_dump(jv_copy(param->sv.value), 0);
        //printf("<%d>", jv_get_refcnt(param->sv.value));
        printf("<%d>", param->sv.pathidx);
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
      stack_push(jq, stackval_replace(stack_pop(jq), v));
      break;
    }

    case DUP: {
      stackval v = stack_pop(jq);
      stackval v2 = v;
      v2.value = jv_copy(v.value);
      stack_push(jq, v);
      stack_push(jq, v2);
      break;
    }

    case DUP2: {
      stackval keep = stack_pop(jq);
      stackval v = stack_pop(jq);
      stackval v2 = v;
      v2.value = jv_copy(v.value);
      stack_push(jq, v);
      stack_push(jq, keep);
      stack_push(jq, v2);
      break;
    }

    case SWAP: {
      stackval a = stack_pop(jq);
      stackval b = stack_pop(jq);
      stack_push(jq, a);
      stack_push(jq, b);
      break;
    }
      
    case POP: {
      jv_free(stack_pop(jq).value);
      break;
    }

    case APPEND: {
      // FIXME paths
      jv v = stack_pop(jq).value;
      uint16_t level = *pc++;
      uint16_t vidx = *pc++;
      frame_ptr fp = frame_get_level(&jq->frame_stk, frame_current(&jq->frame_stk), level);
      jv* var = frame_local_var(fp, vidx);
      assert(jv_get_kind(*var) == JV_KIND_ARRAY);
      *var = jv_array_append(*var, v);
      break;
    }

    case INSERT: {
      stackval stktop = stack_pop(jq);
      jv v = stack_pop(jq).value;
      jv k = stack_pop(jq).value;
      stackval objv = stack_pop(jq);
      assert(jv_get_kind(k) == JV_KIND_STRING);
      assert(jv_get_kind(objv.value) == JV_KIND_OBJECT);
      objv.value = jv_object_set(objv.value, k, v);
      stack_push(jq, objv);
      stack_push(jq, stktop);
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
      stack_push(jq, stackval_replace(stack_pop(jq), jv_copy(*var)));
      break;
    }

    case STOREV: {
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      frame_ptr fp = frame_get_level(&jq->frame_stk, frame_current(&jq->frame_stk), level);
      jv* var = frame_local_var(fp, v);
      stackval val = stack_pop(jq);
      if (jq->debug_trace_enabled) {
        printf("V%d = ", v);
        jv_dump(jv_copy(val.value), 0);
        printf("\n");
      }
      jv_free(*var);
      *var = val.value;
      break;
    }

    case GETPATH: {
      stackval path_end = stack_pop(jq);
      stackval path_start = stack_pop(jq);
      jv_free(path_end.value);
      jv path = jv_array();
      for (int i=path_start.pathidx; i<path_end.pathidx; i++) {
        path = jv_array_append(path, jv_copy(jq->pathbuf[i]));
      }
      stack_push(jq, stackval_replace(path_start, path));
      break;
    }

    case INDEX: {
      stackval t = stack_pop(jq);
      jv k = stack_pop(jq).value;
      int pathidx = path_push(jq, t, jv_copy(k));
      jv v = jv_get(t.value, k);
      if (jv_is_valid(v)) {
        stackval sv;
        sv.value = v;
        sv.pathidx = pathidx;
        stack_push(jq, sv);
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
      stackval t = stack_pop(jq);
      jv_kind kind = jv_get_kind(t.value);
      if (kind == JV_KIND_FALSE || kind == JV_KIND_NULL) {
        pc += offset;
      }
      stack_push(jq, t); // FIXME do this better
      break;
    }

    case EACH: 
      stack_push(jq, stackval_root(jv_number(-1)));
      // fallthrough
    case ON_BACKTRACK(EACH): {
      int idx = jv_number_value(stack_pop(jq).value);
      stackval container = stack_pop(jq);

      int keep_going;
      jv key, value;
      if (jv_get_kind(container.value) == JV_KIND_ARRAY) {
        if (opcode == EACH) idx = 0;
        else idx = idx + 1;
        keep_going = idx < jv_array_length(jv_copy(container.value));
        if (keep_going) {
          key = jv_number(idx);
          value = jv_array_get(jv_copy(container.value), idx);
        }
      } else if (jv_get_kind(container.value) == JV_KIND_OBJECT) {
        if (opcode == EACH) idx = jv_object_iter(container.value);
        else idx = jv_object_iter_next(container.value, idx);
        keep_going = jv_object_iter_valid(container.value, idx);
        if (keep_going) {
          key = jv_object_iter_key(container.value, idx);
          value = jv_object_iter_value(container.value, idx);
        }
      } else {
        assert(opcode == EACH);
        print_error(jv_invalid_with_msg(jv_string_fmt("Cannot iterate over %s",
                                                      jv_kind_name(jv_get_kind(container.value)))));
        keep_going = 0;
      }

      if (!keep_going) {
        jv_free(container.value);
        goto do_backtrack;
      } else {
        stack_save(jq);
        stack_push(jq, container);
        stack_push(jq, stackval_root(jv_number(idx)));
        frame_push_backtrack(&jq->frame_stk, pc - 1);
        stack_switch(jq);
        
        stackval sv = {value,
                       path_push(jq, container, key)};
        stack_push(jq, sv);
      }
      break;
    }

    do_backtrack:
    case BACKTRACK: {
      if (!stack_restore(jq)) {
        return jv_invalid();
      }
      pc = *frame_current_retaddr(&jq->frame_stk);
      frame_pop(&jq->frame_stk);
      backtracking = 1;
      break;
    }

    case FORK: {
      stack_save(jq);
      frame_push_backtrack(&jq->frame_stk, pc - 1);
      stack_switch(jq);
      pc++; // skip offset this time
      break;
    }

    case ON_BACKTRACK(FORK): {
      uint16_t offset = *pc++;
      pc += offset;
      break;
    }

    case YIELD: {
      jv value = stack_pop(jq).value;
      frame_push_backtrack(&jq->frame_stk, pc);
      return value;
    }
      
    case CALL_BUILTIN: {
      int nargs = *pc++;
      stackval top = stack_pop(jq);
      cfunc_input[0] = top.value;
      for (int i = 1; i < nargs; i++) {
        cfunc_input[i] = stack_pop(jq).value;
      }
      struct cfunction* func = &frame_current_bytecode(&jq->frame_stk)->globals->cfunctions[*pc++];
      top.value = cfunction_invoke(func, cfunc_input);
      if (jv_is_valid(top.value)) {
        stack_push(jq, top);
      } else {
        print_error(top.value);
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
      pc = *frame_current_retaddr(&jq->frame_stk);
      frame_pop(&jq->frame_stk);
      break;
    }
    }
  }
}


void jq_init(struct bytecode* bc, jv input, jq_state **jq, int flags) {
  jq_state *new_jq;
  new_jq = jv_mem_alloc(sizeof(*new_jq));
  memset(new_jq, 0, sizeof(*new_jq));
  forkable_stack_init(&new_jq->data_stk, sizeof(stackval) * 100);
  forkable_stack_init(&new_jq->frame_stk, 1024);
  forkable_stack_init(&new_jq->fork_stk, 1024);
  
  stack_push(new_jq, stackval_root(input));
  struct closure top = {bc, -1};
  frame_push(&new_jq->frame_stk, top, 0);
  frame_push_backtrack(&new_jq->frame_stk, bc->code);
  if (flags & JQ_DEBUG_TRACE) {
    new_jq->debug_trace_enabled = 1;
  } else {
    new_jq->debug_trace_enabled = 0;
  }
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

  for (int i=0; i<old_jq->pathsize; i++) {
    jv_free(old_jq->pathbuf[i]);
  }
  jv_mem_free(old_jq->pathbuf);
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
