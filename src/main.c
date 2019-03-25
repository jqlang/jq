#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <processenv.h>
#include <shellapi.h>
#include <wchar.h>
#include <wtypes.h>
#endif

#if !defined(HAVE_ISATTY) && defined(HAVE__ISATTY)
#undef isatty
#define isatty _isatty
#endif

#if defined(HAVE_ISATTY) || defined(HAVE__ISATTY)
#define USE_ISATTY
#endif

#include "compile.h"
#include "jv.h"
#include "jq.h"
#include "jv_alloc.h"
#include "util.h"
#include "src/version.h"

int jq_testsuite(jv lib_dirs, int verbose, int argc, char* argv[]);

static const char* progname;

/*
 * For a longer help message we could use a better option parsing
 * strategy, one that lets stack options.
 */
static void usage(int code, int keep_it_short) {
  FILE *f = stderr;

  if (code == 0)
    f = stdout;

  int ret = fprintf(f,
    "jq - commandline JSON processor [version %s]\n"
    "\nUsage:\t%s [options] <jq filter> [file...]\n"
    "\t%s [options] --args <jq filter> [strings...]\n"
    "\t%s [options] --jsonargs <jq filter> [JSON_TEXTS...]\n\n"
    "jq is a tool for processing JSON inputs, applying the given filter to\n"
    "its JSON text inputs and producing the filter's results as JSON on\n"
    "standard output.\n\n"
    "The simplest filter is ., which copies jq's input to its output\n"
    "unmodified (except for formatting, but note that IEEE754 is used\n"
    "for number representation internally, with all that that implies).\n\n"
    "For more advanced filters see the jq(1) manpage (\"man jq\")\n"
    "and/or https://stedolan.github.io/jq\n\n"
    "Example:\n\n\t$ echo '{\"foo\": 0}' | jq .\n"
    "\t{\n\t\t\"foo\": 0\n\t}\n\n",
    JQ_VERSION, progname, progname, progname);
  if (keep_it_short) {
    fprintf(f,
      "For a listing of options, use %s --help.\n",
      progname);
  } else {
    (void) fprintf(f,
      "Some of the options include:\n"
      "  -c               compact instead of pretty-printed output;\n"
      "  -n               use `null` as the single input value;\n"
      "  -e               set the exit status code based on the output;\n"
      "  -s               read (slurp) all inputs into an array; apply filter to it;\n"
      "  -r               output raw strings, not JSON texts;\n"
      "  -R               read raw strings, not JSON texts;\n"
      "  -C               colorize JSON;\n"
      "  -M               monochrome (don't colorize JSON);\n"
      "  -S               sort keys of objects on output;\n"
      "  --tab            use tabs for indentation;\n"
      "  --arg a v        set variable $a to value <v>;\n"
      "  --argjson a v    set variable $a to JSON value <v>;\n"
      "  --slurpfile a f  set variable $a to an array of JSON texts read from <f>;\n"
      "  --rawfile a f    set variable $a to a string consisting of the contents of <f>;\n"
      "  --args           remaining arguments are string arguments, not files;\n"
      "  --jsonargs       remaining arguments are JSON arguments, not files;\n"
      "  --               terminates argument processing;\n\n"
      "Named arguments are also available as $ARGS.named[], while\n"
      "positional arguments are available as $ARGS.positional[].\n"
      "\nSee the manpage for more options.\n");
  }
  exit((ret < 0 && code == 0) ? 2 : code);
}

static void die() {
  fprintf(stderr, "Use %s --help for help with command-line options,\n", progname);
  fprintf(stderr, "or see the jq manpage, or online docs  at https://stedolan.github.io/jq\n");
  exit(2);
}




static int isoptish(const char* text) {
  return text[0] == '-' && (text[1] == '-' || isalpha(text[1]));
}

