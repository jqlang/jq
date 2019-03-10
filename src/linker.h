#ifndef LINKER_H
#define LINKER_H

#include "compile.h"

#ifdef WIN32
#define JQ_PATH_SEP "\\"
#define JQ_DLL_EXT ".dll"
#else
#define JQ_PATH_SEP "/"
#define JQ_DLL_EXT ".so"
#endif

struct lib_loading_state;
int load_program(jq_state *jq, struct locfile* src, block *out_block, struct lib_loading_state **libs);
void libraries_free(struct lib_loading_state *);
jv load_module_meta(jq_state *jq, jv modname);

#endif
