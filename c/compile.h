#ifndef COMPILE_H
#define COMPILE_H
#include <stdint.h>
#include "bytecode.h"
#include "opcode.h"
#include "locfile.h"

struct inst;
typedef struct inst inst;


typedef struct block {
  inst* first;
  inst* last;
} block;

block gen_location(location, block);

block gen_noop();
block gen_op_simple(opcode op);
block gen_op_const(opcode op, jv constant);
block gen_op_target(opcode op, block target);
block gen_op_var_unbound(opcode op, const char* name);
block gen_op_var_bound(opcode op, block binder);
block gen_op_block_defn(opcode op, const char* name, block block);
block gen_op_block_unbound(opcode op, const char* name);
block gen_op_call(opcode op, block arglist);
block gen_op_symbol(opcode op, const char* name);

block gen_subexp(block a);
block gen_both(block a, block b);
block gen_collect(block expr);
block gen_assign(block expr);
block gen_definedor(block a, block b);
block gen_condbranch(block iftrue, block iffalse);
block gen_and(block a, block b);
block gen_or(block a, block b);

block gen_cond(block cond, block iftrue, block iffalse);

block gen_cbinding(struct symbol_table* functions, block b);

void block_append(block* b, block b2);
block block_join(block a, block b);
block block_bind(block binder, block body, int bindflags);

int block_compile(block, struct locfile*, struct bytecode**);

void block_free(block);



#endif
