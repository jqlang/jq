#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "compile.h"
#include "jv.h"
#include "jq.h"
#include "jv_alloc.h"
#include "util.h"
#include "version.h"

int jq_testsuite(jv lib_dirs, int argc, char* argv[]);

static const char* progname;

/*
 * For a longer help message we could use a better option parsing
 * strategy, one that lets stack options.
 */
static void usage(int code) {
  FILE *f = stderr;
  if (code == 0)
    f = stdout;
  fprintf(f, "\njq - commandline JSON processor [version %s]\n", JQ_VERSION);
  fprintf(f, "Usage: %s [options] <jq filter> [file...]\n\n", progname);
  fprintf(f, "\tjq is a tool for processing JSON inputs, applying the\n");
  fprintf(f, "\tgiven filter to its JSON text inputs and producing the\n");
  fprintf(f, "\tfilter's results as JSON on standard output.\n");
  fprintf(f, "\tThe simplest filter is ., which is the identity filter,\n");
  fprintf(f, "\tcopying jq's input to its output.\n");
  fprintf(f, "\tFor more advanced filters see the jq(1) manpage (\"man jq\")\n");
  fprintf(f, "\tand/or http://stedolan.github.com/jq\n\n");
  fprintf(f, "\tSome of the options include:\n");
  fprintf(f, "\t -h\t\tthis message;\n");
  fprintf(f, "\t -c\t\tcompact instead of pretty-printed output;\n");
  fprintf(f, "\t -n\t\tuse `null` as the single input value;\n");
  fprintf(f, "\t -s\t\tread (slurp) all inputs into an array; apply filter to it;\n");
  fprintf(f, "\t -r\t\toutput raw strings, not JSON texts;\n");
  fprintf(f, "\t -R\t\tread raw strings, not JSON texts;\n");
  fprintf(f, "\t --arg a v\tset variable $a to value <v>;\n");
  fprintf(f, "\t --argjson a v\tset variable $a to JSON value <v>;\n");
  fprintf(f, "\t --argfile a f\tset variable $a to JSON texts read from <f>;\n");
  fprintf(f, "\tSee the manpage for more options.\n");
  exit(code);
}

static void die() {
  fprintf(stderr, "Use %s --help for help with command-line options,\n", progname);
  fprintf(stderr, "or see the jq manpage, or online docs  at http://stedolan.github.com/jq\n");
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
  COMPACT_OUTPUT        = 16,
  ASCII_OUTPUT          = 32,
  COLOUR_OUTPUT         = 64,
  NO_COLOUR_OUTPUT      = 128,
  SORTED_OUTPUT         = 256,
  FROM_FILE             = 512,
  RAW_NO_LF             = 1024,
  UNBUFFERED_OUTPUT     = 2048,
  EXIT_STATUS           = 4096,
  SEQ                   = 8192,
  RUN_TESTS             = 16384,
  /* debugging only */
  DUMP_DISASM           = 32768,
};
static int options = 0;

