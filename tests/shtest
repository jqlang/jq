#!/bin/sh -x

. "${0%/*}/setup" "$@"

msys=false
mingw=false
case "$(uname -s)" in
MSYS*)  msys=true;;
MINGW*) mingw=true;;
esac

JQ_NO_B=$JQ
JQ="$JQ -b"

PATH=$JQBASEDIR:$PATH $JQBASEDIR/tests/jq-f-test.sh > /dev/null

if [ -f "$JQBASEDIR/.libs/libinject_errors.so" ]; then
  # Do some simple error injection tests to check that we're handling
  # I/O errors correctly.
  (
  libinject=$JQBASEDIR/.libs/libinject_errors.so
  cd $d
  LD_PRELOAD=$libinject $JQ . /dev/null
  touch fail_read
  LD_PRELOAD=$libinject $JQ . fail_read && exit 2
  touch fail_close
  LD_PRELOAD=$libinject $JQ . fail_close && exit 2
  true
  )
fi

printf 'a\0b\nc\0d\ne' > $d/input
$VALGRIND $Q $JQ -Rse '. == "a\u0000b\nc\u0000d\ne"' $d/input
$VALGRIND $Q $JQ -Rne '[inputs] == ["a\u0000b", "c\u0000d", "e"]' $d/input

## Test constant folding

## XXX If we add a builtin to list the program's disassembly then we can
## move all of these into tests/all.test

# String constant folding (addition only)
nref=$($VALGRIND $Q $JQ -n --debug-dump-disasm '"foo"' | wc -l)
n=$($VALGRIND $Q $JQ -n --debug-dump-disasm '"foo" + "bar"' | wc -l)
if [ $n -ne $nref ]; then
  echo "Constant expression folding for strings didn't work"
  exit 1
fi

# Numeric constant folding (binary operators)
n=$($VALGRIND $Q $JQ -n --debug-dump-disasm '1+1' | wc -l)
if [ $n -ne $nref ]; then
  echo "Constant expression folding for numbers didn't work"
  exit 1
fi
n=$($VALGRIND $Q $JQ -n --debug-dump-disasm '1-1' | wc -l)
if [ $n -ne $nref ]; then
  echo "Constant expression folding for numbers didn't work"
  exit 1
fi
n=$($VALGRIND $Q $JQ -n --debug-dump-disasm '2*3' | wc -l)
if [ $n -ne $nref ]; then
  echo "Constant expression folding for numbers didn't work"
  exit 1
fi
n=$($VALGRIND $Q $JQ -n --debug-dump-disasm '9/3' | wc -l)
if [ $n -ne $nref ]; then
  echo "Constant expression folding for numbers didn't work"
  exit 1
fi
n=$($VALGRIND $Q $JQ -n --debug-dump-disasm '9%3' | wc -l)
if [ $n -ne $nref ]; then
  echo "Constant expression folding for numbers didn't work"
  exit 1
fi
n=$($VALGRIND $Q $JQ -n --debug-dump-disasm '9==3' | wc -l)
if [ $n -ne $nref ]; then
  echo "Constant expression folding for numbers didn't work"
  exit 1
fi
n=$($VALGRIND $Q $JQ -n --debug-dump-disasm '9!=3' | wc -l)
if [ $n -ne $nref ]; then
  echo "Constant expression folding for numbers didn't work"
  exit 1
fi
n=$($VALGRIND $Q $JQ -n --debug-dump-disasm '9<3' | wc -l)
if [ $n -ne $nref ]; then
  echo "Constant expression folding for numbers didn't work"
  exit 1
fi
n=$($VALGRIND $Q $JQ -n --debug-dump-disasm '9>3' | wc -l)
if [ $n -ne $nref ]; then
  echo "Constant expression folding for numbers didn't work"
  exit 1
fi
n=$($VALGRIND $Q $JQ -n --debug-dump-disasm '9<=3' | wc -l)
if [ $n -ne $nref ]; then
  echo "Constant expression folding for numbers didn't work"
  exit 1
