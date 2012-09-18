#include <stdio.h>
#include <string.h>
#include "compile.h"
#include "builtin.h"
#include "jv.h"
#include "jv_parse.h"
#include "locfile.h"
#include "parser.h"
#include "execute.h"

int main(int argc, char* argv[]) {
  struct bytecode* bc = jq_compile(argv[1]);
  if (!bc) return 1;

#if JQ_DEBUG
  dump_disassembly(0, bc);
  printf("\n");
#endif

  struct jv_parser parser;
  jv_parser_init(&parser);
  while (!feof(stdin)) {
    char buf[4096];
    if (!fgets(buf, sizeof(buf), stdin)) buf[0] = 0;
    jv_parser_set_buf(&parser, buf, strlen(buf), !feof(stdin));
    jv value;
    while (jv_is_valid((value = jv_parser_next(&parser)))) {
      jq_init(bc, value);
      jv result;
      while (jv_is_valid(result = jq_next())) {
        jv_dump(result, JV_PRINT_PRETTY);
        printf("\n");
      }
      jv_free(result);
      jq_teardown();
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
  jv_parser_free(&parser);

  bytecode_free(bc);
  return 0;
}