static int process(jq_state *jq, jv value, int flags, int dumpopts) {
  int ret = 14; // No valid results && -e -> exit(4)
  jq_start(jq, value, flags);
  jv result;
  while (jv_is_valid(result = jq_next(jq))) {
    if ((options & RAW_OUTPUT) && jv_get_kind(result) == JV_KIND_STRING) {
      fwrite(jv_string_value(result), 1, jv_string_length_bytes(jv_copy(result)), stdout);
      ret = 0;
      jv_free(result);
    } else {
      if (jv_get_kind(result) == JV_KIND_FALSE || jv_get_kind(result) == JV_KIND_NULL)
        ret = 11;
      else
        ret = 0;
      if (options & SEQ)
        fwrite("\036", 1, 1, stdout);
      jv_dump(result, dumpopts);
    }
    if (!(options & RAW_NO_LF))
        printf("\n");
    if (options & UNBUFFERED_OUTPUT)
      fflush(stdout);
  }
  if (jv_invalid_has_msg(jv_copy(result))) {
    // Uncaught jq exception
    jv msg = jv_invalid_get_msg(jv_copy(result));
    if (jv_get_kind(msg) == JV_KIND_STRING) {
      fprintf(stderr, "jq: error: %s\n", jv_string_value(msg));
    } else {
      msg = jv_dump_string(msg, 0);
      fprintf(stderr, "jq: error (not a string): %s\n", jv_string_value(msg));
    }
    ret = 5;
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

int main(int argc, char* argv[]) {
  jq_state *jq = NULL;
  int ret = 0;
  int compiled = 0;
  int parser_flags = 0;
  int nfiles = 0;

  if (argc) progname = argv[0];

  jq = jq_init();
  if (jq == NULL) {
    perror("malloc");
    ret = 2;
    goto out;
  }

  const char* program = 0;

  jq_util_input_state input_state = jq_util_input_init(NULL, NULL); // XXX add err_cb

  int further_args_are_files = 0;
  int jq_flags = 0;
  size_t short_opts = 0;
  jv program_arguments = jv_array();
  jv lib_search_paths = jv_null();
  for (int i=1; i<argc; i++, short_opts = 0) {
    if (further_args_are_files) {
      jq_util_input_add_input(input_state, jv_string(argv[i]));
      nfiles++;
    } else if (!strcmp(argv[i], "--")) {
      if (!program) usage(2);
      further_args_are_files = 1;
    } else if (!isoptish(argv[i])) {
      if (program) {
        jq_util_input_add_input(input_state, jv_string(argv[i]));
        nfiles++;
      } else {
        program = argv[i];
      }
    } else {
      if (argv[i][1] == 'L') {
        if (jv_get_kind(lib_search_paths) == JV_KIND_NULL)
          lib_search_paths = jv_array();
        if (argv[i][2] != 0) { // -Lname (faster check than strlen)
            lib_search_paths = jv_array_append(lib_search_paths, jv_string(argv[i]+2));
        } else if (i >= argc - 1) {
          fprintf(stderr, "-L takes a parameter: (e.g. -L /search/path or -L/search/path)\n");
          die();
        } else {
          lib_search_paths = jv_array_append(lib_search_paths, jv_string(argv[i+1]));
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
        options |= COMPACT_OUTPUT;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'C', "color-output", &short_opts)) {
        options |= COLOUR_OUTPUT;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'M', "monochrome-output", &short_opts)) {
        options |= NO_COLOUR_OUTPUT;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'a', "ascii-output", &short_opts)) {
        options |= ASCII_OUTPUT;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 0, "unbuffered", &short_opts)) {
        options |= UNBUFFERED_OUTPUT;
        if (!short_opts) continue;
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
      if (isoption(argv[i], 0, "seq", &short_opts)) {
        options |= SEQ;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 0, "stream", &short_opts)) {
        parser_flags |= JV_PARSE_STREAMING;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 0, "stream-errors", &short_opts)) {
        parser_flags |= JV_PARSE_STREAM_ERRORS;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'e', "exit-status", &short_opts)) {
        options |= EXIT_STATUS;
        if (!short_opts) continue;
      }
      // FIXME: For --arg* we should check that the varname is acceptable
      if (isoption(argv[i], 0, "arg", &short_opts)) {
        if (i >= argc - 2) {
          fprintf(stderr, "%s: --arg takes two parameters (e.g. -a varname value)\n", progname);
          die();
        }
        jv arg = jv_object();
        arg = jv_object_set(arg, jv_string("name"), jv_string(argv[i+1]));
        arg = jv_object_set(arg, jv_string("value"), jv_string(argv[i+2]));
        program_arguments = jv_array_append(program_arguments, arg);
        i += 2; // skip the next two arguments
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 0, "argjson", &short_opts)) {
        if (i >= argc - 2) {
          fprintf(stderr, "%s: --argjson takes two parameters (e.g. -a varname text)\n", progname);
          die();
        }
        jv v = jv_parse(argv[i+2]);
        if (!jv_is_valid(v)) {
          fprintf(stderr, "%s: invalid JSON text passed to --argjson\n", progname);
          die();
        }
        jv arg = jv_object();
        arg = jv_object_set(arg, jv_string("name"), jv_string(argv[i+1]));
        arg = jv_object_set(arg, jv_string("value"), v);
        program_arguments = jv_array_append(program_arguments, arg);
        i += 2; // skip the next two arguments
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 0, "argfile", &short_opts)) {
        if (i >= argc - 2) {
          fprintf(stderr, "%s: --argfile takes two parameters (e.g. -a varname filename)\n", progname);
          die();
        }
        jv arg = jv_object();
        arg = jv_object_set(arg, jv_string("name"), jv_string(argv[i+1]));
        jv data = jv_load_file(argv[i+2], 0);
        if (!jv_is_valid(data)) {
          data = jv_invalid_get_msg(data);
          fprintf(stderr, "%s: Bad JSON in --argfile %s %s: %s\n", progname,
                  argv[i+1], argv[i+2], jv_string_value(data));
          jv_free(data);
          ret = 2;
          goto out;
        }
        arg = jv_object_set(arg, jv_string("value"), jv_array_get(data, 0));
        program_arguments = jv_array_append(program_arguments, arg);
        i += 2; // skip the next two arguments
        if (!short_opts) continue;
      }
      if (isoption(argv[i],  0,  "debug-dump-disasm", &short_opts)) {
        options |= DUMP_DISASM;
        if (!short_opts) continue;
      }
      if (isoption(argv[i],  0,  "debug-trace", &short_opts)) {
        jq_flags |= JQ_DEBUG_TRACE;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'h', "help", &short_opts)) {
        usage(0);
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'V', "version", &short_opts)) {
        printf("jq-%s\n", JQ_VERSION);
        ret = 0;
        goto out;
      }
      if (isoption(argv[i], 0, "run-tests", &short_opts)) {
        i++;
        // XXX Pass program_arguments, even a whole jq_state *, through;
        // could be useful for testing
        jv_free(program_arguments);
        ret = jq_testsuite(lib_search_paths, argc - i, argv + i);
        goto out;
      }

      // check for unknown options... if this argument was a short option
      if (strlen(argv[i]) != short_opts + 1) {
        fprintf(stderr, "%s: Unknown option %s\n", progname, argv[i]);
        die();
      }
    }
  }

  int dumpopts = 0;
