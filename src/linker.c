#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <libgen.h>

#ifdef WIN32
#include <shlwapi.h>
#endif

#include "jq_parser.h"
#include "locfile.h"
#include "jv.h"
#include "jq.h"
#include "parser.h"
#include "util.h"
#include "compile.h"
#include "jv_alloc.h"

struct lib_loading_state {
  char **names;
  block *defs;
  uint64_t ct;
};
static int load_library(jq_state *jq, jv lib_path,
                        int is_data, int raw, int optional,
                        const char *as, block *out_block,
                        struct lib_loading_state *lib_state);

static int path_is_relative(jv p) {
  const char *s = jv_string_value(p);

#ifdef WIN32
  int res = PathIsRelativeA(s);
#else
  int res = *s != '/';
#endif
  jv_free(p);
  return res;
}


// Given a lib_path to search first, creates a chain of search paths
// in the following order:
// 1. lib_path
// 2. -L paths passed in on the command line (from jq_state*) or builtin list
static jv build_lib_search_chain(jq_state *jq, jv search_path, jv jq_origin, jv lib_origin) {
  assert(jv_get_kind(search_path) == JV_KIND_ARRAY);
  jv expanded = jv_array();
  jv expanded_elt;
  jv err = jv_null();
  jv_array_foreach(search_path, i, path) {
    if (jv_get_kind(path) != JV_KIND_STRING) {
      jv_free(path);
      continue;
    }
    path = expand_path(path);
    if (!jv_is_valid(path)) {
      err = path;
      path = jv_null();
      continue;
    }
    if (strcmp(".",jv_string_value(path)) == 0) {
      expanded_elt = jv_copy(path);
    } else if (strncmp("$ORIGIN/",jv_string_value(path),sizeof("$ORIGIN/") - 1) == 0) {
      expanded_elt = jv_string_fmt("%s/%s",
                               jv_string_value(jq_origin),
                               jv_string_value(path) + sizeof ("$ORIGIN/") - 1);
    } else if (jv_get_kind(lib_origin) == JV_KIND_STRING &&
               path_is_relative(jv_copy(path))) {
      expanded_elt = jv_string_fmt("%s/%s",
                               jv_string_value(lib_origin),
                               jv_string_value(path));
    } else {
      expanded_elt = path;
      path = jv_invalid();
    }
    expanded = jv_array_append(expanded, expanded_elt);
    jv_free(path);
  }
  jv_free(jq_origin);
  jv_free(lib_origin);
  jv_free(search_path);
  return JV_ARRAY(expanded, err);
}

// Doesn't actually check that name not be an absolute path, and we
// don't have to: we always append relative paths to others (with a '/'
// in between).
static jv validate_relpath(jv name) {
  const char *s = jv_string_value(name);
  if (strchr(s, '\\')) {
    jv res = jv_invalid_with_msg(jv_string_fmt("Modules must be named by relative paths using '/', not '\\' (%s)", s));
    jv_free(name);
    return res;
  }
  jv components = jv_string_split(jv_copy(name), jv_string("/"));
  jv_array_foreach(components, i, x) {
    if (!strcmp(jv_string_value(x), "..")) {
      jv_free(x);
      jv_free(components);
      jv res = jv_invalid_with_msg(jv_string_fmt("Relative paths to modules may not traverse to parent directories (%s)", s));
      jv_free(name);
      return res;
    }
    if (i > 0 && jv_equal(jv_copy(x), jv_array_get(jv_copy(components), i - 1))) {
      jv_free(x);
      jv_free(components);
      jv res = jv_invalid_with_msg(jv_string_fmt("module names must not have equal consecutive components: %s",
                                                 jv_string_value(name)));
      jv_free(name);
      return res;
    }
    jv_free(x);
  }
  jv_free(components);
  return name;
}

// Assumes name has been validated
static jv jv_basename(jv name) {
  const char *s = jv_string_value(name);
  const char *p = strrchr(s, '/');
  if (!p)
    return name;
  jv res = jv_string_fmt("%s", p);
  jv_free(name);
  return res;
}

