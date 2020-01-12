jq FFI
======

jq now has an FFI.  You can create a module with C-coded builtins.

A jq module's jq source can say:

    module {"cfunctions":true};

to cause a shared object or DLL of the same name to be loaded.

An alternative shared object or DLL name can be specified instead:

    module {"cfunctions":"my_shared_object"};

The shared object or DLL must export a function named `jq_plugin_init`,
or else it must list an alternative name for that function it its module
meta:

    module {"cfunctions":true,"plugin_init_function":"my_plugin_init"};

The module's C sources must `#define JQ_PLUGIN` and must `#include` all
of `<jv.h>`, `<jq.h>`, and `<jq_plugin.h>`.

The init function has the following prototype:

    typedef int (*jq_plugin_init_f)
        (int,                       /* jq plugin min supported ABI version */
         int,                       /* jq plugin max supported ABI version */
         struct jq_state *,
         const char **,             /* error string */
         const char **,             /* jq-coded module contents */
         size_t *,                  /* jq-coded module contents size */
         struct cfunction **,       /* array of C-coded function descriptors */
         size_t *);

and must return 0 on success.  On failure, it should output an error
string via the fourth argument.

The plugin must check that two jq ABI integers include the
`JQ_CURRENT_ABI` that the plugin was compiled against.

The plugin can output a replacement for its jq-coded part via the fifth
and sixth arguments.  If the fifth argument is not `NULL` and the sixth
is `0`, then the string output in the fifth argument must be a C string
(i.e., `NUL`-terminated).

The plugin can output C-coded functions via the seventh and eigth init
function arguments.

Each C-coded function must have one the following prototypes:

    jv my_func(struct jq_state *jq, jv input);
    jv my_func(struct jq_state *jq, jv input, jv arg);
    jv my_func(struct jq_state *jq, jv input, jv a0, jv a1);
    ..
    jv my_func(struct jq_state *jq, jv input, jv a0, jv a1, jv a2, jv a3, jv a4, jv a5);

and must have an entry in the `struct cfunction` array output by the
module's init function.

`struct cfunction` is:

    typedef void (*cfunction_ptr)();
    struct cfunction {
      cfunction_ptr fptr;       /* function pointer */
      const char* name;         /* jq function name */
      int nargs;                /* number of arguments; must be at least 1 */
      unsigned int pure:1;      /* 1 if pure, 0 if impure */
      unsigned int exported:1;  /* 1 if exported, 0 if not */
    };


jq plugins and DLL hell
=======================

All `jv` and `jq` functions in a plugin are replaced with macros of this
form:

    #define jv_fname ((*(struct jq_plugin_vtable **)jq)->jv_fname)

Here `jq` should be the `struct jq_state *` argument to the C-coded
builtin functions and the module's init function.

This means that plugins can only call `jv` and `jq` functions in contexts
where a `jq` variable is visible (and of type `struct jq_state *jq`).

Since the plugin's init function and all its C-coded functions receive a
`struct jq_state *jq` argument, a `struct jq_state *jq` should always be
available for calling `jv` and `jq` functions.

This way, plugins do not need to be linked with `libjq`.  This is quite
important, as it limits DLL hell.  Essentially plugins "link" with
`libjq` dynamically at run-time without having to rely on the system's
RTLD to provide that functionality.

This means that one can use the same plugin object in applications that
use different versions of `libjq`, provided that the jq ABIs of the two
`libjq` versions are sufficiently compatible (meaning, the layout of
`jv` is the same, both provide all the functions needed by the plugin,
and so on).

Depending on the platform and how `libjq` or its callers are built
(e.g., with versioned symbols or direct binding) it should even be
possible even to have two different versions of `libjq` loaded in the
*same process* and loading the same plugins, and the plugins will always
use the `libjq` functions of the version of `libjq` calling them.