#ifndef WIN32
  /* Disable colour by default on Windows builds as Windows
     terminals tend not to display it correctly */
  if (isatty(fileno(stdout)))
    dumpopts |= JV_PRINT_COLOUR;
#endif
  if (options & SORTED_OUTPUT) dumpopts |= JV_PRINT_SORTED;
  if (!(options & COMPACT_OUTPUT)) dumpopts |= JV_PRINT_PRETTY;
  if (options & ASCII_OUTPUT) dumpopts |= JV_PRINT_ASCII;
  if (options & COLOUR_OUTPUT) dumpopts |= JV_PRINT_COLOUR;
  if (options & NO_COLOUR_OUTPUT) dumpopts &= ~JV_PRINT_COLOUR;

  if (jv_get_kind(lib_search_paths) == JV_KIND_NULL) {
    // Default search path list
    lib_search_paths = JV_ARRAY(jv_string("~/.jq"),
                                jv_string("$ORIGIN/../lib/jq"),
                                jv_string("$ORIGIN/lib"));
  }
  jq_set_attr(jq, jv_string("JQ_LIBRARY_PATH"), lib_search_paths);

  char *origin = strdup(argv[0]);
  if (origin == NULL) {
    fprintf(stderr, "Error: out of memory\n");
    exit(1);
  }
  jq_set_attr(jq, jv_string("JQ_ORIGIN"), jv_string(dirname(origin)));
  free(origin);

  if (strchr(JQ_VERSION, '-') == NULL)
    jq_set_attr(jq, jv_string("VERSION_DIR"), jv_string(JQ_VERSION));
  else
    jq_set_attr(jq, jv_string("VERSION_DIR"), jv_string_fmt("%.*s-master", (int)(strchr(JQ_VERSION, '-') - JQ_VERSION), JQ_VERSION));