// Asummes validated relative path to module
static jv find_lib(jq_state *jq, jv rel_path, jv search, const char *suffix, jv jq_origin, jv lib_origin) {
  if (!jv_is_valid(rel_path)) {
    jv_free(search);
    jv_free(jq_origin);
    jv_free(lib_origin);
    return rel_path;
  }
  if (jv_get_kind(rel_path) != JV_KIND_STRING) {
    jv_free(rel_path);
    jv_free(search);
    jv_free(jq_origin);
    jv_free(lib_origin);
    return jv_invalid_with_msg(jv_string_fmt("Module path must be a string"));
  }
  if (jv_get_kind(search) != JV_KIND_ARRAY) {
    jv_free(rel_path);
    jv_free(search);
    jv_free(jq_origin);
    jv_free(lib_origin);
    return jv_invalid_with_msg(jv_string_fmt("Module search path must be an array"));
  }

  struct stat st;
  int ret;

  // Ideally we should cache this somewhere
  search = build_lib_search_chain(jq, search, jq_origin, lib_origin);
  jv err = jv_array_get(jv_copy(search), 1);
  search = jv_array_get(search, 0);

  jv bname = jv_basename(jv_copy(rel_path));

  jv_array_foreach(search, i, spath) {
    if (jv_get_kind(spath) == JV_KIND_NULL) {
      jv_free(spath);
      break;
    }
    if (jv_get_kind(spath) != JV_KIND_STRING ||
        strcmp(jv_string_value(spath), "") == 0) {
      jv_free(spath);
      continue; /* XXX report non-strings in search path?? */
    }
    // Try ${search_dir}/${rel_path}.jq
    jv testpath = jq_realpath(jv_string_fmt("%s/%s%s",
                                            jv_string_value(spath),
                                            jv_string_value(rel_path),
                                            suffix));
    ret = stat(jv_string_value(testpath),&st);
    if (ret == -1 && errno == ENOENT) {
      jv_free(testpath);
      // Try ${search_dir}/$(dirname ${rel_path})/jq/main.jq
      testpath = jq_realpath(jv_string_fmt("%s/%s/%s%s",
                                           jv_string_value(spath),
                                           jv_string_value(rel_path),
                                           "jq/main",
                                           suffix));
      ret = stat(jv_string_value(testpath),&st);
    }
    if (ret == -1 && errno == ENOENT) {
      jv_free(testpath);
      // Try ${search_dir}/${rel_path}/$(basename ${rel_path}).jq
      testpath = jq_realpath(jv_string_fmt("%s/%s/%s%s",
                                           jv_string_value(spath),
                                           jv_string_value(rel_path),
                                           jv_string_value(bname),
                                           suffix));
      ret = stat(jv_string_value(testpath),&st);
    }
    if (ret == 0) {
      jv_free(err);
      jv_free(rel_path);
      jv_free(search);
      jv_free(bname);
      jv_free(spath);
      return testpath;
    }
    jv_free(testpath);
    jv_free(spath);
  }
  jv output;
  if (!jv_is_valid(err)) {
    err = jv_invalid_get_msg(err);
    output = jv_invalid_with_msg(jv_string_fmt("module not found: %s (%s)",
                                               jv_string_value(rel_path),
                                               jv_string_value(err)));
  } else {
    output = jv_invalid_with_msg(jv_string_fmt("module not found: %s",
                                               jv_string_value(rel_path)));
  }
  jv_free(err);
  jv_free(rel_path);
  jv_free(search);
  jv_free(bname);
  return output;
}

static jv default_search(jq_state *jq, jv value) {
  if (!jv_is_valid(value)) {
    // dependent didn't say; prepend . to system search path listj
    jv_free(value);
    return jv_array_concat(JV_ARRAY(jv_string(".")), jq_get_lib_dirs(jq));
  }
  if (jv_get_kind(value) != JV_KIND_ARRAY)
    return JV_ARRAY(value);
  return value;
}

