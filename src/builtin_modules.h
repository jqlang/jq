#ifndef JQ_BUILTIN_MODULES
#define JQ_BUILTIN_MODULES

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <libgen.h>

#include "jv.h"
#include "jq.h"

/* Builtin module "files" */
struct jq_builtin_module {
  const char *name;
  const char *contents;
  jq_plugin_init_f init;
};
extern struct jq_builtin_module *jq_builtin_modules;
extern size_t jq_builtin_nmodules;

#endif /* JQ_BUILTIN_MODULES */
