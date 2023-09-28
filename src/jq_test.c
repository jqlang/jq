#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif
#include "jv.h"
#include "jq.h"

static void jv_test();
static void run_jq_tests(jv, int, FILE *, int, int);
#ifdef HAVE_PTHREAD
static void run_jq_pthread_tests();
#endif

int jq_testsuite(jv libdirs, int verbose, int argc, char* argv[]) {
  FILE *testdata = stdin;
  int skip = -1;
  int take = -1;
  jv_test();
  if (argc > 0) {
    for(int i = 0; i < argc; i++) {
      if (!strcmp(argv[i], "--skip")) {
        skip = atoi(argv[i+1]);
        i++;
      } else if (!strcmp(argv[i], "--take")) {
        take = atoi(argv[i+1]);
        i++;
      } else {
        testdata = fopen(argv[i], "r");
        if (!testdata) {
          perror("fopen");
          exit(1);
        }
      }
    }
  }
  run_jq_tests(libdirs, verbose, testdata, skip, take);
#ifdef HAVE_PTHREAD
  run_jq_pthread_tests();
#endif
  return 0;
}

static int skipline(const char* buf) {
  int p = 0;
  while (buf[p] == ' ' || buf[p] == '\t') p++;
  if (buf[p] == '#' || buf[p] == '\n' || buf[p] == 0) return 1;
  return 0;
}

static int checkerrormsg(const char* buf) {
  return strcmp(buf, "%%FAIL\n") == 0;
}

static int checkfail(const char* buf) {
  return strcmp(buf, "%%FAIL\n") == 0 || strcmp(buf, "%%FAIL IGNORE MSG\n") == 0;
}

struct err_data {
  char buf[4096];
};

static void test_err_cb(void *data, jv e) {
  struct err_data *err_data = data;
  if (jv_get_kind(e) != JV_KIND_STRING)
    e = jv_dump_string(e, JV_PRINT_INVALID);
  if (!strncmp(jv_string_value(e), "jq: error", sizeof("jq: error") - 1))
    snprintf(err_data->buf, sizeof(err_data->buf), "%s", jv_string_value(e));
  if (strchr(err_data->buf, '\n'))
    *(strchr(err_data->buf, '\n')) = '\0';
  jv_free(e);
}