static int isoption(const char* text, char shortopt, const char* longopt, size_t *short_opts) {
  if (text[0] != '-' || text[1] == '-')
    *short_opts = 0;
  if (text[0] != '-') return 0;

  // check long option
  if (text[1] == '-' && !strcmp(text+2, longopt)) return 1;
  else if (text[1] == '-') return 0;

  // must be short option; check it and...
  if (!shortopt) return 0;
  if (strchr(text, shortopt) != NULL) {
    (*short_opts)++; // ...count it (for option stacking)
    return 1;
  }
  return 0;
}

enum {
  SLURP                 = 1,
  RAW_INPUT             = 2,
  PROVIDE_NULL          = 4,
  RAW_OUTPUT            = 8,
  ASCII_OUTPUT          = 32,
  COLOR_OUTPUT          = 64,
  NO_COLOR_OUTPUT       = 128,
  SORTED_OUTPUT         = 256,
  FROM_FILE             = 512,
  RAW_NO_LF             = 1024,
  UNBUFFERED_OUTPUT     = 2048,
  EXIT_STATUS           = 4096,
  EXIT_STATUS_EXACT     = 8192,
  SEQ                   = 16384,
  RUN_TESTS             = 32768,
  /* debugging only */
  DUMP_DISASM           = 65536,
};
static int options = 0;

enum {
    JQ_OK              =  0,
    JQ_OK_NULL_KIND    = -1, /* exit 0 if --exit-status is not set*/
    JQ_ERROR_SYSTEM    =  2,
    JQ_ERROR_COMPILE   =  3,
    JQ_OK_NO_OUTPUT    = -4, /* exit 0 if --exit-status is not set*/
    JQ_ERROR_UNKNOWN   =  5,
};
#define jq_exit_with_status(r)  exit(abs(r))
#define jq_exit(r)              exit( r > 0 ? r : 0 )

static const char *skip_shebang(const char *p) {
  if (strncmp(p, "#!", sizeof("#!") - 1) != 0)
    return p;
  const char *n = strchr(p, '\n');
  if (n == NULL || n[1] != '#')
    return p;
  n = strchr(n + 1, '\n');
  if (n == NULL || n[1] == '#' || n[1] == '\0' || n[-1] != '\\' || n[-2] == '\\')
    return p;
  n = strchr(n + 1, '\n');
  if (n == NULL)
    return p;
  return n+1;
}

static int process(jq_state *jq, jv value, int flags, int dumpopts) {
  int ret = JQ_OK_NO_OUTPUT; // No valid results && -e -> exit(4)
  jq_start(jq, value, flags);
  jv result;
  while (jv_is_valid(result = jq_next(jq))) {
    if ((options & RAW_OUTPUT) && jv_get_kind(result) == JV_KIND_STRING) {
      if (options & ASCII_OUTPUT) {
        jv_dumpf(jv_copy(result), stdout, JV_PRINT_ASCII);
      } else {
        fwrite(jv_string_value(result), 1, jv_string_length_bytes(jv_copy(result)), stdout);
      }
      ret = JQ_OK;
      jv_free(result);
    } else {
      if (jv_get_kind(result) == JV_KIND_FALSE || jv_get_kind(result) == JV_KIND_NULL)
        ret = JQ_OK_NULL_KIND;
      else
        ret = JQ_OK;
      if (options & SEQ)
        priv_fwrite("\036", 1, stdout, dumpopts & JV_PRINT_ISATTY);
      jv_dump(result, dumpopts);
    }
    if (!(options & RAW_NO_LF))
      priv_fwrite("\n", 1, stdout, dumpopts & JV_PRINT_ISATTY);
    if (options & UNBUFFERED_OUTPUT)
      fflush(stdout);
  }
  if (jq_halted(jq)) {
    // jq program invoked `halt` or `halt_error`
    options |= EXIT_STATUS_EXACT;
    jv exit_code = jq_get_exit_code(jq);
    if (!jv_is_valid(exit_code))
      ret = JQ_OK;
    else if (jv_get_kind(exit_code) == JV_KIND_NUMBER)
      ret = jv_number_value(exit_code);
    else
      ret = JQ_ERROR_UNKNOWN;
    jv_free(exit_code);
    jv error_message = jq_get_error_message(jq);
    if (jv_get_kind(error_message) == JV_KIND_STRING) {
      fprintf(stderr, "jq: error: %s", jv_string_value(error_message));
    } else if (jv_get_kind(error_message) == JV_KIND_NULL) {
      // Halt with no output
    } else if (jv_is_valid(error_message)) {
      error_message = jv_dump_string(jv_copy(error_message), 0);
      fprintf(stderr, "jq: error: %s\n", jv_string_value(error_message));
    } // else no message on stderr; use --debug-trace to see a message
    fflush(stderr);
    jv_free(error_message);
  } else if (jv_invalid_has_msg(jv_copy(result))) {
    // Uncaught jq exception
    jv msg = jv_invalid_get_msg(jv_copy(result));
    jv input_pos = jq_util_input_get_position(jq);
    if (jv_get_kind(msg) == JV_KIND_STRING) {
      fprintf(stderr, "jq: error (at %s): %s\n",
              jv_string_value(input_pos), jv_string_value(msg));
    } else {
      msg = jv_dump_string(msg, 0);
      fprintf(stderr, "jq: error (at %s) (not a string): %s\n",
              jv_string_value(input_pos), jv_string_value(msg));
    }
    ret = JQ_ERROR_UNKNOWN;
    jv_free(input_pos);
    jv_free(msg);
  }
  jv_free(result);
  return ret;
}