fi
n=$($VALGRIND $Q $JQ -n --debug-dump-disasm '9>=3' | wc -l)
if [ $n -ne $nref ]; then
  echo "Constant expression folding for numbers didn't work"
  exit 1
fi

## Test JSON sequence support

cat > $d/expected <<EOF
jq: ignoring parse error: Truncated value at line 2, column 5
jq: ignoring parse error: Truncated value at line 2, column 25
jq: ignoring parse error: Truncated value at line 2, column 41
EOF
printf '1\0362 3\n[0,1\036[4,5]true"ab"{"c":4\036{}{"d":5,"e":6"\036false\n'|$VALGRIND $Q $JQ -ces --seq '. == [2,3,[4,5],true,"ab",{},false]' > /dev/null 2> $d/out
cmp $d/out $d/expected

cat > $d/expected <<EOF
jq: ignoring parse error: Truncated value at line 2, column 5
jq: ignoring parse error: Truncated value at line 2, column 25
jq: ignoring parse error: Truncated value at line 3, column 1
EOF
printf '1\0362 3\n[0,1\036[4,5]true"ab"{"c":4\036{}{"d":5,"e":6"false\n\036null'|$VALGRIND $Q $JQ -ces --seq '. == [2,3,[4,5],true,"ab",{},null]' > /dev/null 2> $d/out
cmp $d/out $d/expected

# Note that here jq sees no inputs at all but it still succeeds because
# --seq ignores parse errors
cat > $d/expected <<EOF
jq: ignoring parse error: Unfinished abandoned text at EOF at line 1, column 4
EOF
printf '"foo' | $JQ -c --seq . > $d/out 2>&1
cmp $d/out $d/expected

# with -e option should give 4 here as there's no valid output after
# ignoring parse errors with --seq.
printf '"foo' | $JQ -ce --seq . > $d/out 2>&1 || ret=$?
[ $ret -eq 4 ]
cmp $d/out $d/expected

# Numeric values truncated by EOF are ignored
cat > $d/expected <<EOF
jq: ignoring parse error: Unfinished abandoned text at EOF at line 1, column 1
EOF
printf '1' | $JQ -c --seq . > $d/out 2>&1
cmp $d/out $d/expected

cat > $d/expected <<EOF
jq: error (at <stdin>:1): Unfinished abandoned text at EOF at line 2, column 0
EOF
if printf '1\n' | $JQ -cen --seq '[inputs] == []' >/dev/null 2> $d/out; then
  printf 'Error expected but jq exited successfully\n' 1>&2
  exit 2
fi
cmp $d/out $d/expected


## Test --exit-status
data='{"i": 1}\n{"i": 2}\n{"i": 3}\n'
printf "$data" | $JQ --exit-status 'select(.i==1)' > /dev/null 2>&1
printf "$data" | $JQ --exit-status 'select(.i==2)' > /dev/null 2>&1
printf "$data" | $JQ --exit-status 'select(.i==3)' > /dev/null 2>&1
ret=0
printf "$data" | $JQ --exit-status 'select(.i==4)' > /dev/null 2>&1 || ret=$?
[ $ret -eq 4 ]
ret=0
printf "$data" | $JQ --exit-status 'select(.i==2) | false' > /dev/null 2>&1 || ret=$?
[ $ret -eq 1 ]
printf "$data" | $JQ --exit-status 'select(.i==2) | true' > /dev/null 2>&1


# Regression test for #951
printf '"a\n' > $d/input
if $VALGRIND $Q $JQ -e . $d/input; then
  printf 'Issue #951 is back?\n' 1>&2
  exit 2
fi

# Regression test for #2146
if echo "foobar" | $JQ .; then
  printf 'Issue #2146 is back?\n' 1>&2
  exit 1
elif [ $? -ne 5 ]; then
  echo "Invalid input had wrong error code" 1>&2
  exit 1
fi

