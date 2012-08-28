#include <jansson.h>
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
  json_t* value;
  int pathidx;
} stackval;


json_t** pathbuf;
int pathsize; // number of allocated elements


int path_push(stackval sv, json_t* val) {
  int pos = sv.pathidx;
  assert(pos <= pathsize);
  assert(pos >= 0);
  if (pos == pathsize) {
    pathsize = pathsize ? pathsize * 2 : 100;
    pathbuf = realloc(pathbuf, sizeof(pathbuf[0]) * pathsize);
  }
  pathbuf[pos] = val;
  return pos + 1;
}

stackval stackval_replace(stackval value, json_t* newjs) {
  stackval s = {newjs, value.pathidx};
  return s;
}


// Probably all uses of this function are bugs
stackval stackval_root(json_t* v) {
  stackval s = {v, 0};
  return s;
}

struct forkable_stack data_stk;
typedef struct {
  FORKABLE_STACK_HEADER;
  stackval sv;
} data_stk_elem;

void stk_push(stackval val) {
  data_stk_elem* s = forkable_stack_push(&data_stk, sizeof(data_stk_elem));
  s->sv = val;
}

stackval stk_pop() {
  data_stk_elem* s = forkable_stack_peek(&data_stk);
  stackval sv = s->sv;
  forkable_stack_pop(&data_stk);
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

void stack_restore(){
  struct forkpoint* fork = forkable_stack_peek(&fork_stk);
  forkable_stack_restore(&data_stk, &fork->saved_data_stack);
  forkable_stack_restore(&frame_stk, &fork->saved_call_stack);
  forkable_stack_pop(&fork_stk);
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


#define stack_push stk_push
#define stack_pop stk_pop

#define ON_BACKTRACK(op) ((op)+NUM_OPCODES)

json_t* jq_next() {
  json_t* cfunc_input[MAX_CFUNCTION_ARGS] = {0};
  json_t* cfunc_output[MAX_CFUNCTION_ARGS] = {0};

  assert(!forkable_stack_empty(&frame_stk));
  uint16_t* pc = *frame_current_retaddr(&frame_stk);
  frame_pop(&frame_stk);
  
  assert(!forkable_stack_empty(&frame_stk));

  int backtracking = 0;
  while (1) {
    dump_operation(frame_current_bytecode(&frame_stk), pc);

    uint16_t opcode = *pc++;

    printf("\t");
    const struct opcode_description* opdesc = opcode_describe(opcode);
    data_stk_elem* param;
    for (int i=0; i<opdesc->stack_in; i++) {
      if (i == 0) {
        param = forkable_stack_peek(&data_stk);
      } else {
        param = forkable_stack_peek_next(&data_stk, param);
      }
      json_dumpf(param->sv.value, stdout, JSON_ENCODE_ANY);
      if (i < opdesc->stack_in-1) printf(" | ");
    }

    if (backtracking) {
      printf("\t<backtracking>");
      opcode = ON_BACKTRACK(opcode);
      backtracking = 0;
    }

    printf("\n");

    switch (opcode) {
    default: assert(0 && "invalid instruction");

    case LOADK: {
      json_t* v = json_array_get(frame_current_bytecode(&frame_stk)->constants, *pc++);
      assert(v);
      stack_push(stackval_replace(stack_pop(), v));
      break;
    }

    case DUP: {
      stackval v = stack_pop();
      stack_push(v);
      stack_push(v);
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
      stack_pop();
      break;
    }

    case APPEND: {
      // FIXME paths
      json_t* v = stack_pop().value;
      json_t* array = stack_pop().value;
      array = json_copy(array);
      json_array_append(array, v);
      stack_push(stackval_root(array));
      break;
    }

    case INSERT: {
      stackval stktop = stack_pop();
      json_t* v = stack_pop().value;
      json_t* k = stack_pop().value;
      stackval objv = stack_pop();
      assert(json_is_string(k));
      assert(json_is_object(objv.value));
      json_t* obj = json_copy(objv.value);
      json_object_set(obj, json_string_value(k), v);
      assert(json_is_object(obj));
      stack_push(stackval_replace(objv, obj));
      stack_push(stktop);
      break;
    }

    case LOADV: {
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      frame_ptr fp = frame_get_level(&frame_stk, frame_current(&frame_stk), level);
      json_t** var = frame_local_var(fp, v);
      printf("V%d = ", v);
      json_dumpf(*var, stdout, JSON_ENCODE_ANY);
      printf("\n");
      stack_push(stackval_replace(stack_pop(), *var));
      break;
    }

    case STOREV: {
      uint16_t level = *pc++;
      uint16_t v = *pc++;
      frame_ptr fp = frame_get_level(&frame_stk, frame_current(&frame_stk), level);
      json_t** var = frame_local_var(fp, v);
      stackval val = stack_pop();
      printf("V%d = ", v);
      json_dumpf(val.value, stdout, JSON_ENCODE_ANY);
      printf("\n");
      *var = val.value;
      break;
    }

#if 0
    case DISPLAY: {
      stackval sv = stack_pop();
      if (sv.value) {
        json_dumpf(sv.value, stdout, JSON_ENCODE_ANY);
      } else {
        printf("#ERROR");
      }
      printf(" - ");
      for (int i = 0; i < sv.pathidx; i++) {
        printf("/");
        json_dumpf(pathbuf[i], stdout, JSON_ENCODE_ANY);
      }
      printf("\n");
      return;
    }
#endif

    case ASSIGN_DBG: {
      stackval replacement = stack_pop();
      stackval path_end = stack_pop();
      stackval path_start = stack_pop();
      json_t* obj = jv_insert(path_start.value, replacement.value, pathbuf + path_start.pathidx, path_end.pathidx - path_start.pathidx);
      stack_push(stackval_replace(path_start, obj));
      break;
    }

    case ASSIGN: {
      stackval replacement = stack_pop();
      stackval path_end = stack_pop();
      stackval path_start = stack_pop();

      uint16_t level = *pc++;
      uint16_t v = *pc++;
      frame_ptr fp = frame_get_level(&frame_stk, frame_current(&frame_stk), level);
      json_t** var = frame_local_var(fp, v);
      *var = jv_insert(*var, replacement.value, pathbuf + path_start.pathidx, path_end.pathidx - path_start.pathidx);
      break;
    }

    case INDEX: {
      stackval t = stack_pop();
      json_t* k = stack_pop().value;
      json_t* v = jv_lookup(t.value, k);
      if (v) {
        stackval sv;
        sv.value = v;
        sv.pathidx = path_push(t, k);
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

    case EACH: 
      stack_push(stackval_root(json_integer(0)));
      // fallthrough
    case ON_BACKTRACK(EACH): {
      json_t* idxj = stack_pop().value;
      int idx = json_integer_value(idxj);
      stackval array = stack_pop();
      if (idx >= json_array_size(array.value)) {
        goto do_backtrack;
      } else {
        stack_save();
        stack_push(array);
        stack_push(stackval_root(json_integer(idx+1)));
        frame_push_backtrack(&frame_stk, pc - 1);
        stack_switch();
        
        stackval sv = {json_array_get(array.value, idx), 
                       path_push(array, json_integer(idx))};
        stack_push(sv);
      }
      break;
    }

    do_backtrack:
    case BACKTRACK: {
      if (forkable_stack_empty(&fork_stk)) {
        return 0;
      }
      stack_restore();
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
      json_t* value = stack_pop().value;
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
      printf(" call %s\n", func->name);
      func->fptr(cfunc_input, cfunc_output);
      stack_push(stackval_replace(top, cfunc_output[0]));
      break;
    }

    case CALL_BUILTIN_3_1: {
      assert(*pc == 1); // no closure args allowed
      pc++; // skip nclosures
      pc++; // skip level
      stackval top = stack_pop();
      json_t* a = stack_pop().value;
      json_t* b = stack_pop().value;
      cfunc_input[0] = top.value;
      cfunc_input[1] = a;
      cfunc_input[2] = b;
      struct cfunction* func = &frame_current_bytecode(&frame_stk)->globals->cfunctions[*pc++];
      printf(" call %s\n", func->name);
      func->fptr(cfunc_input, cfunc_output);
      stack_push(stackval_replace(top, cfunc_output[0]));
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


void jq_init(struct bytecode* bc, json_t* input) {
  forkable_stack_init(&data_stk, sizeof(stackval) * 1000); // FIXME: lower this number, see if it breaks
  forkable_stack_init(&frame_stk, 10240); // FIXME: lower this number, see if it breaks
  forkable_stack_init(&fork_stk, 10240); // FIXME: lower this number, see if it breaks
  
  stack_push(stackval_root(input));
  struct closure top = {bc, -1};
  frame_push(&frame_stk, top, 0);
  frame_push_backtrack(&frame_stk, bc->code);
}

void run_program(struct bytecode* bc) {
  jq_init(bc, json_loadf(stdin, 0, 0));
  json_t* result;
  while ((result = jq_next())) {
    json_dumpf(result, stdout, JSON_ENCODE_ANY);
    printf("\n");
  }
  printf("end of results\n");

  //assert(frame == stack_top_frame(bc->framesize));
  //stk_pop_frame(bc->framesize);
  //assert(stackpos == 0);
}
