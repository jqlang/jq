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


jv* pathbuf;
int pathsize; // number of allocated elements

int path_push(stackval sv, jv val) {
  int pos = sv.pathidx;
  assert(pos <= pathsize);
  assert(pos >= 0);
  if (pos == pathsize) {
    int oldpathsize = pathsize;
    pathsize = oldpathsize ? oldpathsize * 2 : 100;
    pathbuf = jv_mem_realloc(pathbuf, sizeof(pathbuf[0]) * pathsize);
    for (int i=oldpathsize; i<pathsize; i++) {
      pathbuf[i] = jv_invalid();
    }
  }
  jv_free(pathbuf[pos]);
  pathbuf[pos] = val;
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

struct forkable_stack data_stk;
typedef struct {
  FORKABLE_STACK_HEADER;
  stackval sv;
} data_stk_elem;

void stack_push(stackval val) {
  assert(jv_is_valid(val.value));
  data_stk_elem* s = forkable_stack_push(&data_stk, sizeof(data_stk_elem));
  s->sv = val;
}

stackval stack_pop() {
  data_stk_elem* s = forkable_stack_peek(&data_stk);
  stackval sv = s->sv;
  if (!forkable_stack_pop_will_free(&data_stk)) {
    sv.value = jv_copy(sv.value);
  }
  forkable_stack_pop(&data_stk);
  assert(jv_is_valid(sv.value));
  return sv;
}

struct forkable_stack frame_stk;


struct forkpoint {
  FORKABLE_STACK_HEADER;
  struct forkable_stack_state saved_data_stack;
  struct forkable_stack_state saved_call_stack;
};

struct forkable_stack fork_stk;

void stack_save(){
  struct forkpoint* fork = forkable_stack_push(&fork_stk, sizeof(struct forkpoint));
  forkable_stack_save(&data_stk, &fork->saved_data_stack);
  forkable_stack_save(&frame_stk, &fork->saved_call_stack);
}

void stack_switch() {
  struct forkpoint* fork = forkable_stack_peek(&fork_stk);
  forkable_stack_switch(&data_stk, &fork->saved_data_stack);
  forkable_stack_switch(&frame_stk, &fork->saved_call_stack);
}

int stack_restore(){
  while (!forkable_stack_empty(&data_stk) && 
         forkable_stack_pop_will_free(&data_stk)) {
    jv_free(stack_pop().value);
  }
  while (!forkable_stack_empty(&frame_stk) && 
         forkable_stack_pop_will_free(&frame_stk)) {
    frame_pop(&frame_stk);
  }
  if (forkable_stack_empty(&fork_stk)) {
    return 0;
  } else {
    struct forkpoint* fork = forkable_stack_peek(&fork_stk);
    forkable_stack_restore(&data_stk, &fork->saved_data_stack);
    forkable_stack_restore(&frame_stk, &fork->saved_call_stack);
    forkable_stack_pop(&fork_stk);
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

jv jq_next() {
  jv cfunc_input[MAX_CFUNCTION_ARGS];

  assert(!forkable_stack_empty(&frame_stk));
  uint16_t* pc = *frame_current_retaddr(&frame_stk);
  frame_pop(&frame_stk);
  
  assert(!forkable_stack_empty(&frame_stk));

  int backtracking = 0;
  while (1) {
    uint16_t opcode = *pc;

#if JQ_DEBUG
    dump_operation(frame_current_bytecode(&frame_stk), pc);
    printf("\t");
    const struct opcode_description* opdesc = opcode_describe(opcode);
    data_stk_elem* param;
    int stack_in = opdesc->stack_in;
    if (stack_in == -1) stack_in = pc[1];
    for (int i=0; i<stack_in; i++) {
      if (i == 0) {
        param = forkable_stack_peek(&data_stk);
      } else {
        printf(" | ");
        param = forkable_stack_peek_next(&data_stk, param);
      }
      if (!param) break;
      jv_dump(jv_copy(param->sv.value), 0);
      printf("<%d>", jv_get_refcnt(param->sv.value));
    }

    if (backtracking) printf("\t<backtracking>");

    printf("\n");
#endif
    if (backtracking) {
      opcode = ON_BACKTRACK(opcode);
      backtracking = 0;
    }
    pc++;

    switch (opcode) {
    default: assert(0 && "invalid instruction");

    case LOADK: {
      jv v = jv_array_get(jv_copy(frame_current_bytecode(&frame_stk)->constants), *pc++);
      assert(jv_is_valid(v));
      stack_push(stackval_replace(stack_pop(), v));
      break;
    }

    case DUP: {
      stackval v = stack_pop();
      stackval v2 = v;
      v2.value = jv_copy(v.value);
      stack_push(v);
      stack_push(v2);
      break;
    }

    case DUP2: {
      stackval keep = stack_pop();
      stackval v = stack_pop();
      stackval v2 = v;
      v2.value = jv_copy(v.value);
      stack_push(v);
      stack_push(keep);
      stack_push(v2);
      break;
    }

    case SWAP: {
      stackval a = stack_pop();
      stackval b = stack_pop();
      stack_push(a);
      stack_push(b);
      break;
    }
      
    case POP: {
      jv_free(stack_pop().value);
      break;
    }

    case APPEND: {
      // FIXME paths
      jv v = stack_pop().value;
      uint16_t level = *pc++;
      uint16_t vidx = *pc++;
      frame_ptr fp = frame_get_level(&frame_stk, frame_current(&frame_stk), level);
      jv* var = frame_local_var(fp, vidx);
      assert(jv_get_kind(*var) == JV_KIND_ARRAY);
      *var = jv_array_append(*var, v);
      break;
    }

    case INSERT: {
      stackval stktop = stack_pop();
      jv v = stack_pop().value;
      jv k = stack_pop().value;
      stackval objv = stack_pop();
      assert(jv_get_kind(k) == JV_KIND_STRING);
      assert(jv_get_kind(objv.value) == JV_KIND_OBJECT);
      objv.value = jv_object_set(objv.value, k, v);
      stack_push(objv);
      stack_push(stktop);
      break;
    }

      // FIXME: loadv/storev may do too much copying/freeing
    case LOADV: {
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      frame_ptr fp = frame_get_level(&frame_stk, frame_current(&frame_stk), level);
      jv* var = frame_local_var(fp, v);
      #if JQ_DEBUG
      printf("V%d = ", v);
      jv_dump(jv_copy(*var), 0);
      printf("\n");
      #endif
      stack_push(stackval_replace(stack_pop(), jv_copy(*var)));
      break;
    }

    case STOREV: {
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      frame_ptr fp = frame_get_level(&frame_stk, frame_current(&frame_stk), level);
      jv* var = frame_local_var(fp, v);
      stackval val = stack_pop();
      #if JQ_DEBUG
      printf("V%d = ", v);
      jv_dump(jv_copy(val.value), 0);
      printf("\n");
      #endif
      jv_free(*var);
      *var = val.value;
      break;
    }

    case GETPATH: {
      stackval path_end = stack_pop();
      stackval path_start = stack_pop();
      jv_free(path_end.value);
      jv path = jv_array();
      for (int i=path_start.pathidx; i<path_end.pathidx; i++) {
        path = jv_array_set(path, i, jv_copy(pathbuf[i]));
      }
      stack_push(stackval_replace(path_start, path));
      break;
    }

    case INDEX: {
      stackval t = stack_pop();
      jv k = stack_pop().value;
      int pathidx = path_push(t, jv_copy(k));
      jv v = jv_get(t.value, k);
      if (jv_is_valid(v)) {
        stackval sv;
        sv.value = v;
        sv.pathidx = pathidx;
        stack_push(sv);
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
      stackval t = stack_pop();
      jv_kind kind = jv_get_kind(t.value);
      if (kind == JV_KIND_FALSE || kind == JV_KIND_NULL) {
        pc += offset;
      }
      stack_push(t); // FIXME do this better
      break;
    }

    case EACH: 
      stack_push(stackval_root(jv_number(-1)));
      // fallthrough
    case ON_BACKTRACK(EACH): {
      int idx = jv_number_value(stack_pop().value);
      stackval container = stack_pop();

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
        stack_save();
        stack_push(container);
        stack_push(stackval_root(jv_number(idx)));
        frame_push_backtrack(&frame_stk, pc - 1);
        stack_switch();
        
        stackval sv = {value,
                       path_push(container, key)};
        stack_push(sv);
      }
      break;
    }

    do_backtrack:
    case BACKTRACK: {
      if (!stack_restore()) {
        return jv_invalid();
      }
      pc = *frame_current_retaddr(&frame_stk);
      frame_pop(&frame_stk);
      backtracking = 1;
      break;
    }

    case FORK: {
      stack_save();
      frame_push_backtrack(&frame_stk, pc - 1);
      stack_switch();
      pc++; // skip offset this time
      break;
    }

    case ON_BACKTRACK(FORK): {
      uint16_t offset = *pc++;
      pc += offset;
      break;
    }

    case YIELD: {
      jv value = stack_pop().value;
      frame_push_backtrack(&frame_stk, pc);
      return value;
    }
      
    case CALL_BUILTIN: {
      int nargs = *pc++;
      stackval top = stack_pop();
      cfunc_input[0] = top.value;
      for (int i = 1; i < nargs; i++) {
        cfunc_input[i] = stack_pop().value;
      }
      struct cfunction* func = &frame_current_bytecode(&frame_stk)->globals->cfunctions[*pc++];
      top.value = cfunction_invoke(func, cfunc_input);
      if (jv_is_valid(top.value)) {
        stack_push(top);
      } else {
        print_error(top.value);
        goto do_backtrack;
      }
      break;
    }

    case CALL_JQ: {
      uint16_t nclosures = *pc++;
      uint16_t* retaddr = pc + 2 + nclosures*2;
      frame_ptr new_frame = frame_push(&frame_stk, 
                                       make_closure(&frame_stk, frame_current(&frame_stk), pc),
                                       retaddr);
      pc += 2;
      frame_ptr old_frame = forkable_stack_peek_next(&frame_stk, new_frame);
      assert(nclosures == frame_self(new_frame)->bc->nclosures);
      for (int i=0; i<nclosures; i++) {
        *frame_closure_arg(new_frame, i) = make_closure(&frame_stk, old_frame, pc);
        pc += 2;
      }

      pc = frame_current_bytecode(&frame_stk)->code;
      break;
    }

    case RET: {
      pc = *frame_current_retaddr(&frame_stk);
      frame_pop(&frame_stk);
      break;
    }
    }
  }
}


void jq_init(struct bytecode* bc, jv input) {
  forkable_stack_init(&data_stk, sizeof(stackval) * 100);
  forkable_stack_init(&frame_stk, 1024);
  forkable_stack_init(&fork_stk, 1024);
  
  stack_push(stackval_root(input));
  struct closure top = {bc, -1};
  frame_push(&frame_stk, top, 0);
  frame_push_backtrack(&frame_stk, bc->code);
}

void jq_teardown() {
  while (stack_restore()) {}

  assert(forkable_stack_empty(&fork_stk));
  assert(forkable_stack_empty(&data_stk));
  assert(forkable_stack_empty(&frame_stk));
  forkable_stack_free(&fork_stk);
  forkable_stack_free(&data_stk);
  forkable_stack_free(&frame_stk);

  for (int i=0; i<pathsize; i++) {
    jv_free(pathbuf[i]);
  }
  jv_mem_free(pathbuf);
  pathbuf = 0;
  pathsize = 0;
}

struct bytecode* jq_compile(const char* str) {
  struct locfile locations;
  locfile_init(&locations, str, strlen(str));
  block program;
  struct bytecode* bc = 0;
  int nerrors = jq_parse(&locations, &program);
  if (nerrors == 0) {
    program = builtins_bind(program);
    nerrors = block_compile(program, &locations, &bc);
  }
  if (nerrors) {
    fprintf(stderr, "%d compile %s\n", nerrors, nerrors > 1 ? "errors" : "error");
  }
  locfile_free(&locations);
  return bc;
}
