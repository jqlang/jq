#include <stdio.h>
#include "compile.h"
#include "parser.tab.h"
#include "builtin.h"

block compile(const char* str);

void jq_init(struct bytecode* bc, json_t* value);
json_t* jq_next();

void run_program(struct bytecode* bc);

int skipline(const char* buf) {
  int p = 0;
  while (buf[p] == ' ' || buf[p] == '\t') p++;
  if (buf[p] == '#' || buf[p] == '\n' || buf[p] == 0) return 1;
  return 0;
}

void run_tests() {
  FILE* testdata = fopen("testdata","r");
  char buf[4096];
  int tests = 0, passed = 0;

  while (1) {
    if (!fgets(buf, sizeof(buf), testdata)) break;
    if (skipline(buf)) continue;
    printf("Testing %s\n", buf);
    int pass = 1;
    block program = compile(buf);
    block_append(&program, gen_op_simple(YIELD));
    block_append(&program, gen_op_simple(BACKTRACK));
    struct bytecode* bc = block_compile(&builtins, program);
    block_free(program);
    fgets(buf, sizeof(buf), testdata);
    json_t* input = json_loads(buf, JSON_DECODE_ANY, 0);
    jq_init(bc, input);

    while (fgets(buf, sizeof(buf), testdata)) {
      if (skipline(buf)) break;
      json_t* expected = json_loads(buf, JSON_DECODE_ANY, 0);
      json_t* actual = jq_next();
      if (!actual) {
        printf("Insufficient results\n");
        pass = 0;
        break;
      } else if (!json_equal(expected, actual)) {
        printf("Expected ");
        json_dumpf(expected, stdout, JSON_ENCODE_ANY);
        printf(", but got ");
        json_dumpf(actual, stdout, JSON_ENCODE_ANY);
        printf("\n");
        pass = 0;
        break;
      }
    }
    if (pass) {
      json_t* extra = jq_next();
      if (extra) {
        printf("Superfluous result: ");
        json_dumpf(extra, stdout, JSON_ENCODE_ANY);
        printf("\n");
        pass = 0;
      }
    }
    tests++;
    passed+=pass;
  }
  fclose(testdata);
  printf("%d of %d tests passed\n", passed,tests);
}

int main(int argc, char* argv[]) {
  if (argc == 1) { run_tests(); return 0; }
  block blk = compile(argv[1]);
  block_append(&blk, block_join(gen_op_simple(YIELD), gen_op_simple(BACKTRACK)));
  struct bytecode* bc = block_compile(&builtins, blk);
  block_free(blk);
  dump_disassembly(bc);
  printf("\n");
  run_program(bc);
}
