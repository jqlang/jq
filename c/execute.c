#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "opcode.h"
#include "bytecode.h"
#include "compile.h"

#include "forkable_stack.h"
#include "frame_layout.h"

#include "jv.h"


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
    pathbuf = realloc(pathbuf, sizeof(pathbuf[0]) * pathsize);
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


#define ON_BACKTRACK(op) ((op)+NUM_OPCODES)

jv jq_next() {
  jv cfunc_input[MAX_CFUNCTION_ARGS];
  jv cfunc_output[MAX_CFUNCTION_ARGS];

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
    for (int i=0; i<opdesc->stack_in; i++) {
      if (i == 0) {
        param = forkable_stack_peek(&data_stk);
      } else {
        printf(" | ");
        param = forkable_stack_peek_next(&data_stk, param);
      }
      if (!param) break;
      jv_dump(jv_copy(param->sv.value));
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
      jv array = stack_pop().value;
      array = jv_array_append(array, v);
      stack_push(stackval_root(array));
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
      jv_dump(jv_copy(*var));
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
      jv_dump(jv_copy(val.value));
      printf("\n");
      #endif
      jv_free(*var);
      *var = val.value;
      break;
    }

    case ASSIGN: {
      stackval replacement = stack_pop();
      stackval path_end = stack_pop();
      stackval path_start = stack_pop();
      jv_free(path_end.value);
      jv_free(path_start.value);

      uint16_t level = *pc++;
      uint16_t v = *pc++;
      frame_ptr fp = frame_get_level(&frame_stk, frame_current(&frame_stk), level);
      jv* var = frame_local_var(fp, v);
      *var = jv_insert(*var, replacement.value, pathbuf + path_start.pathidx, path_end.pathidx - path_start.pathidx);
      break;
    }

    case INDEX: {
      stackval t = stack_pop();
      jv k = stack_pop().value;
      int pathidx = path_push(t, jv_copy(k));
      jv v = jv_lookup(t.value, k);
      if (1 /* fixme invalid lookups */) {
        stackval sv;
        sv.value = v;
        sv.pathidx = pathidx;
        stack_push(sv);
      } else {
        assert(0 && "bad lookup");
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
      stack_push(stackval_root(jv_number(0)));
      // fallthrough
    case ON_BACKTRACK(EACH): {
      int idx = jv_number_value(stack_pop().value);
      stackval array = stack_pop();
      if (idx >= jv_array_length(jv_copy(array.value))) {
        jv_free(array.value);
        goto do_backtrack;
      } else {
        stack_save();
        stackval array2 = array;
        array2.value = jv_copy(array2.value);
        stack_push(array2);
        stack_push(stackval_root(jv_number(idx+1)));
        frame_push_backtrack(&frame_stk, pc - 1);
        stack_switch();
        
        stackval sv = {jv_array_get(array.value, idx), 
                       path_push(array, jv_number(idx))};
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
      
    case CALL_BUILTIN_1_1: {
      assert(*pc == 1); // no closure args allowed
      pc++; // skip nclosures
      pc++; // skip level
      stackval top = stack_pop();
      cfunc_input[0] = top.value;
      struct cfunction* func = &frame_current_bytecode(&frame_stk)->globals->cfunctions[*pc++];
      func->fptr(cfunc_input, cfunc_output);
      top.value = cfunc_output[0];
      stack_push(top);
      break;
    }

    case CALL_BUILTIN_3_1: {
      assert(*pc == 1); // no closure args allowed
      pc++; // skip nclosures
      pc++; // skip level
      stackval top = stack_pop();
      jv a = stack_pop().value;
      jv b = stack_pop().value;
      cfunc_input[0] = top.value;
      cfunc_input[1] = a;
      cfunc_input[2] = b;
      struct cfunction* func = &frame_current_bytecode(&frame_stk)->globals->cfunctions[*pc++];
      func->fptr(cfunc_input, cfunc_output);
      top.value = cfunc_output[0];
      stack_push(top);
      break;
    }

    case CALL_1_1: {
      uint16_t nclosures = *pc++;
      frame_ptr new_frame = frame_push(&frame_stk, 
                                       make_closure(&frame_stk, frame_current(&frame_stk), pc),
                                       pc + nclosures * 2);
      pc += 2;
      frame_ptr old_frame = forkable_stack_peek_next(&frame_stk, new_frame);
      assert(nclosures - 1 == frame_self(new_frame)->bc->nclosures);
      for (int i=0; i<nclosures-1; i++) {
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
  forkable_stack_init(&data_stk, sizeof(stackval) * 1000); // FIXME: lower this number, see if it breaks
  forkable_stack_init(&frame_stk, 10240); // FIXME: lower this number, see if it breaks
  forkable_stack_init(&fork_stk, 10240); // FIXME: lower this number, see if it breaks
  
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
  free(pathbuf);
  pathbuf = 0;
  pathsize = 0;
}

void run_program(struct bytecode* bc) {
#if JQ_DEBUG
  dump_disassembly(0, bc);
  printf("\n");
#endif
  char buf[409600];
  fgets(buf, sizeof(buf), stdin);
  jq_init(bc, jv_parse(buf));
  jv result;
  while (jv_is_valid(result = jq_next())) {
    jv_dump(result);
    printf("\n");
  }
  #if JQ_DEBUG
  printf("end of results\n");
  #endif
  jq_teardown();
}