#if (!defined(WIN32) && defined(HAVE_ISATTY)) || defined(HAVE__ISATTY)

#if defined(HAVE__ISATTY) && defined(isatty)
#undef isatty
#define isatty _isatty
#endif

  if (!program && isatty(STDOUT_FILENO) && !isatty(STDIN_FILENO))
    program = ".";
#endif

  if (!program) usage(2);

  if ((options & PROVIDE_NULL) && (options & (RAW_INPUT | SLURP))) {
    fprintf(stderr, "%s: --null-input cannot be used with --raw-input or --slurp\n", progname);
    die();
  }
  
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
      ret = 2;
      goto out;
    }
    jq_set_attr(jq, jv_string("PROGRAM_ORIGIN"), jq_realpath(jv_string(dirname(program_origin))));
    compiled = jq_compile_args(jq, jv_string_value(data), program_arguments);
    free(program_origin);
    jv_free(data);
  } else {
    jq_set_attr(jq, jv_string("PROGRAM_ORIGIN"), jq_realpath(jv_string("."))); // XXX is this good?
    compiled = jq_compile_args(jq, program, program_arguments);
  }
  if (!compiled){
    ret = 3;
    goto out;
  }

  if (options & DUMP_DISASM) {
    jq_dump_disassembly(jq, 0);
    printf("\n");
  }

  if ((options & SEQ))
    parser_flags |= JV_PARSE_SEQ;

  if (!(options & RAW_INPUT))
    jq_util_input_set_parser(input_state, jv_parser_new(parser_flags), (options & SLURP) ? 1 : 0);

  // Let jq program read from inputs
  jq_set_input_cb(jq, jq_util_input_next_input_cb, input_state);

  // Let jq program call `debug` builtin and have that go somewhere
  jq_set_debug_cb(jq, debug_cb, &dumpopts);

  if (nfiles == 0)
    jq_util_input_add_input(input_state, jv_string("-"));

  if (options & PROVIDE_NULL) {
    ret = process(jq, jv_null(), jq_flags, dumpopts);
  } else {
    jv value;
    while (jq_util_input_open_errors(input_state) == 0 &&
           (jv_is_valid((value = jq_util_input_next_input(input_state))) || jv_invalid_has_msg(jv_copy(value)))) {
      if (jv_is_valid(value)) {
        ret = process(jq, value, jq_flags, dumpopts);
        continue;
      }

      // Parse error
      jv msg = jv_invalid_get_msg(value);
      if (!(options & SEQ)) {
        // --seq -> errors are not fatal
        ret = 4;
        fprintf(stderr, "parse error: %s\n", jv_string_value(msg));
        jv_free(msg);
        break;
      }
      fprintf(stderr, "ignoring parse error: %s\n", jv_string_value(msg));
      jv_free(msg);
    }
  }

  if (jq_util_input_open_errors(input_state) != 0)
    ret = 2;

out:
  /* XXX We really should catch ENOSPC and such errors in jv_dumpf()! */
  if (fclose(stdout)!=0) {
    fprintf(stderr,"Error: writing output failed: %s\n", strerror(errno));
    ret = 2;
  }

  jq_util_input_free(&input_state);
  jq_teardown(&jq);
  if (ret >= 10 && (options & EXIT_STATUS))
    return ret - 10;
  if (ret >= 10)
    return 0;
  return ret;
}
