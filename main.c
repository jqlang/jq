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
  fprintf(f, "\t -i\t\tedit the [first] file in-place;\n");
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
  IN_PLACE              = 8192,
  SEQ                   = 16384,
  RUN_TESTS             = 32768,
  /* debugging only */
  DUMP_DISASM           = 65536,
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
    jv_free(msg);
  }
  jv_free(result);
  return ret;
}

// XXX Move this and related functions into libjq (with a better name), into util.[ch] say
struct next_input_state {
  FILE* current_input;
  const char** input_filenames;
  int alloced;
  int ninput_files;
  int next_input_idx;
  int open_failures;
  struct jv_parser *parser;
  jv slurped;
  char buf[4096];
};
typedef struct next_input_state *next_input_state;

static void input_state_free(next_input_state *state) {
  next_input_state old_state = *state;
  *state = NULL;
  if (old_state == NULL)
    return;
  if (old_state->parser != NULL)
    jv_parser_free(old_state->parser);
  jv_mem_free(old_state->input_filenames);
  jv_free(old_state->slurped);
  jv_mem_free(old_state);
}

static int input_state_init(next_input_state *state, int max_inputs) {
  next_input_state new_state = jv_mem_alloc(sizeof(*new_state));
  new_state->next_input_idx = 0;
  new_state->open_failures = 0;
  new_state->ninput_files = 0;
  new_state->current_input = NULL;
  new_state->parser = NULL;         // initialized when we know the flags
  new_state->slurped = jv_invalid();
  new_state->buf[0] = 0;

  // XXX a jv_mem_calloc() would be nice
  assert(max_inputs > 0 && (uintmax_t)max_inputs * sizeof(const char*) < SIZE_MAX);
  new_state->input_filenames = jv_mem_alloc(sizeof(const char*) * max_inputs);
  new_state->alloced = max_inputs;
  for (; max_inputs > 0; max_inputs--) 
    new_state->input_filenames[max_inputs - 1] = NULL;
  *state = new_state;
  return 0;
}

static void input_state_add_input(next_input_state state, const char *input) {
  assert(state->ninput_files < state->alloced);
  state->input_filenames[state->ninput_files++] = input;
}

static int input_state_read_more(next_input_state state) {
  if (!state->current_input || feof(state->current_input)) {
    if (state->current_input) {
      if (state->current_input == stdin) {
        clearerr(stdin); // perhaps we can read again; anyways, we don't fclose(stdin)
      } else {
        fclose(state->current_input);
      }
      state->current_input = NULL;
    }
    if (state->next_input_idx < state->ninput_files) {
      if (!strcmp(state->input_filenames[state->next_input_idx], "-")) {
        state->current_input = stdin;
      } else {
        state->current_input = fopen(state->input_filenames[state->next_input_idx], "r");
        if (!state->current_input) {
          fprintf(stderr, "%s: Error: could not open %s: %s\n", progname, state->input_filenames[state->next_input_idx], strerror(errno));
          state->open_failures++;
        }
      }
      state->next_input_idx++;
    }
  }

  state->buf[0] = 0;
  if (state->current_input) {
    if (!fgets(state->buf, sizeof(state->buf), state->current_input))
      state->buf[0] = 0;
  }
  return state->next_input_idx == state->ninput_files && (!state->current_input || feof(state->current_input));
}

// Blocks to read one more input from stdin and/or given files
// When slurping, it returns just one value
static jv input_state_next_input(jq_state *jq, void *data) {
  next_input_state state = data;
  int is_last = 0;
  jv value = jv_invalid(); // need more input
  do {
    if (options & RAW_INPUT) {
      is_last = input_state_read_more(state);
      if (state->buf[0] == '\0')
        continue;
      int len = strlen(state->buf); // Raw input doesn't support NULs
      if (len > 0) {
        if (options & SLURP) {
          state->slurped = jv_string_concat(state->slurped, jv_string(state->buf));
        } else {
          if (!jv_is_valid(value))
            value = jv_string("");
          if (state->buf[len-1] == '\n') {
            // whole line
            state->buf[len-1] = 0;
            return jv_string_concat(value, jv_string(state->buf));
          }
          value = jv_string_concat(value, jv_string(state->buf));
          state->buf[0] = '\0';
        }
      }
    } else {
      if (jv_parser_remaining(state->parser) == 0) {
        is_last = input_state_read_more(state);
        jv_parser_set_buf(state->parser, state->buf, strlen(state->buf), !is_last); // NULs also not supported here
      }
      value = jv_parser_next(state->parser);
      if (options & SLURP) {
        if (jv_is_valid(value)) {
          state->slurped = jv_array_append(state->slurped, value);
          value = jv_invalid();
        } else if (jv_invalid_has_msg(jv_copy(value)))
          return value;
      } else if (jv_is_valid(value) || jv_invalid_has_msg(jv_copy(value))) {
        return value;
      }
    }
  } while (!is_last);
  return value;
}
// XXX End of stuff to move into utils