# Regression test for #2367; make sure to call jq twice
if ! echo '{"a": 1E9999999999}' | $JQ . | $JQ -e .a; then
  printf 'Issue #2367 is back?\n' 1>&2
  exit 1
fi

# Regression test for #1534
echo "[1,2,3,4]" > $d/expected
printf "[1,2][3,4]" | $JQ -cs add > $d/out 2>&1
cmp $d/out $d/expected
printf "[1,2][3,4]\n" | $JQ -cs add > $d/out 2>&1
cmp $d/out $d/expected

# Regression test for --raw-output0
printf "a\0b\0" > $d/expected
printf '["a", "b"]' | $VALGRIND $Q $JQ --raw-output0 .[] > $d/out
cmp $d/out $d/expected
printf "a\0" > $d/expected
if printf '["a", "c\\u0000d", "b"]' | $VALGRIND $Q $JQ --raw-output0 .[] > $d/out; then
  echo "Should exit error on string containing NUL with --raw-output0" 1>&2
  exit 1
elif [ $? -ne 5 ]; then
  echo "Invalid error code" 1>&2
  exit 1
else
  cmp $d/out $d/expected
fi

## Test streaming parser

## If we add an option to stream to the `import ... as $symbol;` directive
## then we can move these tests into tests/all.test.
$VALGRIND $Q $JQ -c '. as $d|path(..) as $p|$d|getpath($p)|select((type|. != "array" and . != "object") or length==0)|[$p,.]' < "$JQTESTDIR/torture/input0.json" > $d/out0
$VALGRIND $Q $JQ --stream -c '.|select(length==2)' < "$JQTESTDIR/torture/input0.json" > $d/out1
diff $d/out0 $d/out1

printf '["Unfinished JSON term at EOF at line 1, column 1",[0]]\n' > $d/expected
printf '[' | $VALGRIND $Q $JQ --stream-errors -c . > $d/out 2>&1
diff $d/out $d/expected


## XXX This test can be moved to tests/all.test _now_
clean=false
if which seq > /dev/null 2>&1; then
    # XXX We should try every prefix of input0.json, but that makes this
    # test very, very slow when run with valgrind, and the whole point
    # is to run it with valgrind.
    #
    #len=$(wc -c < "$JQTESTDIR/torture/input0.json")
    if [ -z "$VALGRIND" ]; then
        start=1
        end=$(wc -c < "$JQTESTDIR/torture/input0.json")
    else
        start=120
        end=151
    fi
    for i in $(seq $start $end); do
        dd "if=tests/torture/input0.json" bs=$i count=1 2>/dev/null |
            $VALGRIND $JQ -c . > $d/out0 2>$d/err || true
        if [ -n "$VALGRIND" ]; then
            grep '^==[0-9][0-9]*== ERROR SUMMARY: 0 errors' $d/err > /dev/null
        else
            tail -n1 -- "$d/err" | grep -Ei 'assert|abort|core' && false
        fi

        dd "if=tests/torture/input0.json" bs=$i count=1 2>/dev/null |
            $VALGRIND $JQ -cn --stream 'fromstream(inputs)' > $d/out1 2>$d/err || true
        if [ -n "$VALGRIND" ]; then
            grep '^==[0-9][0-9]*== ERROR SUMMARY: 0 errors' $d/err > /dev/null
        else
            tail -n1 -- "$d/err" | grep -Ei 'assert|abort|core' && false
        fi

        diff $d/out0 $d/out1
    done
else
    echo "Not doing torture tests"
fi

## Regression test for issue #2378 assert when stream parse broken object pair
echo '{"a":1,"b",' | $JQ --stream  > /dev/null 2> $d/err || true
grep 'Objects must consist of key:value pairs' $d/err > /dev/null

