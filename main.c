#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "compile.h"
#include "jv.h"
#include "jq.h"
#include "jv_alloc.h"
#include "version.h"

int jq_testsuite(int argc, char* argv[]);

static const char* progname;

static void usage() {
  fprintf(stderr, "\njq - commandline JSON processor [version %s]\n", JQ_VERSION);
  fprintf(stderr, "Usage: %s [options] <jq filter> [file...]\n\n", progname);
  fprintf(stderr, "For a description of the command line options and\n");
  fprintf(stderr, "how to write jq filters (and why you might want to)\n");
  fprintf(stderr, "see the jq manpage, or the online documentation at\n");
  fprintf(stderr, "http://stedolan.github.com/jq\n\n");
  exit(2);
}

static void die() {
  fprintf(stderr, "Use %s --help for help with command-line options,\n", progname);
  fprintf(stderr, "or see the jq documentation at http://stedolan.github.com/jq\n");
  exit(2);
}




static int isoptish(const char* text) {
  return text[0] == '-' && (text[1] == '-' || isalpha(text[1]));
}

static int isoption(const char* text, char shortopt, const char* longopt) {
  if (text[0] != '-') return 0;
  if (strlen(text) == 2 && text[1] == shortopt) return 1;
  if (text[1] == '-' && !strcmp(text+2, longopt)) return 1;
  return 0;
}

enum {
  SLURP = 1,
  RAW_INPUT = 2,
  PROVIDE_NULL = 4,

  RAW_OUTPUT = 8,
  COMPACT_OUTPUT = 16,
  ASCII_OUTPUT = 32,
  COLOUR_OUTPUT = 64,
  NO_COLOUR_OUTPUT = 128,
  SORTED_OUTPUT = 256,
  UNBUFFERED_OUTPUT = 4096,

  FROM_FILE = 512,

  EXIT_STATUS = 8192,

  /* debugging only */
  DUMP_DISASM = 2048,
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
    printf("\n");
    if (options & UNBUFFERED_OUTPUT)
      fflush(stdout);
  }
  jv_free(result);
  return ret;
}

FILE* current_input;
const char** input_filenames = NULL;
int ninput_files;
int next_input_idx;
static int read_more(char* buf, size_t size) {
  while (!current_input || feof(current_input)) {
    if (current_input) {
      fclose(current_input);
      current_input = 0;
    }
    if (next_input_idx == ninput_files) {
      return 0;
    }
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
  return 1;
}

int main(int argc, char* argv[]) {
  jq_state *jq = NULL;
  int ret = 0;
  int compiled = 0;

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
  jv program_arguments = jv_array();
  for (int i=1; i<argc; i++) {
    if (further_args_are_files) {
      input_filenames[ninput_files++] = argv[i];
    } else if (!strcmp(argv[i], "--")) {
      if (!program) usage();
      further_args_are_files = 1;
    } else if (!isoptish(argv[i])) {
      if (program) {
        input_filenames[ninput_files++] = argv[i];
      } else {
        program = argv[i];
      }
    } else if (isoption(argv[i], 's', "slurp")) {
      options |= SLURP;
    } else if (isoption(argv[i], 'r', "raw-output")) {
      options |= RAW_OUTPUT;
    } else if (isoption(argv[i], 'c', "compact-output")) {
      options |= COMPACT_OUTPUT;
    } else if (isoption(argv[i], 'C', "color-output")) {
      options |= COLOUR_OUTPUT;
    } else if (isoption(argv[i], 'M', "monochrome-output")) {
      options |= NO_COLOUR_OUTPUT;
    } else if (isoption(argv[i], 'a', "ascii-output")) {
      options |= ASCII_OUTPUT;
    } else if (isoption(argv[i], 0, "unbuffered")) {
      options |= UNBUFFERED_OUTPUT;
    } else if (isoption(argv[i], 'S', "sort-keys")) {
      options |= SORTED_OUTPUT;
    } else if (isoption(argv[i], 'R', "raw-input")) {
      options |= RAW_INPUT;
    } else if (isoption(argv[i], 'n', "null-input")) {
      options |= PROVIDE_NULL;
    } else if (isoption(argv[i], 'f', "from-file")) {
      options |= FROM_FILE;
    } else if (isoption(argv[i], 'e', "exit-status")) {
      options |= EXIT_STATUS;
    } else if (isoption(argv[i], 0, "arg")) {
      if (i >= argc - 2) {
        fprintf(stderr, "%s: --arg takes two parameters (e.g. -a varname value)\n", progname);
        die();
      }
      jv arg = jv_object();
      arg = jv_object_set(arg, jv_string("name"), jv_string(argv[i+1]));
      arg = jv_object_set(arg, jv_string("value"), jv_string(argv[i+2]));
      program_arguments = jv_array_append(program_arguments, arg);
      i += 2; // skip the next two arguments
    } else if (isoption(argv[i], 0, "argfile")) {
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
    } else if (isoption(argv[i],  0,  "debug-dump-disasm")) {
      options |= DUMP_DISASM;
    } else if (isoption(argv[i],  0,  "debug-trace")) {
      jq_flags |= JQ_DEBUG_TRACE;
    } else if (isoption(argv[i], 'h', "help")) {
      usage();
    } else if (isoption(argv[i], 'V', "version")) {
      printf("jq-%s\n", JQ_VERSION);
      ret = 0;
      goto out;
    } else {
      fprintf(stderr, "%s: Unknown option %s\n", progname, argv[i]);
      die();
    }
  }
  if (!program) usage();
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
    while (read_more(buf, sizeof(buf))) {
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
        jv_parser_set_buf(parser, buf, strlen(buf), !feof(stdin));
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
out:
  jv_mem_free(input_filenames);
  jq_teardown(&jq);
  if (ret >= 10 && (options & EXIT_STATUS))
    return ret - 10;
  if (ret >= 10)
    return 0;
  return ret;
}
