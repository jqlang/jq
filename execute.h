#ifndef EXECUTE_H
#define EXECUTE_H
#include "bytecode.h"


struct bytecode* jq_compile(const char* str);

void jq_init(struct bytecode* bc, jv value);
jv jq_next();
void jq_teardown();


#endif