## Regression tests for issue #2463 assert when stream parse non-scalar object key
echo '{{"a":"b"}}' | $JQ --stream > /dev/null 2> $d/err || true
grep "Expected string key after '{', not '{'" $d/err > /dev/null
echo '{"x":"y",{"a":"b"}}' | $JQ --stream > /dev/null 2> $d/err || true
grep "Expected string key after ',' in object, not '{'" $d/err > /dev/null
echo '{["a","b"]}' | $JQ --stream > /dev/null 2> $d/err || true
grep "Expected string key after '{', not '\\['" $d/err > /dev/null
echo '{"x":"y",["a","b"]}' | $JQ --stream > /dev/null 2> $d/err || true
grep "Expected string key after ',' in object, not '\\['" $d/err > /dev/null

# debug, stderr
$VALGRIND $Q $JQ -n '"test", {} | debug, stderr' >/dev/null
$JQ -n -c -j '"hello\nworld", null, [false, 0], {"foo":["bar"]}, "\n" | stderr' >$d/out 2>$d/err
cat > $d/expected <<'EOF'
hello
worldnull[false,0]{"foo":["bar"]}
EOF
cmp $d/out $d/expected
cmp $d/err $d/expected

# --arg, --argjson, $ARGS.named
$VALGRIND $JQ -n -c --arg foo 1 --argjson bar 2 '{$foo, $bar} | ., . == $ARGS.named' > $d/out
printf '{"foo":"1","bar":2}\ntrue\n' > $d/expected
cmp $d/out $d/expected

# --slurpfile, --rawfile
$VALGRIND $JQ -n --slurpfile foo $JQBASEDIR/tests/modules/data.json \
  --rawfile bar $JQBASEDIR/tests/modules/data.json '{$foo, $bar}' > $d/out
cat > $d/expected <<'EOF'
{
  "foo": [
    {
      "this": "is a test",
      "that": "is too"
    }
  ],
  "bar": "{\n  \"this\": \"is a test\",\n  \"that\": \"is too\"\n}\n"
}
EOF
cmp $d/out $d/expected

# --args, --jsonargs, $ARGS.positional
$VALGRIND $JQ -n -c --args '$ARGS.positional' foo bar baz > $d/out
printf '["foo","bar","baz"]\n' > $d/expected
cmp $d/out $d/expected
$VALGRIND $JQ -n -c --jsonargs '$ARGS.positional' null true '[]' '{}' > $d/out
printf '[null,true,[],{}]\n' > $d/expected
cmp $d/out $d/expected
$VALGRIND $JQ -n -c '$ARGS.positional' --args foo 1 --jsonargs 2 '{}' --args 3 4 > $d/out
printf '["foo","1",2,{},"3","4"]\n' > $d/expected
cmp $d/out $d/expected
$VALGRIND $JQ -n -c '$ARGS.positional' --args --jsonargs > $d/out
printf '[]\n' > $d/expected
cmp $d/out $d/expected

## Regression test for issue #2572 assert when using --jsonargs and invalid JSON
$VALGRIND $JQ -n --jsonargs null invalid && EC=$? || EC=$?
if [ "$EC" -ne 2 ]; then
    echo "--jsonargs exited with wrong exit code, expected 2 got $EC" 1>&2
    exit 1
fi
# this tests the args_done code path "--"
$VALGRIND $JQ -n --jsonargs null -- invalid && EC=$? || EC=$?
if [ "$EC" -ne 2 ]; then
    echo "--jsonargs exited with wrong exit code, expected 2 got $EC" 1>&2
    exit 1
fi

## Fuzz parser

## XXX With a $(urandom) builtin we could move this test into tests/all.test
clean=false
if dd if=/dev/urandom bs=16 count=1024 > $d/rand 2>/dev/null; then
    # Have a /dev/urandom, good
    $VALGRIND $Q $JQ --seq . $d/rand >/dev/null 2>&1
    $VALGRIND $Q $JQ --seq --stream . $d/rand >/dev/null 2>&1
    dd if=/dev/urandom bs=16 count=1024 > $d/rand 2>/dev/null
    $VALGRIND $Q $JQ --seq . $d/rand >/dev/null 2>&1
    $VALGRIND $Q $JQ --seq --stream . $d/rand >/dev/null 2>&1
    dd if=/dev/urandom bs=16 count=1024 > $d/rand 2>/dev/null
    $VALGRIND $Q $JQ --seq . $d/rand >/dev/null 2>&1
    $VALGRIND $Q $JQ --seq --stream . $d/rand >/dev/null 2>&1
