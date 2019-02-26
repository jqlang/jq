#ifndef COMPILE_H
#define COMPILE_H
#include <stdint.h>
#include "jv.h"
#include "bytecode.h"
#include "locfile.h"

struct inst;
typedef struct inst inst;


typedef struct block {
  inst* first;
  inst* last;
} block;

block gen_location(location, struct locfile*, block);

block gen_noop();
int block_is_noop(block b);
block gen_op_simple(opcode op);
block gen_const(jv constant);
block gen_const_global(jv constant, const char *name);
int block_is_const(block b);
int block_is_const_inf(block b);
jv_kind block_const_kind(block b);
jv block_const(block b);
block gen_op_target(opcode op, block target);
block gen_op_unbound(opcode op, const char* name);
block gen_op_bound(opcode op, block binder);
block gen_op_var_fresh(opcode op, const char* name);
block gen_op_pushk_under(jv constant);

block gen_module(block metadata);
jv block_module_meta(block b);
block gen_import(const char* name, const char *as, int is_data);
block gen_import_meta(block import, block metadata);
block gen_function(const char* name, block formals, block body);
block gen_param_regular(const char* name);
block gen_param(const char* name);
block gen_lambda(block body);
block gen_call(const char* name, block body);
block gen_subexp(block a);
block gen_both(block a, block b);
block gen_const_object(block expr);
block gen_collect(block expr);
block gen_reduce(block source, block matcher, block init, block body);
block gen_foreach(block source, block matcher, block init, block update, block extract);
block gen_definedor(block a, block b);
block gen_condbranch(block iftrue, block iffalse);
block gen_and(block a, block b);
block gen_or(block a, block b);
block gen_dictpair(block k, block v);

block gen_var_binding(block var, const char* name, block body);
block gen_array_matcher(block left, block curr);
block gen_object_matcher(block name, block curr);
block gen_destructure(block var, block matcher, block body);
block gen_destructure_alt(block matcher);

block gen_cond(block cond, block iftrue, block iffalse);
block gen_try_handler(block handler);
block gen_try(block exp, block handler);
block gen_label(const char *label, block exp);

block gen_cbinding(const struct cfunction* functions, int nfunctions, block b);

void block_append(block* b, block b2);
block block_join(block a, block b);
int block_has_only_binders_and_imports(block, int bindflags);
int block_has_only_binders(block, int bindflags);
int block_has_main(block);
int block_is_funcdef(block b);
int block_is_single(block b);
block block_bind_library(block binder, block body, int bindflags, const char* libname);
block block_bind_referenced(block binder, block body, int bindflags);
block block_bind_self(block binder, int bindflags);
block block_drop_unreferenced(block body);

jv block_take_imports(block* body);
jv block_list_funcs(block body, int omit_underscores);

int block_compile(block, struct bytecode**, struct locfile*, jv);

void block_free(block);



// Here's some horrible preprocessor gunk so that code
// sequences can be contructed as BLOCK(block1, block2, block3)

#define BLOCK_1(b1) (b1)
#define BLOCK_2(b1,b2) (block_join((b1),(b2)))
#define BLOCK_3(b1,b2,b3) (block_join(BLOCK_2(b1,b2),(b3)))
#define BLOCK_4(b1,b2,b3,b4) (block_join(BLOCK_3(b1,b2,b3),(b4)))
#define BLOCK_5(b1,b2,b3,b4,b5) (block_join(BLOCK_4(b1,b2,b3,b4),(b5)))
#define BLOCK_6(b1,b2,b3,b4,b5,b6) (block_join(BLOCK_5(b1,b2,b3,b4,b5),(b6)))
#define BLOCK_7(b1,b2,b3,b4,b5,b6,b7) (block_join(BLOCK_6(b1,b2,b3,b4,b5,b6),(b7)))
#define BLOCK_8(b1,b2,b3,b4,b5,b6,b7,b8) (block_join(BLOCK_7(b1,b2,b3,b4,b5,b6,b7),(b8)))

#define BLOCK_IDX(_1,_2,_3,_4,_5,_6,_7,_8,NAME,...) NAME
#define BLOCK(...) \
  BLOCK_IDX(__VA_ARGS__, BLOCK_8, BLOCK_7, BLOCK_6, BLOCK_5, BLOCK_4, BLOCK_3, BLOCK_2, BLOCK_1)(__VA_ARGS__)


#endif
