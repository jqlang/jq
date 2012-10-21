#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "compile.h"
#include "builtin.h"
#include "jv.h"
#include "jv_parse.h"
#include "locfile.h"
#include "parser.h"
#include "execute.h"
#include "version.gen.h"

static const char* progname;

static void usage() {
  fprintf(stderr, "\njq - commandline JSON processor [version %s]\n", JQ_VERSION);
  fprintf(stderr, "Usage: %s [options] <jq filter>\n\n", progname);
  fprintf(stderr, "For a description of the command line options and\n");
  fprintf(stderr, "how to write jq filters (and why you might want to)\n");
  fprintf(stderr, "see the jq documentation at http://stedolan.github.com/jq\n\n");
  exit(1);
}

static void die() {
  fprintf(stderr, "Use %s --help for help with command-line options,\n", progname);
  fprintf(stderr, "or see the jq documentation at http://stedolan.github.com/jq\n");
  exit(1);
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
};
static int options = 0;
static struct bytecode* bc;

static void process(jv value) {
  jq_init(bc, value);
  jv result;
  while (jv_is_valid(result = jq_next())) {
    if ((options & RAW_OUTPUT) && jv_get_kind(result) == JV_KIND_STRING) {
      fwrite(jv_string_value(result), 1, jv_string_length(jv_copy(result)), stdout);
    } else {
      int dumpopts = 0;
      if (!(options & COMPACT_OUTPUT)) dumpopts |= JV_PRINT_PRETTY;
      if (options & ASCII_OUTPUT) dumpopts |= JV_PRINT_ASCII;
      jv_dump(result, dumpopts);
    }
    printf("\n");
  }
  jv_free(result);
  jq_teardown();
}

int main(int argc, char* argv[]) {
  if (argc) progname = argv[0];

  const char* program = 0;
  for (int i=1; i<argc; i++) {
    if (!isoptish(argv[i])) {
      if (program) usage();
      program = argv[i];
    } else if (isoption(argv[i], 's', "slurp")) {
      options |= SLURP;
    } else if (isoption(argv[i], 'r', "raw-output")) {
      options |= RAW_OUTPUT;
    } else if (isoption(argv[i], 'c', "compact-output")) {
      options |= COMPACT_OUTPUT;
    } else if (isoption(argv[i], 'a', "ascii-output")) {
      options |= ASCII_OUTPUT;
    } else if (isoption(argv[i], 'R', "raw-input")) {
      options |= RAW_INPUT;
    } else if (isoption(argv[i], 'n', "null-input")) {
      options |= PROVIDE_NULL;
    } else if (isoption(argv[i], 'h', "help")) {
      usage();
    } else if (isoption(argv[i], 'V', "version")) {
      fprintf(stderr, "jq version %s\n", JQ_VERSION);
      return 0;
    } else {
      fprintf(stderr, "%s: Unknown option %s\n", progname, argv[i]);
      die();
    }
  }
  if (!program) usage();

  if ((options & PROVIDE_NULL) && (options & (RAW_INPUT | SLURP))) {
    fprintf(stderr, "%s: --null-input cannot be used with --raw-input or --slurp\n", program);
    die();
  }

  bc = jq_compile(program);
  if (!bc) return 1;

#if JQ_DEBUG
  dump_disassembly(0, bc);
  printf("\n");
#endif

  if (options & PROVIDE_NULL) {
    process(jv_null());
  } else {
    jv slurped;
    if (options & SLURP) slurped = jv_invalid();
    int first = 1;
    struct jv_parser parser;
    jv_parser_init(&parser);
    while (!feof(stdin)) {
      char buf[4096];
      if (!fgets(buf, sizeof(buf), stdin)) buf[0] = 0;
      if (options & RAW_INPUT) {
        int len = strlen(buf);
        if (len > 0) {
          if (options & SLURP) {
            if (first) slurped = jv_string(buf);
            else slurped = jv_string_concat(slurped, jv_string(buf));
          } else {
            if (buf[len-1] == '\n') buf[len-1] = 0;
            process(jv_string(buf));
          }
        }
      } else {
        jv_parser_set_buf(&parser, buf, strlen(buf), !feof(stdin));
        jv value;
        while (jv_is_valid((value = jv_parser_next(&parser)))) {
          if (options & SLURP) {
            if (first) slurped = jv_array();
            slurped = jv_array_append(slurped, value);
          } else {
            process(value);
          }
        }
        if (jv_invalid_has_msg(jv_copy(value))) {
          jv msg = jv_invalid_get_msg(value);
          fprintf(stderr, "parse error: %s\n", jv_string_value(msg));
          jv_free(msg);
          break;
        } else {
          jv_free(value);
        }
      }
      first = 0;
    }
    jv_parser_free(&parser);
    if (options & SLURP) {
      if (jv_is_valid(slurped)) {
        process(slurped);
      } else {
        jv_free(slurped);
      }
    }
  }

  bytecode_free(bc);
  return 0;
}