fi
clean=true

## Test library/module system

# Check handling of ~/.jq; these can't move into jq_test.c yet because
# they depend on $HOME
if [ "$(HOME="$mods/home1" $VALGRIND $Q $JQ -nr fg)" != foobar ]; then
    echo "Bug #479 appears to be back" 1>&2
    exit 1
fi

if $msys || $mingw; then
    HOME_BAK=$HOME
    unset HOME
    if [ "$(USERPROFILE="$mods/home1" $VALGRIND $Q $JQ -nr foo)" != baz ]; then
        echo "Bug #3104 regressed (sourcing %USERPROFILE%/.jq on Windows)" 1>&2
        exit 1
    fi
    export HOME=$HOME_BAK
    unset HOME_BAK
fi

if [ $(HOME="$mods/home1" $VALGRIND $Q $JQ --debug-dump-disasm -n fg | grep '^[a-z]' | wc -l) -ne 3 ]; then
    echo "Binding too many defs into program" 1>&2
    exit 1
fi

if ! HOME="$mods/home2" $VALGRIND $Q $JQ -n 'include "g"; empty'; then
    echo "Mishandling directory ~/.jq" 1>&2
    exit 1
fi

cd "$JQBASEDIR" # so that relative library paths are guaranteed correct
if ! $VALGRIND $Q $JQ -L ./tests/modules -ne 'import "test_bind_order" as check; check::check==true'; then
    echo "Issue #817 regression?" 1>&2
    exit 1
fi

cd "$JQBASEDIR"
if ! $VALGRIND $Q $JQ -L tests/modules -ne 'import "test_bind_order" as check; check::check==true'; then
    echo "Issue #817 regression?" 1>&2
    exit 1
fi

## Halt

if ! $VALGRIND $Q $JQ -n halt; then
    echo "jq halt didn't work as expected" 1>&2
    exit 1
fi
if $VALGRIND $Q $VG_EXIT0 $JQ -n 'halt_error(1)'; then
    echo "jq halt_error(1) didn't work as expected" 1>&2
    exit 1
elif [ $? -ne 1 ]; then
    echo "jq halt_error(1) had wrong error code" 1>&2
    exit 1
fi
if $VALGRIND $Q $VG_EXIT0 $JQ -n 'halt_error(11)'; then
    echo "jq halt_error(11) didn't work as expected" 1>&2
    exit 1
elif [ $? -ne 11 ]; then
    echo "jq halt_error(11) had wrong error code" 1>&2
    exit 1
fi
if [ -n "$($VALGRIND $Q $JQ -n 'halt_error(1)' 2>&1)" ]; then
    echo "jq halt_error(1) had unexpected output" 1>&2
    exit 1
fi
if [ -n "$($VALGRIND $Q $JQ -n '"xyz\n" | halt_error(1)' 2>/dev/null)" ]; then
    echo "jq halt_error(1) had unexpected output on stdout" 1>&2
    exit 1
fi
if [ "$($VALGRIND $Q $JQ -n '"xy" | halt_error(1)' 2>&1 || echo "z")" != "xyz" ]; then
    echo "jq halt_error(1) had unexpected output" 1>&2
    exit 1
fi
if [ "$($VALGRIND $Q $JQ -n '"x\u0000y\u0000z" | halt_error(1)' 2>&1 | tr '\0' '.')" != "x.y.z" ]; then
    echo "jq halt_error(1) had unexpected output" 1>&2
    exit 1
fi
if [ "$($VALGRIND $Q $JQ -n '{"a":"xyz"} | halt_error(1)' 2>&1)" != '{"a":"xyz"}' ]; then
    echo "jq halt_error(1) had unexpected output" 1>&2
    exit 1