// XXX Split this into a util that takes a callback, and then...
static int process_dependencies(jq_state *jq, jv jq_origin, jv lib_origin, block *src_block, struct lib_loading_state *lib_state) {
  jv deps = block_take_imports(src_block);
  block bk = *src_block;
  int nerrors = 0;

  // XXX This is a backward jv_array_foreach because bindings go in reverse
  for (int i = jv_array_length(jv_copy(deps)); i > 0; ) {
    i--;
    jv dep = jv_array_get(jv_copy(deps), i);

    const char *as_str = NULL;
    int is_data = jv_get_kind(jv_object_get(jv_copy(dep), jv_string("is_data"))) == JV_KIND_TRUE;
    int raw = 0;
    jv v = jv_object_get(jv_copy(dep), jv_string("raw"));
    if (jv_get_kind(v) == JV_KIND_TRUE)
      raw = 1;
    int optional = 0;
    if (jv_get_kind(jv_object_get(jv_copy(dep), jv_string("optional"))) == JV_KIND_TRUE)
      optional = 1;
    jv_free(v);
    jv relpath = validate_relpath(jv_object_get(jv_copy(dep), jv_string("relpath")));
    jv as = jv_object_get(jv_copy(dep), jv_string("as"));
    assert(!jv_is_valid(as) || jv_get_kind(as) == JV_KIND_STRING);
    if (jv_get_kind(as) == JV_KIND_STRING)
      as_str = jv_string_value(as);
    jv search = default_search(jq, jv_object_get(dep, jv_string("search")));
    // dep is now freed; do not reuse

    // find_lib does a lot of work that could be cached...
    jv resolved = find_lib(jq, relpath, search, is_data ? ".json" : ".jq", jv_copy(jq_origin), jv_copy(lib_origin));
    // XXX ...move the rest of this into a callback.
    if (!jv_is_valid(resolved)) {
      jv_free(as);
      if (optional) {
        jv_free(resolved);
        continue;
      }
      jv emsg = jv_invalid_get_msg(resolved);
      jq_report_error(jq, jv_string_fmt("jq: error: %s\n",jv_string_value(emsg)));
      jv_free(emsg);
      jv_free(deps);
      jv_free(jq_origin);
      jv_free(lib_origin);
      return 1;
    }

    if (is_data) {
      // Can't reuse data libs because the wrong name is bound
      block dep_def_block;
      nerrors += load_library(jq, resolved, is_data, raw, optional, as_str, &dep_def_block, lib_state);
      if (nerrors == 0) {
        // Bind as both $data::data and $data for backward compatibility vs common sense
        bk = block_bind_library(dep_def_block, bk, OP_IS_CALL_PSEUDO, as_str);
        bk = block_bind_library(dep_def_block, bk, OP_IS_CALL_PSEUDO, NULL);
      }
    } else {
      uint64_t state_idx = 0;
      for (; state_idx < lib_state->ct; ++state_idx) {
        if (strcmp(lib_state->names[state_idx],jv_string_value(resolved)) == 0)
          break;
      }

      if (state_idx < lib_state->ct) { // Found
        jv_free(resolved);
        // Bind the library to the program
        bk = block_bind_library(lib_state->defs[state_idx], bk, OP_IS_CALL_PSEUDO, as_str);
      } else { // Not found.   Add it to the table before binding.
        block dep_def_block = gen_noop();
        nerrors += load_library(jq, resolved, is_data, raw, optional, as_str, &dep_def_block, lib_state);
        // resolved has been freed
        if (nerrors == 0) {
          // Bind the library to the program
          bk = block_bind_library(dep_def_block, bk, OP_IS_CALL_PSEUDO, as_str);
        }
      }
    }

    jv_free(as);
  }
  jv_free(lib_origin);
  jv_free(jq_origin);
  jv_free(deps);
  return nerrors;
}

