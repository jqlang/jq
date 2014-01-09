#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jv.h"
#include "jq.h"

extern int main(int, char **);

static void jv_test();
static void run_jq_tests();


int jq_testsuite(int argc, char* argv[]) {
  FILE *testdata = stdin;
  jv_test();
  if (argc > 2) {
    testdata = fopen(argv[2], "r");
    if (!testdata) {
      perror("fopen");
      exit(1);
    }
  }
  run_jq_tests(testdata);
  return 0;
}

static int skipline(const char* buf) {
  int p = 0;
  while (buf[p] == ' ' || buf[p] == '\t') p++;
  if (buf[p] == '#' || buf[p] == '\n' || buf[p] == 0) return 1;
  return 0;
}

static void run_jq_tests(FILE *testdata) {
  char buf[4096];
  int tests = 0, passed = 0, invalid = 0;
  jq_state *jq = NULL;

  jq = jq_init();
  assert(jq);

  while (1) {
    if (!fgets(buf, sizeof(buf), testdata)) break;
    if (skipline(buf)) continue;
    if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = 0;
    printf("Testing %s\n", buf);
    int pass = 1;
    tests++;
    int compiled = jq_compile(jq, buf);
    if (!compiled) {invalid++; continue;}
    printf("Disassembly:\n");
    jq_dump_disassembly(jq, 2);
    printf("\n");
    if (!fgets(buf, sizeof(buf), testdata)) { invalid++; break; }
    jv input = jv_parse(buf);
    if (!jv_is_valid(input)){ invalid++; continue; }
    jq_start(jq, input, JQ_DEBUG_TRACE);

    while (fgets(buf, sizeof(buf), testdata)) {
      if (skipline(buf)) break;
      jv expected = jv_parse(buf);
      if (!jv_is_valid(expected)){ invalid++; continue; }
      jv actual = jq_next(jq);
      if (!jv_is_valid(actual)) {
        jv_free(actual);
        printf("*** Insufficient results\n");
        pass = 0;
        break;
      } else if (!jv_equal(jv_copy(expected), jv_copy(actual))) {
        printf("*** Expected ");
        jv_dump(jv_copy(expected), 0);
        printf(", but got ");
        jv_dump(jv_copy(actual), 0);
        printf("\n");
        pass = 0;
      }
      jv as_string = jv_dump_string(jv_copy(expected), rand() & ~JV_PRINT_COLOUR);
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
        printf("\n");
        pass = 0;
      } else {
        jv_free(extra);
      }
    }
    passed+=pass;
  }
  jq_teardown(&jq);
  printf("%d of %d tests passed (%d malformed)\n", passed,tests,invalid);
  if (passed != tests) exit(1);
}