fi

# Check $JQ_COLORS
unset JQ_COLORS

## Default colors, null input
$JQ -Ccn . > $d/color
printf '\033[0;90mnull\033[0m\n' > $d/expect
cmp $d/color $d/expect

## Set non-default color, null input
JQ_COLORS='4;31' $JQ -Ccn . > $d/color
printf '\033[4;31mnull\033[0m\n' > $d/expect
cmp $d/color $d/expect

## Default colors, complex input
$JQ -Ccn '[{"a":true,"b":false},"abc",123,null]' > $d/color
{
  printf '\033[1;39m[\033[0m'
  printf '\033[1;39m{\033[0m'
  printf '\033[1;34m"a"\033[0m'
  printf '\033[1;39m:\033[0m'
  printf '\033[0;39mtrue\033[0m'
  printf '\033[1;39m,\033[0m'
  printf '\033[1;34m"b"\033[0m'
  printf '\033[1;39m:\033[0m'
  printf '\033[0;39mfalse\033[0m'
  printf '\033[1;39m}\033[0m'
  printf '\033[1;39m,\033[0m'
  printf '\033[0;32m"abc"\033[0m'
  printf '\033[1;39m,\033[0m'
  printf '\033[0;39m123\033[0m'
  printf '\033[1;39m,\033[0m'
  printf '\033[0;90mnull\033[0m'
  printf '\033[1;39m]\033[0m\n'
} > $d/expect
cmp $d/color $d/expect

## Set non-default colors, complex input
JQ_COLORS='0;30:0;31:0;32:0;33:0;34:1;35:1;36:1;37' \
  $JQ -Ccn '[{"a":true,"b":false},"abc",123,null]' > $d/color
{
  printf '\033[1;35m[\033[0m'
  printf '\033[1;36m{\033[0m'
  printf '\033[1;37m"a"\033[0m'
  printf '\033[1;36m:\033[0m'
  printf '\033[0;32mtrue\033[0m'
  printf '\033[1;36m,\033[0m'
  printf '\033[1;37m"b"\033[0m'
  printf '\033[1;36m:\033[0m'
  printf '\033[0;31mfalse\033[0m'
  printf '\033[1;36m}\033[0m'
  printf '\033[1;35m,\033[0m'
  printf '\033[0;34m"abc"\033[0m'
  printf '\033[1;35m,\033[0m'
  printf '\033[0;33m123\033[0m'
  printf '\033[1;35m,\033[0m'
  printf '\033[0;30mnull\033[0m'
  printf '\033[1;35m]\033[0m\n'
} > $d/expect
cmp $d/color $d/expect

## Default colors, complex input, indented
$JQ -Cn '[{"a":true,"b":false},"abc",123,null]' > $d/color
{
  printf '\033[1;39m[\033[0m\n'
  printf '  \033[1;39m{\033[0m\n'
  printf '    \033[1;34m"a"\033[0m'
  printf '\033[1;39m:\033[0m '
  printf '\033[0;39mtrue\033[0m'
  printf '\033[1;39m,\033[0m\n'
  printf '    \033[1;34m"b"\033[0m'
  printf '\033[1;39m:\033[0m '
  printf '\033[0;39mfalse\033[0m\n'
  printf '  \033[1;39m}\033[0m'
  printf '\033[1;39m,\033[0m\n'
  printf '  \033[0;32m"abc"\033[0m'
  printf '\033[1;39m,\033[0m\n'
  printf '  \033[0;39m123\033[0m'
  printf '\033[1;39m,\033[0m\n'
  printf '  \033[0;90mnull\033[0m\n'
  printf '\033[1;39m]\033[0m\n'
} > $d/expect
cmp $d/color $d/expect