static void debug_cb(void *data, jv input) {
  int dumpopts = *(int *)data;
  jv_dumpf(JV_ARRAY(jv_string("DEBUG:"), input), stderr, dumpopts & ~(JV_PRINT_PRETTY));
  fprintf(stderr, "\n");
}

#ifdef WIN32
int umain(int argc, char* argv[]);

int wmain(int argc, wchar_t* wargv[]) {
  size_t arg_sz;
  char **argv = alloca(argc * sizeof(wchar_t*));
  for (int i = 0; i < argc; i++) {
    argv[i] = alloca((arg_sz = WideCharToMultiByte(CP_UTF8,
                                                   0,
                                                   wargv[i],
                                                   -1, 0, 0, 0, 0)));
    WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], arg_sz, 0, 0);
  }
  return umain(argc, argv);
}

int umain(int argc, char* argv[]) {
#else /*}*/
int main(int argc, char* argv[]) {
#endif
  jq_state *jq = NULL;
  int ret = JQ_OK_NO_OUTPUT;
  int compiled = 0;
  int parser_flags = 0;
  int nfiles = 0;
  int last_result = -1; /* -1 = no result, 0=null or false, 1=true */
  int badwrite;
  jv ARGS = jv_array(); /* positional arguments */
  jv program_arguments = jv_object(); /* named arguments */

#ifdef WIN32
  fflush(stdout);
  fflush(stderr);
  _setmode(fileno(stdout), _O_TEXT | _O_U8TEXT);
  _setmode(fileno(stderr), _O_TEXT | _O_U8TEXT);
#endif

  if (argc) progname = argv[0];

  jq = jq_init();
  if (jq == NULL) {
    perror("malloc");
    ret = JQ_ERROR_SYSTEM;
    goto out;
  }

  int dumpopts = JV_PRINT_INDENT_FLAGS(2);
  const char* program = 0;

  jq_util_input_state *input_state = jq_util_input_init(NULL, NULL); // XXX add err_cb

  int further_args_are_strings = 0;
  int further_args_are_json = 0;
  int args_done = 0;
  int jq_flags = 0;
  size_t short_opts = 0;
  jv lib_search_paths = jv_null();
  for (int i=1; i<argc; i++, short_opts = 0) {
    if (args_done) {
      if (further_args_are_strings) {
        ARGS = jv_array_append(ARGS, jv_string(argv[i]));
      } else if (further_args_are_json) {
        ARGS = jv_array_append(ARGS, jv_parse(argv[i]));
      } else {
        jq_util_input_add_input(input_state, argv[i]);
        nfiles++;
      }
    } else if (!strcmp(argv[i], "--")) {
      if (!program) usage(2, 1);
      args_done = 1;
    } else if (!isoptish(argv[i])) {
      if (program) {
        if (further_args_are_strings) {
          ARGS = jv_array_append(ARGS, jv_string(argv[i]));
        } else if (further_args_are_json) {
          ARGS = jv_array_append(ARGS, jv_parse(argv[i]));
        } else {
          jq_util_input_add_input(input_state, argv[i]);
          nfiles++;
        }
      } else {
        program = argv[i];
      }
    } else {
      if (argv[i][1] == 'L') {
        if (jv_get_kind(lib_search_paths) == JV_KIND_NULL)
          lib_search_paths = jv_array();
        if (argv[i][2] != 0) { // -Lname (faster check than strlen)
            lib_search_paths = jv_array_append(lib_search_paths, jq_realpath(jv_string(argv[i]+2)));
        } else if (i >= argc - 1) {
          fprintf(stderr, "-L takes a parameter: (e.g. -L /search/path or -L/search/path)\n");
          die();
        } else {
          lib_search_paths = jv_array_append(lib_search_paths, jq_realpath(jv_string(argv[i+1])));
          i++;
        }
        continue;
      }

      if (isoption(argv[i], 's', "slurp", &short_opts)) {
        options |= SLURP;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'r', "raw-output", &short_opts)) {
        options |= RAW_OUTPUT;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'c', "compact-output", &short_opts)) {
        dumpopts &= ~(JV_PRINT_TAB | JV_PRINT_INDENT_FLAGS(7));
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'C', "color-output", &short_opts)) {
        options |= COLOR_OUTPUT;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'M', "monochrome-output", &short_opts)) {
        options |= NO_COLOR_OUTPUT;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'a', "ascii-output", &short_opts)) {
        options |= ASCII_OUTPUT;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 0, "unbuffered", &short_opts)) {
        options |= UNBUFFERED_OUTPUT;
        continue;
      }
      if (isoption(argv[i], 'S', "sort-keys", &short_opts)) {
        options |= SORTED_OUTPUT;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'R', "raw-input", &short_opts)) {
        options |= RAW_INPUT;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'n', "null-input", &short_opts)) {
        options |= PROVIDE_NULL;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'f', "from-file", &short_opts)) {
        options |= FROM_FILE;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'j', "join-output", &short_opts)) {
        options |= RAW_OUTPUT | RAW_NO_LF;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'b', "binary", &short_opts)) {
#ifdef WIN32
        fflush(stdout);
        fflush(stderr);
        _setmode(fileno(stdin),  _O_BINARY);
        _setmode(fileno(stdout), _O_BINARY);
        _setmode(fileno(stderr), _O_BINARY);
        if (!short_opts) continue;
#endif
      }
      if (isoption(argv[i], 0, "tab", &short_opts)) {
        dumpopts &= ~JV_PRINT_INDENT_FLAGS(7);
        dumpopts |= JV_PRINT_TAB | JV_PRINT_PRETTY;
        continue;
      }
      if (isoption(argv[i], 0, "indent", &short_opts)) {
        if (i >= argc - 1) {
          fprintf(stderr, "%s: --indent takes one parameter\n", progname);
          die();
        }
        dumpopts &= ~(JV_PRINT_TAB | JV_PRINT_INDENT_FLAGS(7));
        int indent = atoi(argv[i+1]);
        if (indent < -1 || indent > 7) {
          fprintf(stderr, "%s: --indent takes a number between -1 and 7\n", progname);
          die();
        }
        dumpopts |= JV_PRINT_INDENT_FLAGS(indent);
        i++;
        continue;
      }
      if (isoption(argv[i], 0, "seq", &short_opts)) {
        options |= SEQ;
        continue;
      }
      if (isoption(argv[i], 0, "stream", &short_opts)) {
        parser_flags |= JV_PARSE_STREAMING;
        continue;
      }
      if (isoption(argv[i], 0, "stream-errors", &short_opts)) {
        parser_flags |= JV_PARSE_STREAM_ERRORS;
        continue;
      }
      if (isoption(argv[i], 'e', "exit-status", &short_opts)) {
        options |= EXIT_STATUS;
        if (!short_opts) continue;
      }
      // FIXME: For --arg* we should check that the varname is acceptable
      if (isoption(argv[i], 0, "args", &short_opts)) {
        further_args_are_strings = 1;
        further_args_are_json = 0;
        continue;
      }
      if (isoption(argv[i], 0, "jsonargs", &short_opts)) {
        further_args_are_strings = 0;
        further_args_are_json = 1;
        continue;
      }
      if (isoption(argv[i], 0, "arg", &short_opts)) {
        if (i >= argc - 2) {
          fprintf(stderr, "%s: --arg takes two parameters (e.g. --arg varname value)\n", progname);
          die();
        }
        if (!jv_object_has(jv_copy(program_arguments), jv_string(argv[i+1])))
          program_arguments = jv_object_set(program_arguments, jv_string(argv[i+1]), jv_string(argv[i+2]));
        i += 2; // skip the next two arguments
        continue;
      }
      if (isoption(argv[i], 0, "argjson", &short_opts)) {
        if (i >= argc - 2) {
          fprintf(stderr, "%s: --argjson takes two parameters (e.g. --argjson varname text)\n", progname);
          die();
        }
        if (!jv_object_has(jv_copy(program_arguments), jv_string(argv[i+1]))) {
          jv v = jv_parse(argv[i+2]);
          if (!jv_is_valid(v)) {
            fprintf(stderr, "%s: invalid JSON text passed to --argjson\n", progname);
            die();
          }
          program_arguments = jv_object_set(program_arguments, jv_string(argv[i+1]), v);
        }
        i += 2; // skip the next two arguments
        continue;
      }
      if (isoption(argv[i], 0, "argfile", &short_opts) ||
          isoption(argv[i], 0, "rawfile", &short_opts) ||
          isoption(argv[i], 0, "slurpfile", &short_opts)) {
        int raw = isoption(argv[i], 0, "rawfile", &short_opts);
        const char *which;
        if (isoption(argv[i], 0, "argfile", &short_opts))
          which = "argfile";
        else if (raw)
          which = "rawfile";
        else
          which = "slurpfile";
        if (i >= argc - 2) {
          fprintf(stderr, "%s: --%s takes two parameters (e.g. --%s varname filename)\n", progname, which, which);
          die();
        }
        if (!jv_object_has(jv_copy(program_arguments), jv_string(argv[i+1]))) {
          jv data = jv_load_file(argv[i+2], raw);
          if (!jv_is_valid(data)) {
            data = jv_invalid_get_msg(data);
            fprintf(stderr, "%s: Bad JSON in --%s %s %s: %s\n", progname, which,
                    argv[i+1], argv[i+2], jv_string_value(data));
            jv_free(data);
            ret = JQ_ERROR_SYSTEM;
            goto out;
          }
          if (strcmp(which, "argfile") == 0 &&
              jv_get_kind(data) == JV_KIND_ARRAY && jv_array_length(jv_copy(data)) == 1)
              data = jv_array_get(data, 0);
          program_arguments = jv_object_set(program_arguments, jv_string(argv[i+1]), data);
        }
        i += 2; // skip the next two arguments
        continue;
      }
      if (isoption(argv[i],  0,  "debug-dump-disasm", &short_opts)) {
        options |= DUMP_DISASM;
        continue;
      }
      if (isoption(argv[i],  0,  "debug-trace=all", &short_opts)) {
        jq_flags |= JQ_DEBUG_TRACE_ALL;
        if (!short_opts) continue;
      }
      if (isoption(argv[i],  0,  "debug-trace", &short_opts)) {
        jq_flags |= JQ_DEBUG_TRACE;
        continue;
      }
      if (isoption(argv[i], 'h', "help", &short_opts)) {
        usage(0, 0);
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'V', "version", &short_opts)) {
        printf("jq-%s\n", JQ_VERSION);
        ret = JQ_OK;
        goto out;
      }
      if (isoption(argv[i], 0, "run-tests", &short_opts)) {
        i++;
        // XXX Pass program_arguments, even a whole jq_state *, through;
        // could be useful for testing
        ret = jq_testsuite(lib_search_paths,
                           (options & DUMP_DISASM) || (jq_flags & JQ_DEBUG_TRACE),
                           argc - i, argv + i);
        goto out;
      }

      // check for unknown options... if this argument was a short option
      if (strlen(argv[i]) != short_opts + 1) {
        fprintf(stderr, "%s: Unknown option %s\n", progname, argv[i]);
        die();
      }
    }
  }