static void jv_test() {
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


    jv_dump(jv_copy(a), 0); printf("\n");
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

    jv_dump(jv_copy(o2), 0); printf("\n");

    jv_free(o2);
  }

  /// Compile errors
  {
    jq_state *jq = jq_init();
    int compiled = jq_compile_args(jq, "}", 0, jv_array());
    assert(!compiled);
    jq_teardown(&jq);
  }

  // while/until and I/O
  {
    FILE *in = tmpfile();
    FILE *out = tmpfile();
    fprintf(in, "0\n\"foo\"\n2\n");
    fflush(in);
    rewind(in);

    /*
     * Test stdio I/O handles, read, write, eof, until.
     *
     * Roughly like:
     *
     *     % printf '0\n"foo"\n2\n' | jq -n 'until(eof(0))|read(0;{})|write(0;{})|empty'
     *
     * then make sure that we find '1\n"foo"\n3\n' in the output.
     *
     * (We don't just use numbers to make sure that we don't leak jv's
     * that need jv_free()ing.)
     */
    jq_state *jq = jq_init();
    jq_handle_create_stdio(jq, 0, in, 0, 0);
    jq_handle_create_stdio(jq, 1, out, 0, 0);
    jq_handle_create_stdio(jq, 2, stderr, 0, 0);
    jq_handle_create_buffer(jq, 3);
    jq_compile_args(jq, "until(eof(0))|read(0;{})|write(1;{})|empty", 0, jv_array());
    jq_start(jq, jv_null(), 0);
    jv res = jq_next(jq);
    assert(jv_get_kind(res) == JV_KIND_INVALID);
    jv_free(res);
    jq_teardown(&jq);

    char buf[64];
    rewind(out);
    int i = 0;
    while (fgets(buf, sizeof(buf), out)) {
      switch (i) {
      case 0: assert(strcmp(buf, "0\n") == 0); break;
      case 1: assert(strcmp(buf, "\"foo\"\n") == 0); break;
      case 2: assert(strcmp(buf, "2\n") == 0); break;
      }
      i++;
    }
    assert(i == 3);
    fclose(out);

    /*
     * Test buffer I/O handles.
     *
     * Roughly like:
     *
     *     % printf '0\n"foo"\n2\n' | jq -n 'def END: read(3;{}); until(eof(0))|read(0;{})|write(3;{})|empty'
     *
     * then make sure that we find '3' in the output (except that the
     * output here is the output of jq_next() executing JQ_END, not
     * anything in stdout).
     */
    rewind(in);
    jq = jq_init();
    jq_handle_create_stdio(jq, 0, in, 0, 0);
    jq_handle_create_stdio(jq, 1, stderr, 0, 0);
    jq_handle_create_stdio(jq, 2, stderr, 0, 0);
    jq_handle_create_buffer(jq, 3);
    jq_compile_args(jq, "def BEGIN: true; def END: read(3;{}); until(eof(0))|read(0;{})|write(3;{})|empty", JQ_BEGIN_END, jv_array());
    jq_start(jq, jv_null(), 0);
    res = jq_next(jq);
    assert(jv_get_kind(res) == JV_KIND_INVALID);
    jq_start(jq, jv_null(), JQ_END);
    res = jq_next(jq);
    assert(jv_get_kind(res) == JV_KIND_NUMBER && jv_number_value(res) == 2);
    jv_free(res);
    jq_teardown(&jq);

    /*
     * Test null I/O handles, using fopen.
     *
     * Roughly like:
     *
     *     % printf '0\n"foo"\n2\n' | jq --allow-write -n \
     *          'def BEGIN: null|fopen({"mode":"w+"})|write(3;{});
     *           read(3;{})|. as $null_handle|until(eof(0))|read(0;{})|write(3;{})|empty'
     *
     * then make sure that we find nothing in the output.
     */
    rewind(in);
    jq = jq_init();
    jq_handle_create_stdio(jq, 0, in, 0, 0);
    jq_handle_create_stdio(jq, 1, stderr, 0, 0);
    jq_handle_create_stdio(jq, 2, stderr, 0, 0);
    jq_handle_create_buffer(jq, 3);
    jq_compile_args(jq,
                    "def BEGIN: fopen(null; {\"mode\":\"w+\"})|write(3;{}); "
                    "read(3;{})|. as $null_handle|"
                    "until(eof(0))|read(0;{})|write($null_handle;{})|empty",
                    JQ_BEGIN_END, jv_array());
    jq_start(jq, jv_null(), JQ_BEGIN);
    res = jq_next(jq);
    assert(jv_get_kind(res) == JV_KIND_NUMBER && jv_number_value(res) == 4);
    jq_start(jq, jv_null(), 0);
    res = jq_next(jq);
    assert(jv_get_kind(res) == JV_KIND_INVALID);
    jv_free(res);
    jq_teardown(&jq);

    /*
     * Test stdio fopen.
     */
    rewind(in);
    jq = jq_init();
    jq_handle_create_stdio(jq, 0, in, 0, 0);
    jq_handle_create_stdio(jq, 1, stderr, 0, 0);
    jq_handle_create_stdio(jq, 2, stderr, 0, 0);
    jq_handle_create_buffer(jq, 3);
    jq_compile_args(jq,
                    "def BEGIN: fopen(.; {\"mode\":\"w+\"})|write(3;{}); "
                    "read(3;{})|until(eof(0))|read(0;{})|write(3;{})|empty",
                    JQ_BEGIN_END, jv_array());
    char fname_buf[2048];
    jq_start(jq, jv_string(tmpnam(fname_buf)), JQ_BEGIN | JQ_OPEN_FILES | JQ_OPEN_WRITE);
    res = jq_next(jq);
    assert(jv_get_kind(res) == JV_KIND_NUMBER && jv_number_value(res) == 4);
    jq_start(jq, jv_null(), 0);
    res = jq_next(jq);
    assert(jv_get_kind(res) == JV_KIND_INVALID);
    jv_free(res);
    jq_teardown(&jq);

    /*
     * Test stdio popen.
     */
    rewind(in);
    jq = jq_init();
    jq_handle_create_stdio(jq, 0, in, 0, 0);
    jq_handle_create_stdio(jq, 1, stderr, 0, 0);
    jq_handle_create_stdio(jq, 2, stderr, 0, 0);
    jq_handle_create_buffer(jq, 3);
    jq_compile_args(jq,
                    "def BEGIN: popen(\"echo foo\"; {\"mode\":\"r\"})|write(3;{}); "
                    "read(3;{})|read(0;{\"raw\":true})",
                    JQ_BEGIN_END, jv_array());
    jq_start(jq, jv_null(), JQ_BEGIN | JQ_EXEC);
    res = jq_next(jq);
    assert(jv_get_kind(res) == JV_KIND_NUMBER && jv_number_value(res) == 4);
    jq_start(jq, jv_null(), 0);
    res = jq_next(jq);
    assert(jv_get_kind(res) == JV_KIND_STRING && strcmp(jv_string_value(res), "foo"));
    jv_free(res);
    jq_teardown(&jq);
  }

  /*
   * Test jq -e and BEGIN/END behavior.
   *
   * We ought to re-direct stdout here, but it doesn't really matter for
   * now.
   */
  {
    char *args[] = { "jq", "-n", "-e", "def BEGIN: 0|write(3;{}); def END: read(3;{}); empty", NULL };
    assert(main(sizeof(args)/sizeof(args[0]) - 1, args) == 0);
  }

  {
    char *args[] = { "jq", "-n", "-e", "empty", NULL };
    assert(main(sizeof(args)/sizeof(args[0]) - 1, args) == 4);
  }

  {
    char *args[] = { "jq", "-n", "-e", "def BEGIN: false; true", NULL };
    assert(main(sizeof(args)/sizeof(args[0]) - 1, args) == 1);
  }

  {
    char *args[] = { "jq", "-n", "-e", "def END: false; true", NULL };
    assert(main(sizeof(args)/sizeof(args[0]) - 1, args) == 1);
  }

  {
    char *args[] = { "jq", "-n", "-e", "def END: true; empty", NULL };
    assert(main(sizeof(args)/sizeof(args[0]) - 1, args) == 0);
  }

  {
    char *args[] = { "jq", "-n", "empty", NULL };
    assert(main(sizeof(args)/sizeof(args[0]) - 1, args) == 0);
  }
}