## Set non-default colors, complex input, indented
JQ_COLORS='0;30:0;31:0;32:0;33:0;34:1;35:1;36:1;37' \
  $JQ -Cn '[{"a":true,"b":false},"abc",123,null]' > $d/color
{
  printf '\033[1;35m[\033[0m\n'
  printf '  \033[1;36m{\033[0m\n'
  printf '    \033[1;37m"a"\033[0m'
  printf '\033[1;36m:\033[0m '
  printf '\033[0;32mtrue\033[0m'
  printf '\033[1;36m,\033[0m\n'
  printf '    \033[1;37m"b"\033[0m'
  printf '\033[1;36m:\033[0m '
  printf '\033[0;31mfalse\033[0m\n'
  printf '  \033[1;36m}\033[0m'
  printf '\033[1;35m,\033[0m\n'
  printf '  \033[0;34m"abc"\033[0m'
  printf '\033[1;35m,\033[0m\n'
  printf '  \033[0;33m123\033[0m'
  printf '\033[1;35m,\033[0m\n'
  printf '  \033[0;30mnull\033[0m\n'
  printf '\033[1;35m]\033[0m\n'
} > $d/expect
cmp $d/color $d/expect

# Check garbage in JQ_COLORS.  We write each color sequence into a 16
# char buffer that needs to hold ESC [ <color> m NUL, so each color
# sequence can be no more than 12 chars (bytes).  These emit a warning
# on stderr.
set -vx
echo 'Failed to set $JQ_COLORS' > $d/expect_warning
$JQ -Ccn '[{"a":true,"b":false},"abc",123,null]' > $d/expect
JQ_COLORS='garbage;30:*;31:,;3^:0;$%:0;34:1;35:1;36' \
  $JQ -Ccn '[{"a":true,"b":false},"abc",123,null]' > $d/color 2>$d/warning
cmp $d/color $d/expect
cmp $d/warning $d/expect_warning
JQ_COLORS='1234567890123456789;30:0;31:0;32:0;33:0;34:1;35:1;36' \
  $JQ -Ccn '[{"a":true,"b":false},"abc",123,null]' > $d/color 2>$d/warning
cmp $d/color $d/expect
cmp $d/warning $d/expect_warning
JQ_COLORS='1;31234567890123456789:0;31:0;32:0;33:0;34:1;35:1;36' \
  $JQ -Ccn '[{"a":true,"b":false},"abc",123,null]' > $d/color 2>$d/warning
cmp $d/color $d/expect
cmp $d/warning $d/expect_warning
JQ_COLORS='1234567890123456;1234567890123456:1234567890123456;1234567890123456:1234567890123456;1234567890123456:1234567890123456;1234567890123456:1234567890123456;1234567890123456:1234567890123456;1234567890123456:1234567890123456;1234567890123456' \
  $JQ -Ccn '[{"a":true,"b":false},"abc",123,null]' > $d/color 2>$d/warning
cmp $d/color $d/expect
cmp $d/warning $d/expect_warning
JQ_COLORS="0123456789123:0123456789123:0123456789123:0123456789123:0123456789123:0123456789123:0123456789123:0123456789123:" \
  $JQ -Ccn '[{"a":true,"b":false},"abc",123,null]' > $d/color 2>$d/warning
cmp $d/color $d/expect
cmp $d/warning $d/expect_warning

# Check $NO_COLOR
test_no_color=true
$msys  && test_no_color=false
$mingw && test_no_color=false
if $test_no_color && command -v script >/dev/null 2>&1; then
  unset NO_COLOR
  if script -qc echo /dev/null >/dev/null 2>&1; then
    faketty() { script -qec "$*" /dev/null; }
  else # macOS
    faketty() { script -q /dev/null "$@" /dev/null |
      sed 's/^\x5E\x44\x08\x08//'; }
  fi

  faketty $JQ_NO_B -n . > $d/color
  printf '\033[0;90mnull\033[0m\r\n' > $d/expect
  od -tc $d/expect
  od -tc $d/color
  cmp $d/color $d/expect
  NO_COLOR= faketty $JQ_NO_B -n . > $d/color
  printf '\033[0;90mnull\033[0m\r\n' > $d/expect
  od -tc $d/expect
  od -tc $d/color
  cmp $d/color $d/expect
  NO_COLOR=1 faketty $JQ_NO_B -n . > $d/color
  printf 'null\r\n' > $d/expect
  od -tc $d/expect
  od -tc $d/color
  cmp $d/color $d/expect
  NO_COLOR=1 faketty $JQ_NO_B -Cn . > $d/color
  printf '\033[0;90mnull\033[0m\r\n' > $d/expect
  od -tc $d/expect
  od -tc $d/color
  cmp $d/color $d/expect
