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
#include "version.h"

int jq_testsuite(int argc, char* argv[]);

static const char* progname;

/*
 * For a longer help message we could use a better option parsing
 * strategy, one that lets stack options.
 */
static void usage(int code) {
  fprintf(stderr, "\njq - commandline JSON processor [version %s]\n", JQ_VERSION);
  fprintf(stderr, "Usage: %s [options] <jq filter> [file...]\n\n", progname);
  fprintf(stderr, "\tjq is a tool for processing JSON inputs, applying the\n");
  fprintf(stderr, "\tgiven filter to its JSON text inputs and producing the\n");
  fprintf(stderr, "\tfilter's results as JSON on standard output.\n");
  fprintf(stderr, "\tThe simplest filter is ., which is the identity filter,\n");
  fprintf(stderr, "\tcopying jq's input to its output.\n");
  fprintf(stderr, "\tFor more advanced filters see the jq(1) manpage (\"man jq\")\n");
  fprintf(stderr, "\tand/or http://stedolan.github.com/jq\n\n");
  fprintf(stderr, "\tSome of the options include:\n");
  fprintf(stderr, "\t -h\t\tthis message;\n");
  fprintf(stderr, "\t -c\t\tcompact instead of pretty-printed output;\n");
  fprintf(stderr, "\t -n\t\tuse `null` as the single input value;\n");
  fprintf(stderr, "\t -i\t\tedit the [first] file in-place;\n");
  fprintf(stderr, "\t -s\t\tread (slurp) all inputs into an array; apply filter to it;\n");
  fprintf(stderr, "\t -r\t\toutput raw strings, not JSON texts;\n");
  fprintf(stderr, "\t -R\t\tread raw strings, not JSON texts;\n");
  fprintf(stderr, "\t --arg a v\tset variable $a to value <v>;\n");
  fprintf(stderr, "\t --argfile a f\tset variable $a to JSON texts read from <f>;\n");
  fprintf(stderr, "\tSee the manpage for more options.\n");
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
  IN_PLACE              = 8192,
  /* debugging only */
  DUMP_DISASM           = 16384,
};
static int options = 0;

