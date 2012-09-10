#include <stdio.h>
#include "compile.h"
#include "parser.tab.h"
#include "builtin.h"
#include "jv.h"

int compile(const char* str, block* answer);

void jq_init(struct bytecode* bc, jv value);
jv jq_next();
void jq_teardown();


void run_program(struct bytecode* bc) {
#if JQ_DEBUG
  dump_disassembly(0, bc);
  printf("\n");
#endif
  char buf[409600];
  fgets(buf, sizeof(buf), stdin);
  jv value = jv_parse(buf);
  if (!jv_is_valid(value)) {
    assert(0 && "couldn't parse input"); //FIXME
  }
  jq_init(bc, value);
  jv result;
  while (jv_is_valid(result = jq_next())) {
    jv_dump(result);
    printf("\n");
  }
  jv_free(result);
  #if JQ_DEBUG
  printf("end of results\n");
  #endif
  jq_teardown();
}

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
    block program;
    int nerrors = compile(buf, &program);
    assert(nerrors == 0);
    block_append(&program, gen_op_simple(YIELD));
    block_append(&program, gen_op_simple(BACKTRACK));
    program = gen_cbinding(&builtins, program);
    struct bytecode* bc = block_compile(program);
    block_free(program);
    printf("Disassembly:\n");
    dump_disassembly(2, bc);
    printf("\n");
    fgets(buf, sizeof(buf), testdata);
    jv input = jv_parse(buf);
    assert(jv_is_valid(input));
    jq_init(bc, input);

    while (fgets(buf, sizeof(buf), testdata)) {
      if (skipline(buf)) break;
      jv expected = jv_parse(buf);
      assert(jv_is_valid(expected));
      jv actual = jq_next();
      if (!jv_is_valid(actual)) {
        jv_free(actual);
        printf("Insufficient results\n");
        pass = 0;
        break;
      } else if (!jv_equal(jv_copy(expected), jv_copy(actual))) {
        printf("Expected ");
        jv_dump(jv_copy(expected));
        printf(", but got ");
        jv_dump(jv_copy(actual));
        printf("\n");
        pass = 0;
      }
      jv_free(expected);
      jv_free(actual);
    }
    if (pass) {
      jv extra = jq_next();
      if (jv_is_valid(extra)) {
        printf("Superfluous result: ");
        jv_dump(extra);
        printf("\n");
        pass = 0;
      } else {
        jv_free(extra);
      }
    }
    jq_teardown();
    bytecode_free(bc);
    tests++;
    passed+=pass;
  }
  fclose(testdata);
  printf("%d of %d tests passed\n", passed,tests);
}

int main(int argc, char* argv[]) {
  if (argc == 1) { run_tests(); return 0; }
  block blk;
  int nerrors = compile(argv[1], &blk);
  if (nerrors > 0) {
    printf("%d compile %s\n", nerrors, nerrors > 1 ? "errors" : "error");
    return 1;
  }
  block_append(&blk, block_join(gen_op_simple(YIELD), gen_op_simple(BACKTRACK)));
  blk = gen_cbinding(&builtins, blk);
  struct bytecode* bc = block_compile(blk);
  block_free(blk);
  run_program(bc);
  bytecode_free(bc);
  return 0;
}
