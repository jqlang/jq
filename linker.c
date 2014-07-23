#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <libgen.h>

#include "jq_parser.h"
#include "locfile.h"
#include "jv.h"
#include "jq.h"
#include "parser.h"
#include "util.h"
#include "compile.h"

struct lib_loading_state {
  char **names;
  block *defs;
  uint64_t ct;
};
static int load_library(jq_state *jq, jv lib_path, block *out_block, struct lib_loading_state *lib_state);

// Given a lib_path to search first, creates a chain of search paths
// in the following order:
// 1. lib_path
// 2. -L paths passed in on the command line (from jq_state*)
// 3. JQ_LIBRARY_PATH environment variable
jv build_lib_search_chain(jq_state *jq, jv lib_path) {
  assert(jv_get_kind(lib_path) == JV_KIND_STRING);

  jv out_paths = jv_array();
  if (jv_string_length_bytes(jv_copy(lib_path)))
    out_paths = jv_array_append(out_paths, lib_path);
  else
    jv_free(lib_path);
  jv lib_dirs = jq_get_lib_dirs(jq);
  jv_array_foreach(lib_dirs, i, path) {
    if (jv_string_length_bytes(jv_copy(path)) == 0)  {
      jv_free(path);
      continue;
    }
    path = expand_path(path);
    if (jv_is_valid(path)) {
      out_paths = jv_array_append(out_paths, path);
    } else {
      jv emsg = jv_invalid_get_msg(path);
      fprintf(stderr, "jq: warning: skipping search path: %s\n", jv_string_value(emsg));
      jv_free(emsg);
    } 
  }
  jv_free(lib_dirs);
  return out_paths;
}

static jv find_lib(jq_state *jq, jv lib_name, jv lib_search_path) {
  assert(jv_get_kind(lib_search_path) == JV_KIND_STRING);
  assert(jv_get_kind(lib_name) == JV_KIND_STRING);

  lib_search_path = expand_path(lib_search_path);

  struct stat st;
  int ret;

  jv lib_search_paths = build_lib_search_chain(jq, lib_search_path);

  jv_array_foreach(lib_search_paths, i, spath) {
    jv testpath = jq_realpath(jv_string_fmt("%s/%s.jq",jv_string_value(spath),jv_string_value(lib_name)));
    
    jv_free(spath);
    ret = stat(jv_string_value(testpath),&st);
    if (ret == 0) {
      jv_free(lib_name);
      jv_free(lib_search_paths);
      return testpath;
    }
    jv_free(testpath);
  }
  jv output = jv_invalid_with_msg(jv_string_fmt("could not find library: %s", jv_string_value(lib_name)));
  jv_free(lib_name);
  jv_free(lib_search_paths);
  return output;
}

static int process_dependencies(jq_state *jq, jv lib_origin, block *src_block, struct lib_loading_state *lib_state) {
  jv deps = block_take_imports(src_block);
  block bk = *src_block;
  int nerrors = 0;

  jv_array_foreach(deps, i, dep) {
    jv name = jv_object_get(jv_copy(dep), jv_string("name"));
    jv as = jv_object_get(jv_copy(dep), jv_string("as"));
    if (!jv_is_valid(as)) {
      jv_free(as);
      as = jv_copy(name);
    }
    jv search = jv_object_get(dep, jv_string("search"));
    if (!jv_is_valid(search)) {
      jv_free(search);
      search = jv_string("");
    }
    if (strncmp("$ORIGIN/",jv_string_value(search),8) == 0) {
      jv tsearch = jv_string_fmt("%s/%s",jv_string_value(lib_origin),jv_string_value(search)+8);
      jv_free(search);
      search = tsearch;
    }
    jv lib_path = find_lib(jq, name, search);
    if (!jv_is_valid(lib_path)) {
      jv emsg = jv_invalid_get_msg(lib_path);
      fprintf(stderr, "jq: error: %s\n",jv_string_value(emsg));
      jv_free(emsg);
      jv_free(lib_origin);
      jv_free(as);
      jv_free(deps);
      return 1;
    }
    uint64_t state_idx = 0;
    for (; state_idx < lib_state->ct; ++state_idx) {
      if (strcmp(lib_state->names[state_idx],jv_string_value(lib_path)) == 0)
        break;
    }
    if (state_idx < lib_state->ct) { // Found
      bk = block_bind_library(lib_state->defs[state_idx], bk, OP_IS_CALL_PSEUDO, jv_string_value(as));
      jv_free(lib_path);
    } else { // Not found.   Add it to the table before binding.
      block dep_def_block = gen_noop();
      nerrors += load_library(jq, lib_path, &dep_def_block, lib_state);
      if (nerrors == 0)
        bk = block_bind_library(dep_def_block, bk, OP_IS_CALL_PSEUDO, jv_string_value(as));
    }
    jv_free(as);
  }
  jv_free(lib_origin);
  jv_free(deps);
  return nerrors;
}

// Loads the library at lib_path into lib_state, putting the library's defs
// into *out_block
static int load_library(jq_state *jq, jv lib_path, block *out_block, struct lib_loading_state *lib_state) {
  int nerrors = 0;
  struct locfile* src;
  block program;
  jv data = jv_load_file(jv_string_value(lib_path), 1);
  int state_idx;
  if (jv_is_valid(data)) {
    src = locfile_init(jq, jv_string_value(data), jv_string_length_bytes(jv_copy(data)));
    nerrors += jq_parse_library(src, &program);
    if (nerrors == 0) {
      state_idx = lib_state->ct++;
      lib_state->names = realloc(lib_state->names, lib_state->ct * sizeof(const char *));
      lib_state->defs = realloc(lib_state->defs, lib_state->ct * sizeof(block));
      lib_state->names[state_idx] = strdup(jv_string_value(lib_path));
      lib_state->defs[state_idx] = program;
      char *lib_origin = strdup(jv_string_value(lib_path));
      nerrors += process_dependencies(jq, jv_string(dirname(lib_origin)), &lib_state->defs[state_idx], lib_state);
      free(lib_origin);
      *out_block = lib_state->defs[state_idx];
    }
    locfile_free(src);
  }
  jv_free(lib_path);
  jv_free(data);
  return nerrors;
}

int load_program(jq_state *jq, struct locfile* src, block *out_block) {
  int nerrors = 0;
  block program;
  struct lib_loading_state lib_state = {0,0,0};
  nerrors = jq_parse(src, &program);
  if (nerrors)
    return nerrors;

  nerrors = process_dependencies(jq, jq_get_lib_origin(jq), &program, &lib_state);
  block libs = gen_noop();
  for (uint64_t i = 0; i < lib_state.ct; ++i) {
    free(lib_state.names[i]);
    if (nerrors == 0)
      libs = block_join(libs, lib_state.defs[i]);
    else
      block_free(lib_state.defs[i]);
  }
  free(lib_state.names);
  free(lib_state.defs);
  if (nerrors)
    block_free(program);
  else
    *out_block = block_drop_unreferenced(block_join(libs, program));

  return nerrors;
}
