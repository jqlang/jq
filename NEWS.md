# 1.7.1

## Security

- CVE-2023-50246: Fix heap buffer overflow in jvp\_literal\_number\_literal
- CVE-2023-50268: fix stack-buffer-overflow if comparing nan with payload

## CLI changes

- Make the default background color more suitable for bright backgrounds. @mjarosie @taoky @nicowilliams @itchyny #2904
- Allow passing the inline jq script after `--`. @emanuele6 #2919
- Restrict systems operations on OpenBSD and remove unused `mkstemp`. @klemensn #2934
- Fix possible uninitialised value dereference if `jq_init()` fails. @emanuele6 @nicowilliams #2935

## Language changes

- Simplify `paths/0` and `paths/1`. @asheiduk @emanuele6 #2946
- Reject `U+001F` in string literals. @torsten-schenk @itchyny @wader #2911
- Remove unused nref accumulator in `block_bind_library`. @emanuele6 #2914
- Remove a bunch of unused variables, and useless assignments. @emanuele6 #2914
- main.c: Remove unused EXIT\_STATUS\_EXACT option. @emanuele6 #2915
- Actually use the number correctly casted from double to int as index. @emanuele6 #2916
- src/builtin.c: remove unnecessary jv\_copy-s in type\_error/type\_error2. @emanuele6 #2937
- Remove undefined behavior caught by LLVM 10 UBSAN. @Gaelan @emanuele6 #2926
- Convert decnum to binary64 (double) instead of decimal64. This makes jq behave like the JSON specification suggests and more similar to other languages. @wader @leonid-s-usov #2949
- Fix memory leaks on invalid input for `ltrimstr/1` and `rtrimstr/1`. @emanuele6 #2977
- Fix memory leak on failed get for `setpath/2`. @emanuele6 #2970
- Fix nan from json parsing also for nans with payload that start with 'n'. @emanuele6 #2985
- Allow carriage return characters in comments. @emanuele6 #2942 #2984

## Documentation changes

- Generate links in the man page. @emanuele6 #2931
- Standardize arch types to AMD64 & ARM64 from index page download dropdown. @owenthereal #2884

## libjq

- Add extern C for C++. @rockwotj #2953

## Build and test changes

- Fix incorrect syntax for checksum file. @kamontat @wader #2899
- Remove `-dirty` version suffix for windows release build. @itchyny #2888
- Make use of `od` in tests more compatible. @nabijaczleweli @emanuele6 @nicowilliams #2922
- Add dependabot. @yeikel #2889
- Extend fuzzing setup to fuzz parser and and JSON serializer. @DavidKorczynski @emanuele6 #2952
- Keep releasing executables with legacy names. @itchyny #2951

# 1.7

