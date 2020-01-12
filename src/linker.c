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
#include "jq.h"
#include "linker.h"
#include "parser.h"
#include "util.h"
#include "builtin_modules.h"
#include "compile.h"
#include "jv_alloc.h"

#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#include <libloaderapi.h>

typedef union {
  HMODULE dl_handle;
  struct jq_builtin_module *bmodule;
} jq_dl_handle;

char *jq_dl_error() {
#ifdef _MSC_VER
  static __declspec(thread) char *s = 0;
#else
  static __thread char *s = 0;
#endif /* _MSC_VER */

  LocalFree(s);
  s = 0;
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                0, GetLastError(), 0, (LPTSTR) &s, 0, 0);
  return s;
}
#define JQ_DL_OPEN(name) LoadLibraryA(name)
#define JQ_DL_CLOSE(h) FreeLibrary(h)
#define JQ_DL_SYM(h, sym) GetProcAddress(h, sym)
#define JQ_DL_ERROR() jq_dl_error()

#elif defined(HAVE_DLOPEN)
#include <dlfcn.h>

#ifndef RTLD_GROUP
#define RTLD_GROUP 0
#endif

typedef union {
  void *dl_handle;
  struct jq_builtin_module *bmodule;
} jq_dl_handle;
#define JQ_DL_OPEN(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL | RTLD_GROUP)
#define JQ_DL_CLOSE(h) dlclose(h)
#define JQ_DL_SYM(h, sym) dlsym(h, sym)
#define JQ_DL_ERROR() dlerror()

#else

typedef union {
  void *dl_handle;
  struct jq_builtin_module *bmodule;
} jq_dl_handle;
#define JQ_DL_OPEN(name) (NULL)
#define JQ_DL_CLOSE(h)
#define JQ_DL_SYM(h, sym) (NULL)

#endif

typedef void *(*jq_dl_sym_f)(jq_dl_handle, const char *);
typedef void (*jq_dl_close_f)(jq_dl_handle);

