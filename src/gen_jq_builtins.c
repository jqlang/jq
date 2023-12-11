#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "main-win32.h"
#include "math-macos.h"

static uint_least8_t index = 0;

static bool indentation(void) {
  if (index == 0 && fputs("  ", stdout) == EOF) {
    perror("fputs");
    return false;
  }
  return true;
}

static bool separator(void) {
  if (putchar(index < 10 ? ' ' : '\n') == EOF) {
    perror("putchar");
    return false;
  }
  index = (index + 1) % 11;
  return true;
}

static bool gen_char(char const ch) {
  if (!indentation())
    return false;
  if (printf("%.4o,", (unsigned)ch) == EOF)
    return false;
  return separator();
}

static bool gen_string(char const *const string)
{
  for (char const *ch = string; *ch; ++ch) {
    if (!gen_char(*ch))
      return false;
  }
  return true;
}

static bool gen_file(FILE *const fp) {
  for (int ch; (ch = getc(fp)) != EOF;) {
    if (!gen_char(ch))
      return false;
  }
  if (ferror(fp)) {
    perror("getc");
    return false;
  }
  return true;
}

DEFINE_MAIN(int argc, char *const argv[]) {
  /* argv[1] must be the path to "src/builtin.jq" */
  if (argc != 2) {
    static char const err[] =
      "gen_builtin_inc: Wrong number of arguments.\n";
    if (fputs(err, stderr) == EOF)
      perror("fputs");
    return EXIT_FAILURE;
  }

  if (puts("{") == EOF) {
    perror("puts");
    return EXIT_FAILURE;
  }

  FILE *const builtin_jq = fopen(argv[1], "r");
  if (!builtin_jq) {
    perror("fopen");
    return EXIT_FAILURE;
  }
  if (!gen_file(builtin_jq))
    return EXIT_FAILURE;

#define GEN_STRING(string) \
  do { \
    if (!gen_string(string)) \
      return EXIT_FAILURE; \
  } while (0)

  /* Unsupported math functions */
#define LIBM_DD(name)
#define LIBM_DDD(name)
#define LIBM_DDDD(name)
#define LIBM_DD_NO(name) \
  gen_string( \
    "def " #name ":" \
      "\"Error: " #name "/0 not found at build time\"|error;");
#define LIBM_DDD_NO(name) \
  gen_string( \
    "def " #name "(a;b):" \
      "\"Error: " #name "/2 not found at build time\"|error;");
#define LIBM_DDDD_NO(name) \
  gen_string("def " #name "(a;b;c):" \
    "\"Error: " #name "/3 not found at build time\"|error;");
#include "libm.h"
#undef LIBM_DD
#undef LIBM_DDD
#undef LIBM_DDDD
#undef LIBM_DD_NO
#undef LIBM_DDD_NO
#undef LIBM_DDDD_NO

#ifndef HAVE_FREXP
  GEN_STRING(
    "def frexp:"
      "\"Error: frexp/0 not found at build time\"|error;");
#endif
#ifndef HAVE_MODF
  GEN_STRING(
    "def modf:"
      "\"Error: modf/0 not found at build time\"|error;");
#endif
#ifndef HAVE_LGAMMA_R
  GEN_STRING(
    "def lgamma_r:"
      "\"Error: lgamma_r/0 not found at build time\"|error;");
#endif

#undef GEN_STRING

  if (puts("'\\0',\n}") == EOF) {
    perror("puts");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