After a five year hiatus we're back with a GitHub organization, with new admins and new maintainers who have brought a great deal of energy to make a long-awaited and long-needed new release.  We're very grateful for all the new owners, admins, and maintainers.  Special thanks go to Owen Ou (@owenthereal) for pushing to set up a new GitHub organization for jq, Stephen Dolan (@stedolan) for transferring the jq repository to the new organization, @itchyny for doing a great deal of work to get the release done, Mattias Wadman (@wader) and Emanuele Torre (@emanuele6) for many PRs and code reviews.  Many others also contributed PRs, issues, and code reviews as well, and you can find their contributions in the Git log and on the [closed issues and PRs page](https://github.com/jqlang/jq/issues?q=is%3Aclosed+sort%3Aupdated-desc).

Since the last stable release many things have happened:

- jq now lives at <https://github.com/jqlang>
- New maintainers, admins, and owners have been recruited.
  - A list of [current maintainers](https://github.com/jqlang/jq/blob/jq-1.7/AUTHORS#L4-L14)
- NEWS file is replaced by NEWS.md with Markdown format. @wader #2599
- CI, scan builds, release, website builds etc now use GitHub actions. @owenthereal @wader @itchyny #2596 #2603 #2620 #2723
- Lots of documentation improvements and fixes.
- Website updated with new section search box, better section ids for linking, dark mode, etc. @itchyny #2628
- Release builds for:
  - Linux `amd64`, `arm64`, `armel`, `armhf`, `i386`, `mips`, `mips64`, `mips64el`, `mips64r6`, `mips64r6el`, `mipsel`, `mipsr6`, `mipsr6el`, `powerpc`, `ppc64el`, `riscv64` and `s390x`
  - macOS `amd64` and `arm64`
  - Windows `i386` and `amd64`
  - Docker `linux/386`, `linux/amd64`, `linux/arm64`, `linux/mips64le`, `linux/ppc64le`, `linux/riscv64` and `linux/s390x`
  - More details see @owenthereal #2665
- Docker images are now available from `ghcr.io/jqlang/jq` instead of Docker Hub. @itchyny #2652 #2686
- OSS-fuzz. @DavidKorczynski #2760 #2762

Full commit log can be found at <https://github.com/jqlang/jq/compare/jq-1.6...jq-1.7> but here are some highlights:

## CLI changes

- Make object key color configurable using `JQ_COLORS` environment variable. @itchyny @haguenau @ericpruitt #2703

  ```sh
  # this would make "field" bold yellow (`1;33`, the last value)
  $ JQ_COLORS="0;90:0;37:0;37:0;37:0;32:1;37:1;37:1;33" ./jq -n '{field: 123}'
  {
    "field": 123
  }
  ```

- Change the default color of null to Bright Black. @itchyny #2824
- Respect `NO_COLOR` environment variable to disable color output. See <https://no-color.org> for details. @itchyny #2728
- Improved `--help` output. Now mentions all options and nicer order. @itchyny @wader #2747 #2766 #2799
- Fix multiple issues of exit code using `--exit-code`/`-e` option. @ryo1kato #1697

  ```sh
  # true-ish last output value exits with zero
  $ jq -ne true ; echo $?
  true
  0
  # false-ish last output value (false and null) exits with 1
  $ jq -ne false ; echo $?
  false
  1
  # no output value exists with 4
  $ jq -ne empty ; echo $?
  4
  ```

- Add `--binary`/`-b` on Windows for binary output. To get `\n` instead of `\r\n` line endings. @nicowilliams 0dab2b1
- Add `--raw-output0` for NUL (zero byte) separated output. @asottile @pabs3 @itchyny #1990 #2235 #2684

  ```sh
  # will output a zero byte after each output
  $ jq -n --raw-output0 '1,2,3' | xxd
  00000000: 3100 3200 3300                           1.2.3.
  # can be used with xargs -0
  $ jq -n --raw-output0 '"a","b","c"' | xargs -0 -n1
  a
  b
  c
  $ jq -n --raw-output0 '"a b c", "d\ne\nf"' | xargs -0 printf '[%s]\n'
  [a b c]
  [d
  e
  f]
  # can be used with read -d ''
  $ while IFS= read -r -d '' json; do
  >   jq '.name' <<< "$json"
  > done < <(jq -n --raw-output0 '{name:"a b c"},{name:"d\ne\nf"}')
  "a b c"
  "d\ne\nf"
  # also it's an error to output a string containing a NUL when using NUL separator
  $ jq -n --raw-output0 '"\u0000"'
  jq: error (at <unknown>): Cannot dump a string containing NUL with --raw-output0 option
  ```

- Fix assert crash and validate JSON for `--jsonarg`. @wader #2658
- Remove deprecated `--argfile` option. @itchyny #2768
- Enable stack protection. @nicowilliams #2801

## Language changes

- Use decimal number literals to preserve precision. Comparison operations respects precision but arithmetic operations might truncate. @leonid-s-usov #1752

  ```sh
  # precision is preserved
  $ echo '100000000000000000' | jq .
  100000000000000000
  # comparison respects precision (this is false in JavaScript)
  $ jq -n '100000000000000000 < 100000000000000001'
  true
  # sort/0 works
  $ jq -n -c '[100000000000000001, 100000000000000003, 100000000000000004, 100000000000000002] | sort'
  [100000000000000001,100000000000000002,100000000000000003,100000000000000004]
  # arithmetic operations might truncate (same as JavaScript)
  $ jq -n '100000000000000000 + 10'
  100000000000000020
  ```

- Adds new builtin `pick(stream)` to emit a projection of the input object or array. @pkoppstein #2656 #2779

  ```sh
  $ jq -n '{"a": 1, "b": {"c": 2, "d": 3}, "e": 4} | pick(.a, .b.c, .x)'
  {
    "a": 1,
    "b": {
      "c": 2
    },
    "x": null
  }
  ```

- Adds new builtin `debug(msgs)` that works like `debug` but applies a filter on the input before writing to stderr. @pkoppstein #2710

  ```sh
  $ jq -n '1 as $x | 2 | debug("Entering function foo with $x == \($x)", .) | (.+1)'
  ["DEBUG:","Entering function foo with $x == 1"]
  ["DEBUG:",2]
  3
  $ jq -n '{a: 1, b: 2, c: 3} | debug({a, b, sum: (.a+.b)})'
  ["DEBUG:",{"a":1,"b":2,"sum":3}]
  {
    "a": 1,
    "b": 2,
    "c": 3
  }
  ```

- Adds new builtin `scan($re; $flags)`. Was documented but not implemented. @itchyny #1961

  ```sh
  # look for pattern "ab" in "abAB" ignoring casing
  $ jq -n '"abAB" | scan("ab"; "i")'
  "ab"
  "AB"
  ```

- Adds new builtin `abs` to get absolute value. This potentially allows the literal value of numbers to be preserved as `length` and `fabs` convert to float. @pkoppstein #2767
- Allow `if` without `else`-branch. When skipped the `else`-branch will be `.` (identity). @chancez @wader #1825 #2481

  ```sh
  # convert 1 to "one" otherwise keep as is
  $ jq -n '1,2 | if . == 1 then "one" end'
  "one"
  2
  # behaves the same as
  $ jq -n '1,2 | if . == 1 then "one" else . end'
  "one"
  2
  # also works with elif
  $ jq -n '1,2,3 | if . == 1 then "one" elif . == 2 then "two" end
  "one"
  "two"
  3
  ```

- Allow use of `$binding` as key in object literals. @nicowilliams 8ea4a55

  ```sh
  $ jq -n '"a" as $key | {$key: 123}'
  {
    "a": 123
  }
  # previously parentheses were needed
  $ jq -n '"a" as $key | {($key): 123}'
  {
    "a": 123
  }
  ```

- Allow dot between chained indexes when using `.["index"]` @nicowilliams #1168

  ```sh
  $ jq -n '{"a": {"b": 123}} | .a["b"]'
  123
  # now this also works
  $ jq -n '{"a": {"b": 123}} | .a.["b"]'
  123
  ```

- Allow dot for chained value iterator `.[]`, `.[]?` @wader #2650

  ```sh
  $ jq -n '{"a": [123]} | .a[]'
  123
  # now this also works
  $ jq -n '{"a": [123]} | .a.[]'
  123
  ```

- Fix try/catch catches more than it should. @nicowilliams #2750
- Speed up and refactor some builtins, also remove `scalars_or_empty/0`. @muhmuhten #1845
- Now `halt` and `halt_error` exit immediately instead of continuing to the next input. @emanuele6 #2667
- Fix issue converting string to number after previous convert error. @thalman #2400
- Fix issue representing large numbers on some platforms causing invalid JSON output. @itchyny #2661
- Fix deletion using assigning empty against arrays. @itchyny #2133

  ```sh
  # now this works as expected, filter out all values over 2 by assigning empty
  $ jq -c '(.[] | select(. >= 2)) |= empty' <<< '[1,5,3,0,7]'
  [1,0]
  ```

- Allow keywords to be used as binding name in more places. @emanuele6 #2681
- Allow using `nan` as NaN in JSON. @emanuele6 #2712
- Expose a module's function names in `modulemeta`. @mrwilson #2837
- Fix `contains/1` to handle strings with NUL. @nicowilliams 61cd6db
- Fix `stderr/0` to output raw text without any decoration. @itchyny #2751
- Fix `nth/2` to emit empty on index out of range. @itchyny #2674
- Fix `implode` to not assert and instead replace invalid unicode codepoints. @wader #2646
- Fix `indices/1` and `rindex/1` in case of overlapping matches in strings. @emanuele6 #2718
- Fix `sub/3` to resolve issues involving global search-and-replace (gsub) operations. @pkoppstein #2641
- Fix `significand/0`, `gamma/0` and `drem/2` to be available on macOS. @itchyny #2756 #2775
- Fix empty regular expression matches. @itchyny #2677
- Fix overflow exception of the modulo operator. @itchyny #2629
- Fix string multiplication by 0 (and less than 1) to emit empty string. @itchyny #2142
- Fix segfault when using libjq and threads. @thalman #2546
- Fix constant folding of division and reminder with zero divisor. @itchyny #2797
- Fix `error/0`, `error/1` to throw null error. @emanuele6 #2823
- Simpler and faster `transpose`. @pkoppstein #2758
- Simple and efficient implementation of `walk/1`. @pkoppstein #2795
- Remove deprecated filters `leaf_paths`, `recurse_down`. @itchyny #2666

# Previous releases

Release history

- jq version 1.6 was released on Fri Nov 2 2018
- jq version 1.5 was released on Sat Aug 15 2015
- jq version 1.4 was released on Mon Jun 9 2014
- jq version 1.3 was released on Sun May 19 2013
- jq version 1.2 was released on Thu Dec 20 2012
- jq version 1.1 was released on Sun Oct 21 2012
- jq version 1.0 was released on Sun Oct 21 2012

New features in 1.6 since 1.5:

- Destructuring Alternation

- New Builtins:
  - builtins/0
  - stderr/0
  - halt/0, halt_error/1
  - isempty/1
  - walk/1
  - utf8bytelength/1
  - localtime/0, strflocaltime/1
  - SQL-style builtins
  - and more!

- Add support for ASAN and UBSAN

- Make it easier to use jq with shebangs (8f6f28c)

- Add $ENV builtin variable to access environment

- Add JQ_COLORS env var for configuring the output colors

New features in 1.5 since 1.4:

- regular expressions (with Oniguruma)

- a library/module system

- many new builtins

  - datetime builtins
  - math builtins
  - regexp-related builtins
  - stream-related builtins (e.g., all/1, any/1)
  - minimal I/O builtins (`inputs`, `debug`)

- new syntactic features, including:

  - destructuring (`. as [$first, $second] | ...`)
  - try/catch, generalized `?` operator, and label/break
  - `foreach`
  - multiple definitions of a function with different numbers of
      arguments

- command-line arguments

  - --join-lines / -j for raw output
  - --argjson and --slurpfile
  - --tab and --indent
  - --stream (streaming JSON parser)
  - --seq (RFC7464 JSON text sequence)
  - --run-tests improvements

- optimizations:

  - tail-call optimization
  - reduce and foreach no longer leak a reference to .

New features in 1.4 since 1.3:

- command-line arguments

  - jq --arg-file variable file
  - jq --unbuffered
  - jq -e / --exit-status (set exit status based on outputs)
  - jq -S / --sort-keys (now jq no longer sorts object keys by
      default

- syntax

  - .. -> like // in XPath (recursive traversal)
  - question mark (e.g., .a?) to suppress errors
  - ."foo" syntax (equivalent to .["foo"])
  - better error handling for .foo
  - added % operator (modulo)
  - allow negation without requiring extra parenthesis
  - more function arguments (up to six)

- filters:

  - any, all
  - iterables, arrays, objects, scalars, nulls, booleans, numbers,
      strings, values

- string built-ins:

  - split
  - join (join an array of strings with a given separator string)
  - ltrimstr, rtrimstr
  - startswith, endswith
  - explode, implode
  - fromjson, tojson
  - index, rindex, indices

- math functions

  - floor, sqrt, cbrt, etcetera (depends on what's available from libm)

- libjq -- a C API interface to jq's JSON representation and for
   running jq programs from C applications