static void debug_cb(jq_state *jq, void *data, jv input) {
  int dumpopts = *(int *)data;
  jv_dumpf(JV_ARRAY(jv_string("DEBUG:"), input), stderr, dumpopts & ~(JV_PRINT_PRETTY));
  fprintf(stderr, "\n");
}

int main(int argc, char* argv[]) {
  jq_state *jq = NULL;
  int ret = 0;
  int compiled = 0;
  int parser_flags = 0;
  char *t = NULL;

  if (argc) progname = argv[0];

  jq = jq_init();
  if (jq == NULL) {
    perror("malloc");
    ret = 2;
    goto out;
  }

  const char* program = 0;

  next_input_state input_state;
  input_state_init(&input_state, argc);

  int further_args_are_files = 0;
  int jq_flags = 0;
  size_t short_opts = 0;
  jv program_arguments = jv_array();
  jv lib_search_paths = jv_null();
  for (int i=1; i<argc; i++, short_opts = 0) {
    if (further_args_are_files) {
      input_state_add_input(input_state, argv[i]);
    } else if (!strcmp(argv[i], "--")) {
      if (!program) usage(2);
      further_args_are_files = 1;
    } else if (!isoptish(argv[i])) {
      if (program) {
        input_state_add_input(input_state, argv[i]);
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
      if (isoption(argv[i], 'i', "in-place", &short_opts)) {
        options |= IN_PLACE;
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
  if ((options & IN_PLACE)) {
    if (input_state->ninput_files == 0) usage(2);
    if (strcmp(input_state->input_filenames[0], "-") == 0) usage(2);
    size_t tlen = strlen(input_state->input_filenames[0]) + 7;
    t = jv_mem_alloc(tlen);
    int n = snprintf(t, tlen,"%sXXXXXX", input_state->input_filenames[0]);
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

  if (input_state->ninput_files == 0) input_state->current_input = stdin;
  input_state->parser = jv_parser_new(parser_flags);
  if ((options & RAW_INPUT) && (options & SLURP))
    input_state->slurped = jv_string("");
  else if ((options & SLURP))
    input_state->slurped = jv_array();
  else
    input_state->slurped = jv_invalid();

  // Let jq program read from inputs
  jq_set_input_cb(jq, input_state_next_input, input_state);

  jq_set_debug_cb(jq, debug_cb, &dumpopts);

  if (options & PROVIDE_NULL) {
    ret = process(jq, jv_null(), jq_flags, dumpopts);
  } else {
    jv value;
    while (input_state->open_failures == 0 &&
           (jv_is_valid((value = input_state_next_input(jq, input_state))) || jv_invalid_has_msg(jv_copy(value)))) {
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
    if (options & SLURP) {
      ret = process(jq, input_state->slurped, jq_flags, dumpopts);
      input_state->slurped = jv_invalid();
    }
  }

  if (ret == 0 && input_state->open_failures != 0)
    ret = 2;

  if (ret != 0)
    goto out;

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
    assert(input_state->ninput_files > 0 && !strcmp(input_state->input_filenames[0], "-"));
    if (rename(t, input_state->input_filenames[0]) == -1) {
      fprintf(stderr, "Error: %s renaming temporary file\n", strerror(errno));
      exit(3);
    }
    jv_mem_free(t);
  }
out:
  input_state_free(&input_state);
  jq_teardown(&jq);
  if (ret >= 10 && (options & EXIT_STATUS))
    return ret - 10;
  if (ret >= 10)
    return 0;
  return ret;
}