fi

# #2785
if $VALGRIND $Q $JQ -n -f "$JQTESTDIR/no-main-program.jq" > $d/out 2>&1; then
  echo "jq -f $JQTESTDIR/no-main-program.jq succeeded"
  exit 1
else
  EC=$?
  if [ $EC -eq 1 ]; then
    echo "jq -f $JQTESTDIR/no-main-program.jq failed with memory errors"
    exit 1
  fi
  if [ $EC -ne 3 ]; then
    echo "jq -f $JQTESTDIR/no-main-program.jq failed with wrong exit code ($EC)"
    exit 1
  fi
fi
cat > $d/expected <<EOF
jq: error: Top-level program not given (try ".")
jq: 1 compile error
EOF
if ! diff $d/expected $d/out; then
  echo "jq -f $JQTESTDIR/no-main-program.jq failed but its error message is not the expected one"
  exit 1
fi

if ! $VALGRIND $Q $JQ -n -f "$JQTESTDIR/yes-main-program.jq" > $d/out 2>&1; then
  echo "jq -f $JQTESTDIR/yes-main-program.jq failed"
  exit 1
fi

if ! $msys && ! $mingw && locales=$(locale -a); then
  { l=$(grep -Ev '^(C|LANG|POSIX|en)' | grep -Ei '\.utf-?8$' | head -n1) ;} \
<<EOF
$locales
EOF
  if [ -z "$l" ]; then
    echo "WARNING: Not testing localization"
  else
    date=$(LC_ALL=$l date +'%a %d %b %Y at %H:%M:%S')
    if ! LC_ALL=$l $JQ -nR --arg date "$date" '$date|strptime("%a %d %b %Y at %H:%M:%S")'; then
      echo "jq does not honor LC_ALL environment variable"
      exit 1
    fi
  fi
fi

# Comments!
if ! x=$($JQ -n '123 # comment') || [ "$x" != 123 ]; then
  echo 'comment did not work'
  exit 1
fi

cr=$(printf \\r)
if ! x=$($JQ -n "1 # foo$cr + 2") || [ "$x" != 1 ]; then
  echo 'regression: carriage return terminates comment'
  exit 1
fi

if ! x=$($JQ -cn '[
  1,
  # foo \
  2,
  # bar \\
  3,
  4, # baz \\\
  5, \
  6,
  7
  # comment \
    comment \
    comment
]') || [ "$x" != '[1,3,4,7]' ]; then
  echo 'multiline comment was not handled correctly'
  exit 1
fi

# CVE-2023-50268: No stack overflow comparing a nan with a large payload
if $JQ -ne 'have_decnum'; then
  $VALGRIND $Q $JQ '1 != .' <<\EOF >/dev/null
  Nan4000
EOF
fi

# Allow passing the inline jq script before -- #2919
if ! r=$($JQ --args -rn -- '$ARGS.positional[0]' bar) || [ "$r" != bar ]; then
    echo "passing the inline script after -- didn't work"
    exit 1
fi
if ! r=$($JQ --args -rn 1 -- '$ARGS.positional[0]' bar) || [ "$r" != 1 ]; then
    echo "passing the inline script before -- didn't work"
    exit 1
fi

# CVE-2023-50246: No heap overflow for '-10E-1000000001'
$VALGRIND $Q $JQ . <<\NUM
-10E-1000000001
NUM

exit 0