#ifdef USE_ISATTY
  if (isatty(STDOUT_FILENO)) {
#ifndef WIN32
    dumpopts |= JV_PRINT_ISATTY | JV_PRINT_COLOR;
#else
  /* Verify we actually have the console, as the NUL device is also regarded as
     tty.  Windows can handle color if ANSICON (or ConEmu) is installed, or
     Windows 10 supports the virtual terminal */
    DWORD mode;
    HANDLE con = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleMode(con, &mode)) {
      dumpopts |= JV_PRINT_ISATTY;
      if (getenv("ANSICON") != NULL ||
          SetConsoleMode(con, mode | 4/*ENABLE_VIRTUAL_TERMINAL_PROCESSING*/))
        dumpopts |= JV_PRINT_COLOR;
    }
#endif
  }
#endif
  if (options & SORTED_OUTPUT) dumpopts |= JV_PRINT_SORTED;
  if (options & ASCII_OUTPUT) dumpopts |= JV_PRINT_ASCII;
  if (options & COLOR_OUTPUT) dumpopts |= JV_PRINT_COLOR;
  if (options & NO_COLOR_OUTPUT) dumpopts &= ~JV_PRINT_COLOR;

  if (getenv("JQ_COLORS") != NULL && !jq_set_colors(getenv("JQ_COLORS")))
      fprintf(stderr, "Failed to set $JQ_COLORS\n");

  if (jv_get_kind(lib_search_paths) == JV_KIND_NULL) {
    // Default search path list
    lib_search_paths = JV_ARRAY(jv_string("~/.jq"),
                                jv_string("$ORIGIN/../lib/jq"),
                                jv_string("$ORIGIN/lib"));
  }
  jq_set_attr(jq, jv_string("JQ_LIBRARY_PATH"), lib_search_paths);

  char *origin = strdup(argv[0]);
  if (origin == NULL) {
    fprintf(stderr, "jq: error: out of memory\n");
    exit(1);
  }
  jq_set_attr(jq, jv_string("JQ_ORIGIN"), jv_string(dirname(origin)));
  free(origin);

  if (strchr(JQ_VERSION, '-') == NULL)
    jq_set_attr(jq, jv_string("VERSION_DIR"), jv_string(JQ_VERSION));
  else
    jq_set_attr(jq, jv_string("VERSION_DIR"), jv_string_fmt("%.*s-master", (int)(strchr(JQ_VERSION, '-') - JQ_VERSION), JQ_VERSION));