struct jq_lib {
  jv jname;
  jv jbasename; /* basename() of jname */
  jv origin;    /* dirname()  of jname */
  jv contents;  /* jq code or JSON text */
  const char *name;   /* jv_string_value(jname) */
  block defs;   /* parsed library program */
  jq_dl_handle h;
  jq_dl_sym_f dl_sym;
  jq_dl_close_f dl_close;
  struct cfunction *cfuncs;
  size_t ncfuncs;
};
struct lib_loading_state {
  struct jq_lib *libs;
  size_t nlibs;
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

static jv load_file(jv path, int raw) {
  /*
   * For now.  Change this in 1.8.
   */
  jv contents = jv_load_file(jv_string_value(path), raw);
  if (strncmp(jv_string_value(path), "jq/", sizeof("jq/") - 1) != 0)
    return contents;

  if (jv_is_valid(contents)) {
    if (strncmp(jv_string_value(path), "jq/", sizeof("jq/") - 1) == 0)
      fprintf(stderr, "jq: warning: the module name prefix \"jq/\" is reserved for builtin modules\n");
    return contents;
  }

  contents = jv_invalid_has_msg(jv_copy(contents)) ?
              jv_invalid_get_msg(contents) : jv_string("");

  jv ret;
  for (size_t i = 0; i < jq_builtin_nmodules; i++) {
    if (strcmp(jq_builtin_modules[i].name, jv_string_value(path)))
      continue;
    if (!raw) {
      ret = jv_invalid_with_msg(jv_string_fmt("Builtin jq module %s is not JSON; %s",
                                              jv_string_value(path), jv_string_value(contents)));
      jv_free(contents);
      return ret;
    }
    jv_free(contents);
    return jv_string(jq_builtin_modules[i].contents);
  }
  ret = jv_invalid_with_msg(jv_string_fmt("No such builtin module %s; %s",
                                          jv_string_value(path), jv_string_value(contents)));
  jv_free(contents);
  return ret;
}

static void * dl_builtin_sym(jq_dl_handle h, const char *name) {
  struct jq_builtin_module *mod = h.bmodule;

  if (strcmp(name, "jq_plugin_init") == 0)
    return mod->init;
  return 0;
}

static void *jq_dl_sym(jq_dl_handle h, const char *name) {
  return JQ_DL_SYM(h.dl_handle, name);
}

static void jq_dl_close(jq_dl_handle h) {
  JQ_DL_CLOSE(h.dl_handle);
}

static jv jq_dl_open(const char *path, jq_dl_handle *h, jq_dl_sym_f *dl_sym, jq_dl_close_f *dl_close) {
  if (strncmp(path, "jq/", sizeof("jq/") - 1) == 0) {
    for (size_t i = 0; i < jq_builtin_nmodules; i++) {
      if (strcmp(jq_builtin_modules[i].name, path))
        continue;
      h->bmodule = &jq_builtin_modules[i];
      *dl_sym = dl_builtin_sym;
      *dl_close = 0;
      return jv_true();
    }
    h->bmodule = 0;
    *dl_sym = 0;
    *dl_close = 0;
    return jv_invalid_with_msg(jv_string("no such builtin module object"));
  }
  if (!(h->dl_handle = JQ_DL_OPEN(path))) {
    char *s = JQ_DL_ERROR();
    *dl_sym = 0;
    *dl_close = 0;
    return jv_invalid_with_msg(jv_string(s ? s : "<unknown error>"));
  }
  *dl_sym = jq_dl_sym;
  *dl_close = jq_dl_close;
  return jv_true();
}

// Asummes validated relative path to module
static jv find_lib(jq_state *jq, jv rel_path, jv search, const char *suffix, jv jq_origin, jv lib_origin) {
  if (!jv_is_valid(rel_path)) {
    jv_free(lib_origin);
    jv_free(jq_origin);
    jv_free(search);
    return rel_path;
  }
  if (jv_get_kind(rel_path) != JV_KIND_STRING) {
    jv_free(lib_origin);
    jv_free(jq_origin);
    jv_free(rel_path);
    jv_free(search);
    return jv_invalid_with_msg(jv_string_fmt("Module path must be a string"));
  }
  if (jv_get_kind(search) != JV_KIND_ARRAY) {
    jv_free(lib_origin);
    jv_free(jq_origin);
    jv_free(rel_path);
    jv_free(search);
    return jv_invalid_with_msg(jv_string_fmt("Module search path must be an array"));
  }

  if (strncmp(jv_string_value(rel_path), "jq/", sizeof("jq/") - 1) == 0) {
    /* Builtin module */
    jv_free(lib_origin);
    jv_free(jq_origin);
    jv_free(search);
    return jv_string_concat(rel_path, jv_string(suffix));
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
      nerrors += load_library(jq, resolved, 1, raw, optional, as_str, &dep_def_block, lib_state);
      if (nerrors == 0) {
        // Bind as both $data::data and $data for backward compatibility vs common sense
        bk = block_bind_library(dep_def_block, bk, OP_IS_CALL_PSEUDO, as_str);
        bk = block_bind_library(dep_def_block, bk, OP_IS_CALL_PSEUDO, NULL);
      }
    } else {
      uint64_t state_idx = 0;
      for (; state_idx < lib_state->nlibs; ++state_idx) {
        if (strcmp(lib_state->libs[state_idx].name,jv_string_value(resolved)) == 0)
          break;
      }

      if (state_idx < lib_state->nlibs) { // Found
        jv_free(resolved);
        // Bind the library to the program
        bk = block_bind_library(lib_state->libs[state_idx].defs, bk, OP_IS_CALL_PSEUDO, as_str);
      } else { // Not found.   Add it to the table before binding.
        block dep_def_block = gen_noop();
        nerrors += load_library(jq, resolved, 0, 1, optional, as_str, &dep_def_block, lib_state);
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

static int load_shared_object(jq_state *jq, struct jq_lib *lib) {
  jq_plugin_init_f get_cfuncs;
  int ret = 0;
  const char *err = 0;
  const char *data = 0;
  size_t datasz = 0;
  jv meta = block_module_meta(lib->defs);
  jv oname = jv_invalid();
  jv sym = jv_string("jq_plugin_init");
  jv tmp = jv_invalid();

  if (jv_get_kind(meta) != JV_KIND_OBJECT ||
      !jv_is_valid((oname = jv_object_get(jv_copy(meta), jv_string("cfunctions")))))
    goto out;

  if (jv_object_has(jv_copy(meta), jv_string("plugin_init_function")))
    sym = jv_object_get(jv_copy(meta), jv_string("plugin_init_function"));

  ret = 1;
  if (jv_get_kind(oname) == JV_KIND_TRUE)
    oname = jv_copy(lib->jbasename);
  if (jv_get_kind(oname) != JV_KIND_STRING) {
    jq_report_error(jq, jv_string_fmt("module %s: value of \"cfunctions\" key must be a string",
                                      lib->name));
    goto out;
  }
  oname = jv_string_concat(oname, jv_string(JQ_DLL_EXT));
  if (!jv_is_valid((tmp = validate_relpath(jv_copy(oname))))) {
    jq_report_error(jq, jv_string_fmt("module %s: path for DLL / shared object is not relative: %s: %s",
                                      lib->name, jv_string_value(oname), jv_string_value(tmp)));
    goto out;
  }
  /* A more idiomatic version of this would be nice: */
  jv_free(tmp);
  tmp = jv_string_fmt("%s%s%s", jv_string_value(lib->origin), JQ_PATH_SEP, jv_string_value(oname));
  jv_free(oname);
  oname = tmp;
  tmp = jv_invalid();

  jv msg = jq_dl_open(jv_string_value(oname), &lib->h, &lib->dl_sym, &lib->dl_close);
  if (!jv_is_valid(msg)) {
    jq_report_error(jq, jv_string_fmt("module %s: could not load DLL / shared object: %s",
                                      lib->name, jv_string_value(oname)));
    goto out;
  }
  if ((get_cfuncs = lib->dl_sym(lib->h, jv_string_value(sym))) == NULL) {
    jq_report_error(jq, jv_string_fmt("module %s: could find \"%s\" symbol in DLL / shared object: %s",
                                      lib->name, jv_string_value(sym), jv_string_value(oname)));
    goto out;
  }

  if ((ret = get_cfuncs(JQ_MIN_ABI, JQ_MAX_ABI, jq, &err, &data, &datasz, &lib->cfuncs, &lib->ncfuncs))) {
    jq_report_error(jq, jv_string_fmt("module %s: failed to initialize DLL / shared object: %s",
                                      lib->name, err ? err : "<unknown error>"));
    goto out;
  }

  ret = 0;
  if (data) {
    struct locfile* src = NULL;

    jv_free(lib->contents);
    block_free(lib->defs);
    lib->contents = datasz ? jv_string_sized(data, datasz) : jv_string(data);

    src = locfile_init(jq, lib->name,
                       jv_string_value(lib->contents),
                       jv_string_length_bytes(jv_copy(lib->contents)));
    ret = jq_parse_library(src, &lib->defs);
    locfile_free(src);
  }

out:
  jv_free(oname);
  jv_free(meta);
  jv_free(sym);
  jv_free(tmp);
  return ret;
}


// Loads the library at lib_path into lib_state, putting the library's defs
// into *out_block
static int load_library(jq_state *jq, jv lib_path, int is_data, int raw, int optional, const char *as, block *out_block, struct lib_loading_state *lib_state) {
  int nerrors = 0;
  struct locfile* src = NULL;
  char *dname = strdup(jv_string_value(lib_path));  // XXX check ENOMEM
  char *bname = strdup(dname);                      // XXX check ENOMEM
  size_t state_idx = lib_state->nlibs++;

  *out_block = gen_noop();
  lib_state->libs = jv_mem_realloc(lib_state->libs, lib_state->nlibs * sizeof(lib_state->libs[0]));

  /*
   * lib_state->libs gets realloc'ed in load_library(), so we don't want to
   * hold on to an element of that array.
   */
#define this_lib (&lib_state->libs[state_idx])
  this_lib->jname = jv_copy(lib_path);
  this_lib->origin = dname ? jv_string(dirname(dname)) : jv_invalid_with_msg(jv_false()); // XXX ENOMEM
  this_lib->jbasename = bname ? jv_string(basename(bname)) : jv_invalid_with_msg(jv_false()); // XXX ENOMEM
  this_lib->name = jv_string_value(this_lib->jname);
  this_lib->defs = gen_noop();
  memset(&this_lib->h, 0, sizeof(this_lib->h));
  this_lib->dl_sym = 0;
  this_lib->dl_close = 0;
  this_lib->cfuncs = NULL;
  this_lib->ncfuncs = 0;

  if (is_data && !raw)
    this_lib->contents = load_file(lib_path, 0);
  else
    this_lib->contents = load_file(lib_path, 1);
  if (!jv_is_valid(this_lib->contents)) {
    this_lib->defs = gen_noop();
    if (!optional) {
      if (jv_invalid_has_msg(jv_copy(this_lib->contents)))
        this_lib->contents = jv_invalid_get_msg(this_lib->contents);
      else
        this_lib->contents = jv_string("unknown error");
      jq_report_error(jq, jv_string_fmt("jq: error loading data file %s: %s\n", jv_string_value(lib_path), jv_string_value(this_lib->contents)));
      nerrors++;
    }
    goto out;
  } else if (is_data) {
    // import "foo" as $bar;
    this_lib->defs = gen_const_global(jv_copy(this_lib->contents), as);
  } else {
    // import "foo" [as bar] OR include "foo";
    src = locfile_init(jq, jv_string_value(lib_path),
                       jv_string_value(this_lib->contents),
                       jv_string_length_bytes(jv_copy(this_lib->contents)));
    nerrors += jq_parse_library(src, &this_lib->defs);
    /*
     * Load the shared object now because it may replace the jq code of the
     * library.
     */
    if (nerrors == 0)
      nerrors += load_shared_object(jq, this_lib);
    if (nerrors == 0) {
      nerrors += process_dependencies(jq, jq_get_jq_origin(jq),
                                      jv_copy(this_lib->origin), &this_lib->defs,
                                      lib_state);
      if (this_lib->ncfuncs)
        this_lib->defs = gen_cbinding(this_lib->cfuncs, this_lib->ncfuncs, this_lib->defs);
      this_lib->defs = block_hide(block_bind_self(this_lib->defs, OP_IS_CALL_PSEUDO));
    }
  }
  *out_block = this_lib->defs;
#undef this_lib
  if (src)
    locfile_free(src);
out:
  free(dname);
  free(bname);
  jv_free(lib_path);
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
    }
    locfile_free(src);
    block_free(program);
  }
  jv_free(lib_path);
  jv_free(data);
  return meta;
}

int load_program(jq_state *jq, struct locfile* src, block *out_block, struct lib_loading_state **out_libs) {
  int nerrors = 0;
  block program;
  nerrors = jq_parse(src, &program);
  if (nerrors)
    return nerrors;

  char* home = getenv("HOME");
  if (home) {    // silently ignore no $HOME
    /* Import ~/.jq as a library named "" found in $HOME */
    block import = gen_import_meta(gen_import("", NULL, 0),
        gen_const(JV_OBJECT(
            jv_string("optional"), jv_true(),
            jv_string("search"), jv_string(home))));
    program = BLOCK(import, program);
  }

  struct lib_loading_state *lib_state = jv_mem_calloc(1, sizeof(*lib_state));
  nerrors = process_dependencies(jq, jq_get_jq_origin(jq), jq_get_prog_origin(jq), &program, lib_state);
  block libs = gen_noop();
  for (size_t i = 0; i < lib_state->nlibs; ++i) {
    if (nerrors == 0 && !block_is_const(lib_state->libs[i].defs)) {
      libs = block_join(libs, lib_state->libs[i].defs);
      lib_state->libs[i].defs = gen_noop();
    }
  }
  if (nerrors) {
    block_free(program);
    libraries_free(lib_state);
  } else {
    *out_block = block_drop_unreferenced(block_join(libs, program));
    *out_libs = lib_state;
  }

  return nerrors;
}

void libraries_free(struct lib_loading_state *libs) {
  if (!libs)
    return;

  for (size_t i = 0; i < libs->nlibs; ++i) {
    if (libs->libs[i].dl_close)
      jq_dl_close(libs->libs[i].h);
    jv_free(libs->libs[i].jname);
    jv_free(libs->libs[i].jbasename);
    jv_free(libs->libs[i].origin);
    jv_free(libs->libs[i].contents);
    block_free(libs->libs[i].defs);
  }
  free(libs->libs);
  free(libs);
}