static void run_jq_tests(jv lib_dirs, int verbose, FILE *testdata, int skip, int take) {
  char prog[4096];
  char buf[4096];
  struct err_data err_msg;
  int tests = 0, passed = 0, invalid = 0;
  unsigned int lineno = 0;
  int must_fail = 0;
  int check_msg = 0;
  jq_state *jq = NULL;

  int tests_to_skip = skip > 0 ? skip : 0;
  int tests_to_take = take;

  jq = jq_init();
  assert(jq);
  if (jv_get_kind(lib_dirs) == JV_KIND_NULL)
    lib_dirs = jv_array();
  jq_set_attr(jq, jv_string("JQ_LIBRARY_PATH"), lib_dirs);

  while (1) {
    if (!fgets(prog, sizeof(prog), testdata)) break;
    lineno++;
    if (skipline(prog)) continue;
    if (checkfail(prog)) {
      must_fail = 1;
      check_msg = checkerrormsg(prog);
      jq_set_error_cb(jq, test_err_cb, &err_msg);
      continue;
    }
    if (prog[strlen(prog)-1] == '\n') prog[strlen(prog)-1] = 0;

    if (skip > 0) {
      skip--;

      // skip past test data
      while (fgets(buf, sizeof(buf), testdata)) {
        lineno++;
        if (buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n'))
          break;
      }

      must_fail = 0;
      check_msg = 0;

      continue;
    } else if (skip == 0) {
      printf("Skipped %d tests\n", tests_to_skip);
      skip = -1;
    }

    if (take > 0) {
      take--;
    } else if (take == 0) {
      printf("Hit the number of tests limit (%d), breaking\n", tests_to_take);
      break;
    }

    int pass = 1;
    tests++;
    printf("Test #%d: '%s' at line number %u\n", tests + tests_to_skip, prog, lineno);
    int compiled = jq_compile(jq, prog);

    if (must_fail) {
      jq_set_error_cb(jq, NULL, NULL);
      if (!fgets(buf, sizeof(buf), testdata)) { invalid++; break; }
      lineno++;
      if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = 0;
      if (compiled) {
        printf("*** Test program compiled that should not have at line %u: %s\n", lineno, prog);
        must_fail = 0;
        check_msg = 0;
        invalid++;
        continue;
      }
      if (check_msg && strcmp(buf, err_msg.buf) != 0) {
        printf("*** Erroneous test program failed with wrong message (%s) at line %u: %s\n", err_msg.buf, lineno, prog);
        invalid++;
      } else {
        passed++;
      }
      must_fail = 0;
      check_msg = 0;
      continue;
    }

    if (!compiled) {
      printf("*** Test program failed to compile at line %u: %s\n", lineno, prog);
      invalid++;
      // skip past test data
      while (fgets(buf, sizeof(buf), testdata)) {
        lineno++;
        if (buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n'))
          break;
      }
      continue;
    }
    if (verbose) {
      printf("Disassembly:\n");
      jq_dump_disassembly(jq, 2);
      printf("\n");
    }
    if (!fgets(buf, sizeof(buf), testdata)) { invalid++; break; }
    lineno++;
    jv input = jv_parse(buf);
    if (!jv_is_valid(input)) {
      printf("*** Input is invalid on line %u: %s\n", lineno, buf);
      invalid++;
      continue;
    }
    jq_start(jq, input, verbose ? JQ_DEBUG_TRACE : 0);

    while (fgets(buf, sizeof(buf), testdata)) {
      lineno++;
      if (skipline(buf)) break;
      jv expected = jv_parse(buf);
      if (!jv_is_valid(expected)) {
        printf("*** Expected result is invalid on line %u: %s\n", lineno, buf);
        invalid++;
        continue;
      }
      jv actual = jq_next(jq);
      if (!jv_is_valid(actual)) {
        jv_free(expected);
        jv_free(actual);
        printf("*** Insufficient results for test at line number %u: %s\n", lineno, prog);
        pass = 0;
        break;
      } else if (!jv_equal(jv_copy(expected), jv_copy(actual))) {
        printf("*** Expected ");
        jv_dump(jv_copy(expected), 0);
        printf(", but got ");
        jv_dump(jv_copy(actual), 0);
        printf(" for test at line number %u: %s\n", lineno, prog);
        pass = 0;
      }
      jv as_string = jv_dump_string(jv_copy(expected), rand() & ~(JV_PRINT_COLOR|JV_PRINT_REFCOUNT));
      jv reparsed = jv_parse_sized(jv_string_value(as_string), jv_string_length_bytes(jv_copy(as_string)));
      assert(jv_equal(jv_copy(expected), jv_copy(reparsed)));
      jv_free(as_string);
      jv_free(reparsed);
      jv_free(expected);
      jv_free(actual);
    }
    if (pass) {
      jv extra = jq_next(jq);
      if (jv_is_valid(extra)) {
        printf("*** Superfluous result: ");
        jv_dump(extra, 0);
        printf(" for test at line number %u, %s\n", lineno, prog);
        pass = 0;
      } else {
        jv_free(extra);
      }
    }
    passed+=pass;
  }
  jq_teardown(&jq);

  int total_skipped = tests_to_skip;

  if (skip > 0) {
    total_skipped = tests_to_skip - skip;
  }

  printf("%d of %d tests passed (%d malformed, %d skipped)\n",
    passed, tests, invalid, total_skipped);

  if (skip > 0) {
    printf("WARN: skipped past the end of file, exiting with status 2\n");
    exit(2);
  }

  if (passed != tests) exit(1);
}


/// pthread regression test
#ifdef HAVE_PTHREAD
#define NUMBER_OF_THREADS 3
struct test_pthread_data {
    int result;
};

static int test_pthread_jq_parse(jq_state *jq, struct jv_parser *parser)
{
    int rv = 0;
    jv value;

    value = jv_parser_next(parser);
    while (jv_is_valid(value)) {
        jq_start(jq, value, 0);
        jv result = jq_next(jq);

        while (jv_is_valid(result)) {
            jv_free(result);
            result = jq_next(jq);
        }
        jv_free(result);
        value = jv_parser_next(parser);
    }
    jv_free(value);
    return rv;
}

static void *test_pthread_run(void *ptr) {
    int rv;
    jq_state *jq;
    const char *prg = ".data";
    const char *buf = "{ \"data\": 1 }";
    struct test_pthread_data *data = ptr;

    jq = jq_init();
    if (jq_compile(jq, prg) == 0) {
        jq_teardown(&jq);
        return NULL;
    }

    struct jv_parser *parser = jv_parser_new(0);
    jv_parser_set_buf(parser, buf, strlen(buf), 0);
    rv = test_pthread_jq_parse(jq, parser);

    data->result = rv;

    jv_parser_free(parser);
    jq_teardown(&jq);
    return NULL;
}

static void run_jq_pthread_tests() {
    pthread_t threads[NUMBER_OF_THREADS];
    struct test_pthread_data data[NUMBER_OF_THREADS];
    int createerror;
    int a;

    memset(&threads, 0, sizeof(threads));
    memset(&data, 0, sizeof(data));

    // Create all threads
    for (a = 0; a < NUMBER_OF_THREADS; ++a) {
        createerror = pthread_create(&threads[a], NULL, test_pthread_run, &data[a]);
        assert(createerror == 0);
    }

    // wait for all threads
    for(a = 0; a < NUMBER_OF_THREADS; ++a) {
        if (threads[a] != 0) {
            pthread_join(threads[a], NULL);
        }
    }

    // check results
    for(a = 0; a < NUMBER_OF_THREADS; ++a) {
        assert(data[a].result == 0);
    }
}
#endif // HAVE_PTHREAD


static void jv_test() {
  /// JSON parser regression tests
  {
    jv v = jv_parse("{\"a':\"12\"}");
    assert(jv_get_kind(v) == JV_KIND_INVALID);
    v = jv_invalid_get_msg(v);
    assert(strcmp(jv_string_value(v), "Expected separator between values at line 1, column 9 (while parsing '{\"a':\"12\"}')") == 0);
    jv_free(v);
  }
  /// Arrays and numbers
  {
    jv a = jv_array();
    assert(jv_get_kind(a) == JV_KIND_ARRAY);
    assert(jv_array_length(jv_copy(a)) == 0);
    assert(jv_array_length(jv_copy(a)) == 0);

    a = jv_array_append(a, jv_number(42));
    assert(jv_array_length(jv_copy(a)) == 1);
    assert(jv_number_value(jv_array_get(jv_copy(a), 0)) == 42);

    jv a2 = jv_array_append(jv_array(), jv_number(42));
    assert(jv_equal(jv_copy(a), jv_copy(a)));
    assert(jv_equal(jv_copy(a2), jv_copy(a2)));
    assert(jv_equal(jv_copy(a), jv_copy(a2)));
    assert(jv_equal(jv_copy(a2), jv_copy(a)));
    jv_free(a2);

    a2 = jv_array_append(jv_array(), jv_number(19));
    assert(!jv_equal(jv_copy(a), jv_copy(a2)));
    assert(!jv_equal(jv_copy(a2), jv_copy(a)));
    jv_free(a2);


    assert(jv_get_refcnt(a) == 1);
    a = jv_array_append(a, jv_copy(a));
    assert(jv_get_refcnt(a) == 1);

    assert(jv_array_length(jv_copy(a)) == 2);
    assert(jv_number_value(jv_array_get(jv_copy(a), 0)) == 42);

    for (int i=0; i<10; i++) {
      jv subarray = jv_array_get(jv_copy(a), 1);
      assert(jv_get_kind(subarray) == JV_KIND_ARRAY);
      assert(jv_array_length(jv_copy(subarray)) == 1);
      assert(jv_number_value(jv_array_get(jv_copy(subarray), 0)) == 42);
      jv_free(subarray);
    }


    jv subarray = jv_array_get(jv_copy(a), 1);
    assert(jv_get_kind(subarray) == JV_KIND_ARRAY);
    assert(jv_array_length(jv_copy(subarray)) == 1);
    assert(jv_number_value(jv_array_get(jv_copy(subarray), 0)) == 42);

    jv sub2 = jv_copy(subarray);
    sub2 = jv_array_append(sub2, jv_number(19));

    assert(jv_get_kind(sub2) == JV_KIND_ARRAY);
    assert(jv_array_length(jv_copy(sub2)) == 2);
    assert(jv_number_value(jv_array_get(jv_copy(sub2), 0)) == 42);
    assert(jv_number_value(jv_array_get(jv_copy(sub2), 1)) == 19);

    assert(jv_get_kind(subarray) == JV_KIND_ARRAY);
    assert(jv_array_length(jv_copy(subarray)) == 1);
    assert(jv_number_value(jv_array_get(jv_copy(subarray), 0)) == 42);

    jv_free(subarray);

    void* before = sub2.u.ptr;
    sub2 = jv_array_append(sub2, jv_number(200));
    void* after = sub2.u.ptr;
    assert(before == after);
    jv_free(sub2);

    jv a3 = jv_array_append(jv_copy(a), jv_number(19));
    assert(jv_array_length(jv_copy(a3)) == 3);
    assert(jv_number_value(jv_array_get(jv_copy(a3), 0)) == 42);
    assert(jv_array_length(jv_array_get(jv_copy(a3), 1)) == 1);
    assert(jv_number_value(jv_array_get(jv_copy(a3), 2)) == 19);
    jv_free(a3);


    jv a4 = jv_array();
    a4 = jv_array_append(a4, jv_number(1));
    a4 = jv_array_append(a4, jv_number(2));
    jv a5 = jv_copy(a4);
    a4 = jv_array_append(a4, jv_number(3));
    a4 = jv_array_slice(a4, 0, 1);
    assert(jv_array_length(jv_copy(a4)) == 1);
    a4 = jv_array_append(a4, jv_number(4));
    assert(jv_array_length(jv_copy(a4)) == 2);
    assert(jv_array_length(jv_copy(a5)) == 2);
    jv_free(a4);
    jv_free(a5);


    assert(jv_array_length(jv_copy(a)) == 2);
    assert(jv_number_value(jv_array_get(jv_copy(a), 0)) == 42);
    assert(jv_array_length(jv_array_get(jv_copy(a), 1)) == 1);


    //jv_dump(jv_copy(a), 0); printf("\n");
    jv_free(a);
  }


  /// Strings
  {
    assert(jv_equal(jv_string("foo"), jv_string_sized("foo", 3)));
    char nasty[] = "foo\0";
    jv shortstr = jv_string(nasty), longstr = jv_string_sized(nasty, sizeof(nasty));
    assert(jv_string_length_bytes(jv_copy(shortstr)) == (int)strlen(nasty));
    assert(jv_string_length_bytes(jv_copy(longstr)) == (int)sizeof(nasty));
    jv_free(shortstr);
    jv_free(longstr);


    char a1s[] = "hello", a2s[] = "hello", bs[] = "goodbye";
    jv a1 = jv_string(a1s), a2 = jv_string(a2s), b = jv_string(bs);
    assert(jv_equal(jv_copy(a1), jv_copy(a2)));
    assert(jv_equal(jv_copy(a2), jv_copy(a1)));
    assert(!jv_equal(jv_copy(a1), jv_copy(b)));

    assert(jv_string_hash(jv_copy(a1)) == jv_string_hash(jv_copy(a1)));
    assert(jv_string_hash(jv_copy(a1)) == jv_string_hash(jv_copy(a2)));
    assert(jv_string_hash(jv_copy(b)) != jv_string_hash(jv_copy(a1)));
    jv_free(a1);
    jv_free(a2);
    jv_free(b);

    assert(jv_equal(jv_string("hello42!"), jv_string_fmt("hello%d%s", 42, "!")));
    char big[20000];
    for (int i=0; i<(int)sizeof(big); i++) big[i] = 'a';
    big[sizeof(big)-1] = 0;
    jv str = jv_string_fmt("%s", big);
    assert(jv_string_length_bytes(jv_copy(str)) == sizeof(big) - 1);
    assert(!strcmp(big, jv_string_value(str)));
    jv_free(str);
  }

  /// Objects
  {
    jv o1 = jv_object();
    o1 = jv_object_set(o1, jv_string("foo"), jv_number(42));
    o1 = jv_object_set(o1, jv_string("bar"), jv_number(24));
    assert(jv_number_value(jv_object_get(jv_copy(o1), jv_string("foo"))) == 42);
    assert(jv_number_value(jv_object_get(jv_copy(o1), jv_string("bar"))) == 24);

    jv o2 = jv_object_set(jv_copy(o1), jv_string("foo"), jv_number(420));
    o2 = jv_object_set(o2, jv_string("bar"), jv_number(240));
    assert(jv_number_value(jv_object_get(jv_copy(o1), jv_string("foo"))) == 42);
    assert(jv_number_value(jv_object_get(jv_copy(o1), jv_string("bar"))) == 24);
    assert(jv_number_value(jv_object_get(jv_copy(o2), jv_string("foo"))) == 420);
    jv_free(o1);
    assert(jv_number_value(jv_object_get(jv_copy(o2), jv_string("bar"))) == 240);

    //jv_dump(jv_copy(o2), 0); printf("\n");
    jv_free(o2);
  }
}