#ifdef USE_ISATTY
  if (!program && (!isatty(STDOUT_FILENO) || !isatty(STDIN_FILENO)))
    program = ".";
#endif

  if (!program) usage(2, 1);

  if (options & FROM_FILE) {
    char *program_origin = strdup(program);
    if (program_origin == NULL) {
      perror("malloc");
      exit(2);
    }

    jv data = jv_load_file(program, 1);
    if (!jv_is_valid(data)) {
      data = jv_invalid_get_msg(data);
      fprintf(stderr, "%s: %s\n", progname, jv_string_value(data));
      jv_free(data);
      ret = JQ_ERROR_SYSTEM;
      goto out;
    }
    jq_set_attr(jq, jv_string("PROGRAM_ORIGIN"), jq_realpath(jv_string(dirname(program_origin))));
    ARGS = JV_OBJECT(jv_string("positional"), ARGS,
                     jv_string("named"), jv_copy(program_arguments));
    program_arguments = jv_object_set(program_arguments, jv_string("ARGS"), jv_copy(ARGS));
    compiled = jq_compile_args(jq, skip_shebang(jv_string_value(data)), jv_copy(program_arguments));
    free(program_origin);
    jv_free(data);
  } else {
    jq_set_attr(jq, jv_string("PROGRAM_ORIGIN"), jq_realpath(jv_string("."))); // XXX is this good?
    ARGS = JV_OBJECT(jv_string("positional"), ARGS,
                     jv_string("named"), jv_copy(program_arguments));
    program_arguments = jv_object_set(program_arguments, jv_string("ARGS"), jv_copy(ARGS));
    compiled = jq_compile_args(jq, program, jv_copy(program_arguments));
  }
  if (!compiled){
    ret = JQ_ERROR_COMPILE;
    goto out;
  }

  if (options & DUMP_DISASM) {
    jq_dump_disassembly(jq, 0);
    printf("\n");
  }

  if ((options & SEQ))
    parser_flags |= JV_PARSE_SEQ;

  if ((options & RAW_INPUT))
    jq_util_input_set_parser(input_state, NULL, (options & SLURP) ? 1 : 0);
  else
    jq_util_input_set_parser(input_state, jv_parser_new(parser_flags), (options & SLURP) ? 1 : 0);

  // Let jq program read from inputs
  jq_set_input_cb(jq, jq_util_input_next_input_cb, input_state);

  // Let jq program call `debug` builtin and have that go somewhere
  jq_set_debug_cb(jq, debug_cb, &dumpopts);

  if (nfiles == 0)
    jq_util_input_add_input(input_state, "-");

  if (options & PROVIDE_NULL) {
    ret = process(jq, jv_null(), jq_flags, dumpopts);
  } else {
    jv value;
    while (jq_util_input_errors(input_state) == 0 &&
           (jv_is_valid((value = jq_util_input_next_input(input_state))) || jv_invalid_has_msg(jv_copy(value)))) {
      if (jv_is_valid(value)) {
        ret = process(jq, value, jq_flags, dumpopts);
        if (ret <= 0 && ret != JQ_OK_NO_OUTPUT)
          last_result = (ret != JQ_OK_NULL_KIND);
        continue;
      }

      // Parse error
      jv msg = jv_invalid_get_msg(value);
      if (!(options & SEQ)) {
        // --seq -> errors are not fatal
        ret = JQ_OK_NO_OUTPUT;
        fprintf(stderr, "jq: parse error: %s\n", jv_string_value(msg));
        jv_free(msg);
        break;
      }
      fprintf(stderr, "jq: ignoring parse error: %s\n", jv_string_value(msg));
      jv_free(msg);
    }
  }

  if (jq_util_input_errors(input_state) != 0)
    ret = JQ_ERROR_SYSTEM;

out:
  badwrite = ferror(stdout);
  if (fclose(stdout)!=0 || badwrite) {
    fprintf(stderr,"jq: error: writing output failed: %s\n", strerror(errno));
    ret = JQ_ERROR_SYSTEM;
  }

  jv_free(ARGS);
  jv_free(program_arguments);
  jq_util_input_free(&input_state);
  jq_teardown(&jq);

  if (options & (EXIT_STATUS|EXIT_STATUS_EXACT)) {
    if (ret != JQ_OK_NO_OUTPUT)
      jq_exit_with_status(ret);
    else
      switch (last_result) {
        case -1: jq_exit_with_status(JQ_OK_NO_OUTPUT);
        case  0: jq_exit_with_status(JQ_OK_NULL_KIND);
        default: jq_exit_with_status(JQ_OK);
      }
  } else
    jq_exit(ret);
}