static int process(jq_state *jq, jv value, int flags) {
  int ret = 14; // No valid results && -e -> exit(4)
  jq_start(jq, value, flags);
  jv result;
  while (jv_is_valid(result = jq_next(jq))) {
    if ((options & RAW_OUTPUT) && jv_get_kind(result) == JV_KIND_STRING) {
      fwrite(jv_string_value(result), 1, jv_string_length_bytes(jv_copy(result)), stdout);
      ret = 0;
      jv_free(result);
    } else {
      int dumpopts;
      /* Disable colour by default on Windows builds as Windows
         terminals tend not to display it correctly */
#ifdef WIN32
      dumpopts = 0;
#else
      dumpopts = isatty(fileno(stdout)) ? JV_PRINT_COLOUR : 0;
#endif
      if (options & SORTED_OUTPUT) dumpopts |= JV_PRINT_SORTED;
      if (!(options & COMPACT_OUTPUT)) dumpopts |= JV_PRINT_PRETTY;
      if (options & ASCII_OUTPUT) dumpopts |= JV_PRINT_ASCII;
      if (options & COLOUR_OUTPUT) dumpopts |= JV_PRINT_COLOUR;
      if (options & NO_COLOUR_OUTPUT) dumpopts &= ~JV_PRINT_COLOUR;
      if (jv_get_kind(result) == JV_KIND_FALSE || jv_get_kind(result) == JV_KIND_NULL)
        ret = 11;
      else
        ret = 0;
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
    jv_free(msg);
  }
  jv_free(result);
  return ret;
}

FILE* current_input;
const char** input_filenames = NULL;
int ninput_files;
int next_input_idx;
static int read_more(char* buf, size_t size, int* islast) {
  while (!current_input || feof(current_input)) {
    if (current_input) {
      if (current_input == stdin) {
        clearerr(stdin); // perhaps we can read again
      } else {
        fclose(current_input);
      }
      current_input = 0;
    }
    if (next_input_idx == ninput_files) {
      assert(*islast == 1);
      return 0;
    }
    *islast = 0;
    if (!strcmp(input_filenames[next_input_idx], "-")) {
      current_input = stdin;
    } else {
      current_input = fopen(input_filenames[next_input_idx], "r");
    }
    if (!current_input) {
      fprintf(stderr, "%s: %s: %s\n", progname, input_filenames[next_input_idx], strerror(errno));
    }
    next_input_idx++;
  }

  if (!fgets(buf, size, current_input)) buf[0] = 0;
  if (!current_input || feof(current_input)) *islast = 1;
  return 1;
}

int main(int argc, char* argv[]) {
  jq_state *jq = NULL;
  int ret = 0;
  int compiled = 0;
  char *t = NULL;

  if (argc) progname = argv[0];

  if (argc > 1 && !strcmp(argv[1], "--run-tests")) {
    return jq_testsuite(argc, argv);
  }

  jq = jq_init();
  if (jq == NULL) {
    perror("malloc");
    ret = 2;
    goto out;
  }

  const char* program = 0;
  input_filenames = jv_mem_alloc(sizeof(const char*) * argc);
  ninput_files = 0;
  int further_args_are_files = 0;
  int jq_flags = 0;
  size_t short_opts = 0;
  jv program_arguments = jv_array();
  jv lib_search_paths = jv_array();
  for (int i=1; i<argc; i++, short_opts = 0) {
    if (further_args_are_files) {
      input_filenames[ninput_files++] = argv[i];
    } else if (!strcmp(argv[i], "--")) {
      if (!program) usage(2);
      further_args_are_files = 1;
    } else if (!isoptish(argv[i])) {
      if (program) {
        input_filenames[ninput_files++] = argv[i];
      } else {
        program = argv[i];
      }
    } else {
      if (argv[i][1] == 'L') {
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
      if (isoption(argv[i], 'i', "in-place", &short_opts)) {
        options |= IN_PLACE;
        if (!short_opts) continue;
      }
      if (isoption(argv[i], 'e', "exit-status", &short_opts)) {
        options |= EXIT_STATUS;
        if (!short_opts) continue;
      }
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
        if (jv_get_kind(data) == JV_KIND_ARRAY && jv_array_length(jv_copy(data)) == 1)
            data = jv_array_get(data, 0);
        arg = jv_object_set(arg, jv_string("value"), data);
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

      // check for unknown options... if this argument was a short option
      if (strlen(argv[i]) != short_opts + 1) {
        fprintf(stderr, "%s: Unknown option %s\n", progname, argv[i]);
        die();
      }
    }
  }

  char *penv = getenv("JQ_LIBRARY_PATH");
  if (penv) {
#ifdef WIN32
#define PATH_ENV_SEPARATOR ";"
#else
#define PATH_ENV_SEPARATOR ":"
#endif
    lib_search_paths = jv_array_concat(lib_search_paths,jv_string_split(jv_string(penv),jv_string(PATH_ENV_SEPARATOR)));
#undef PATH_ENV_SEPARATOR
  }
  jq_set_attr(jq, jv_string("LIB_DIRS"), lib_search_paths);

  char *origin = strdup(argv[0]);
  if (origin == NULL) {
    fprintf(stderr, "Error: out of memory\n");
    exit(1);
  }
  jq_set_attr(jq, jv_string("ORIGIN"), jv_string(dirname(origin)));
  free(origin);

  if (strchr(JQ_VERSION, '-') == NULL)
    jq_set_attr(jq, jv_string("VERSION_DIR"), jv_string(JQ_VERSION));
  else
    jq_set_attr(jq, jv_string("VERSION_DIR"), jv_string_fmt("%.*s-master", strchr(JQ_VERSION, '-') - JQ_VERSION, JQ_VERSION));

#if (!defined(WIN32) && defined(HAVE_ISATTY)) || defined(HAVE__ISATTY)

#if defined(HAVE__ISATTY) && defined(isatty)
#undef isatty
#define isatty _isatty
#endif

  if (!program && isatty(STDOUT_FILENO) && !isatty(STDIN_FILENO))
    program = ".";
#endif

  if (!program) usage(2);
  if ((options & IN_PLACE)) {
    if (ninput_files == 0) usage(2);
    if (strcmp(input_filenames[0], "-") == 0) usage(2);
    size_t tlen = strlen(input_filenames[0]) + 7;
    t = jv_mem_alloc(tlen);
    int n = snprintf(t, tlen,"%sXXXXXX", input_filenames[0]);
    assert(n > 0 && (size_t)n < tlen);
    if (mkstemp(t) == -1) {
      fprintf(stderr, "Error: %s creating temporary file", strerror(errno));
      exit(3);
    }
    if (freopen(t, "w", stdout) == NULL) {
      fprintf(stderr, "Error: %s redirecting stdout to temporary file", strerror(errno));
      exit(3);
    }
  }
  if (ninput_files == 0) current_input = stdin;

  if ((options & PROVIDE_NULL) && (options & (RAW_INPUT | SLURP))) {
    fprintf(stderr, "%s: --null-input cannot be used with --raw-input or --slurp\n", progname);
    die();
  }
  
  if (options & FROM_FILE) {
    jv data = jv_load_file(program, 1);
    if (!jv_is_valid(data)) {
      data = jv_invalid_get_msg(data);
      fprintf(stderr, "%s: %s\n", progname, jv_string_value(data));
      jv_free(data);
      ret = 2;
      goto out;
    }
    compiled = jq_compile_args(jq, jv_string_value(data), program_arguments);
    jv_free(data);
  } else {
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

  if (options & PROVIDE_NULL) {
    ret = process(jq, jv_null(), jq_flags);
  } else {
    jv slurped;
    if (options & SLURP) {
      if (options & RAW_INPUT) {
        slurped = jv_string("");
      } else {
        slurped = jv_array();
      }
    }
    struct jv_parser* parser = jv_parser_new(0);
    char buf[4096];
    int is_last = 0;
    while (read_more(buf, sizeof(buf), &is_last)) {
      if (options & RAW_INPUT) {
        int len = strlen(buf);
        if (len > 0) {
          if (options & SLURP) {
            slurped = jv_string_concat(slurped, jv_string(buf));
          } else {
            if (buf[len-1] == '\n') buf[len-1] = 0;
            ret = process(jq, jv_string(buf), jq_flags);
          }
        }
      } else {
        jv_parser_set_buf(parser, buf, strlen(buf), !is_last);
        jv value;
        while (jv_is_valid((value = jv_parser_next(parser)))) {
          if (options & SLURP) {
            slurped = jv_array_append(slurped, value);
          } else {
            ret = process(jq, value, jq_flags);
          }
        }
        if (jv_invalid_has_msg(jv_copy(value))) {
          jv msg = jv_invalid_get_msg(value);
          fprintf(stderr, "parse error: %s\n", jv_string_value(msg));
          jv_free(msg);
          ret = 4;
          break;
        } else {
          jv_free(value);
        }
      }
    }
    jv_parser_free(parser);
    if (ret != 0)
      goto out;
    if (options & SLURP) {
      ret = process(jq, slurped, jq_flags);
    }
  }
  if ((options & IN_PLACE)) {
    FILE *devnull;
#ifdef WIN32
    devnull = freopen("NUL", "w+", stdout);
#else
    devnull = freopen("/dev/null", "w+", stdout);
#endif
    if (devnull == NULL) {
      fprintf(stderr, "Error: %s opening /dev/null\n", strerror(errno));
      exit(3);
    }
    if (rename(t, input_filenames[0]) == -1) {
      fprintf(stderr, "Error: %s renaming temporary file\n", strerror(errno));
      exit(3);
    }
    jv_mem_free(t);
  }
out:
  jv_mem_free(input_filenames);
  jq_teardown(&jq);
  if (ret >= 10 && (options & EXIT_STATUS))
    return ret - 10;
  if (ret >= 10)
    return 0;
  return ret;
}