// Loads the library at lib_path into lib_state, putting the library's defs
// into *out_block
static int load_library(jq_state *jq, jv lib_path, int is_data, int raw, int optional, const char *as, block *out_block, struct lib_loading_state *lib_state) {
  int nerrors = 0;
  struct locfile* src = NULL;
  block program;
  jv data;
  if (is_data && !raw)
    data = jv_load_file(jv_string_value(lib_path), 0);
  else
    data = jv_load_file(jv_string_value(lib_path), 1);
  int state_idx;
  if (!jv_is_valid(data)) {
    program = gen_noop();
    if (!optional) {
      if (jv_invalid_has_msg(jv_copy(data)))
        data = jv_invalid_get_msg(data);
      else
        data = jv_string("unknown error");
      jq_report_error(jq, jv_string_fmt("jq: error loading data file %s: %s\n", jv_string_value(lib_path), jv_string_value(data)));
      nerrors++;
    }
    goto out;
  } else if (is_data) {
    // import "foo" as $bar;
    program = gen_const_global(jv_copy(data), as);
  } else {
    // import "foo" as bar;
    src = locfile_init(jq, jv_string_value(lib_path), jv_string_value(data), jv_string_length_bytes(jv_copy(data)));
    nerrors += jq_parse_library(src, &program);
    locfile_free(src);
    if (nerrors == 0) {
      char *lib_origin = strdup(jv_string_value(lib_path));
      nerrors += process_dependencies(jq, jq_get_jq_origin(jq),
                                      jv_string(dirname(lib_origin)),
                                      &program, lib_state);
      free(lib_origin);
      program = block_bind_self(program, OP_IS_CALL_PSEUDO);
    }
  }
  state_idx = lib_state->ct++;
  lib_state->names = jv_mem_realloc(lib_state->names, lib_state->ct * sizeof(const char *));
  lib_state->defs = jv_mem_realloc(lib_state->defs, lib_state->ct * sizeof(block));
  lib_state->names[state_idx] = strdup(jv_string_value(lib_path));
  lib_state->defs[state_idx] = program;
out:
  *out_block = program;
  jv_free(lib_path);
  jv_free(data);
  return nerrors;
}

// FIXME It'd be nice to have an option to search the same search path
// as we do in process_dependencies.
jv load_module_meta(jq_state *jq, jv mod_relpath) {
  // We can't know the caller's origin; we could though, if it was passed in
  jv lib_path = find_lib(jq, validate_relpath(mod_relpath), jq_get_lib_dirs(jq), ".jq", jq_get_jq_origin(jq), jv_null());
  if (!jv_is_valid(lib_path))
    return lib_path;
  jv meta = jv_null();
  jv data = jv_load_file(jv_string_value(lib_path), 1);
  if (jv_is_valid(data)) {
    block program;
    struct locfile* src = locfile_init(jq, jv_string_value(lib_path), jv_string_value(data), jv_string_length_bytes(jv_copy(data)));
    int nerrors = jq_parse_library(src, &program);
    if (nerrors == 0) {
      meta = block_module_meta(program);
      if (jv_get_kind(meta) == JV_KIND_NULL)
        meta = jv_object();
      meta = jv_object_set(meta, jv_string("deps"), block_take_imports(&program));
      meta = jv_object_set(meta, jv_string("defs"), block_list_funcs(program, 0));
    }
    locfile_free(src);
    block_free(program);
  }
  jv_free(lib_path);
  jv_free(data);
  return meta;
}

int load_program(jq_state *jq, struct locfile* src, block *out_block) {
  int nerrors = 0;
  block program;
  struct lib_loading_state lib_state = {0,0,0};
  nerrors = jq_parse(src, &program);
  if (nerrors)
    return nerrors;

  if (!block_has_main(program)) {
    jq_report_error(jq, jv_string("jq: error: Top-level program not given (try \".\")"));
    block_free(program);
    return 1;
  }

  jv home = get_home();
  if (jv_is_valid(home)) {
    /* Import ~/.jq as a library named "" found in $HOME or %USERPROFILE% */
    block import = gen_import_meta(gen_import("", NULL, 0),
        gen_const(JV_OBJECT(
            jv_string("optional"), jv_true(),
            jv_string("search"), home)));
    program = BLOCK(import, program);
  } else {    // silently ignore if home dir not determined
    jv_free(home);
  }

  nerrors = process_dependencies(jq, jq_get_jq_origin(jq), jq_get_prog_origin(jq), &program, &lib_state);
  block libs = gen_noop();
  for (uint64_t i = 0; i < lib_state.ct; ++i) {
    free(lib_state.names[i]);
    if (nerrors == 0 && !block_is_const(lib_state.defs[i]))
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
