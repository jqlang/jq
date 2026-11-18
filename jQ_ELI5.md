# jq, explained like you're five

jq reads JSON, lets you pick pieces out of it, change it, and print it back.
Think of JSON as a toy box and jq as the hand you reach in with.

Every example below is a command you can paste into a terminal. Lines that
start with `$` are commands; lines starting with `#` are comments.

The first thing to remember: `.` means "the whole thing I was given".

---

## 1. Pretty-print and minify

```console
# Make messy JSON readable
$ echo '{"name":"Ada","age":36}' | jq .

# Squash it onto one line (compact)
$ echo '{"name": "Ada", "age": 36}' | jq -c .

# Sort the keys alphabetically
$ echo '{"b":1,"a":2}' | jq -S .

# Use tabs instead of spaces when pretty-printing
$ echo '{"a":[1,2]}' | jq --tab .

# Indent with 4 spaces
$ echo '{"a":[1,2]}' | jq --indent 4 .
```

## 2. Pick a value out

```console
# Get a field
$ echo '{"name":"Ada","age":36}' | jq '.name'

# Same, but without the quotes (-r = raw)
$ echo '{"name":"Ada","age":36}' | jq -r '.name'

# Go deeper
$ echo '{"user":{"address":{"city":"Paris"}}}' | jq '.user.address.city'

# Field with a weird name (spaces, dashes)
$ echo '{"first name":"Ada"}' | jq '."first name"'

# Field that might not exist: you get null, not an error
$ echo '{"a":1}' | jq '.missing'

# Give a default when something is missing or null
$ echo '{"a":1}' | jq '.missing // "none"'
```

## 3. Arrays

```console
# First, last, and by position
$ echo '[10,20,30]' | jq '.[0]'
$ echo '[10,20,30]' | jq '.[-1]'

# A slice (positions 1 up to, but not including, 3)
$ echo '[10,20,30,40]' | jq -c '.[1:3]'

# Every item, one per line
$ echo '[10,20,30]' | jq '.[]'

# How many items?
$ echo '[10,20,30]' | jq 'length'

# Put the results back into an array
$ echo '[1,2,3]' | jq -c '[.[] | . * 2]'

# Same thing, shorter: map
$ echo '[1,2,3]' | jq -c 'map(. * 2)'
```

## 4. Lists of objects (the most common case)

```console
# Sample data used below:
#   [{"name":"Ada","age":36},{"name":"Bob","age":17},{"name":"Cy","age":52}]

# Just the names
$ echo '[{"name":"Ada","age":36},{"name":"Bob","age":17},{"name":"Cy","age":52}]' | jq -c 'map(.name)'

# Names, one per line, no quotes
$ echo '[{"name":"Ada","age":36},{"name":"Bob","age":17},{"name":"Cy","age":52}]' | jq -r '.[].name'

# Keep only some items (filter)
$ echo '[{"name":"Ada","age":36},{"name":"Bob","age":17},{"name":"Cy","age":52}]' | jq -c 'map(select(.age >= 18))'

# Sort by a field
$ echo '[{"name":"Ada","age":36},{"name":"Bob","age":17},{"name":"Cy","age":52}]' | jq -c 'sort_by(.age)'

# Oldest person
$ echo '[{"name":"Ada","age":36},{"name":"Bob","age":17},{"name":"Cy","age":52}]' | jq -c 'max_by(.age)'

# Total and average age
$ echo '[{"name":"Ada","age":36},{"name":"Bob","age":17},{"name":"Cy","age":52}]' | jq 'map(.age) | add'
$ echo '[{"name":"Ada","age":36},{"name":"Bob","age":17},{"name":"Cy","age":52}]' | jq 'map(.age) | add / length'

# Build brand-new objects with only the fields you want
$ echo '[{"name":"Ada","age":36},{"name":"Bob","age":17}]' | jq -c '.[] | {who: .name, years: .age}'

# Group by something and count each group
$ echo '[{"t":"a"},{"t":"b"},{"t":"a"}]' | jq -c 'group_by(.t) | map({type: .[0].t, count: length})'

# Remove duplicates
$ echo '[3,1,3,2,1]' | jq -c 'unique'
```

## 5. Searching and testing

```console
# Does a string contain something?
$ echo '"hello world"' | jq 'contains("world")'

# Starts / ends with
$ echo '"hello world"' | jq 'startswith("hello")'
$ echo '"hello world"' | jq 'endswith("world")'

# Regular expressions
$ echo '"order-1234"' | jq 'test("[0-9]+")'
$ echo '"order-1234"' | jq -r 'capture("(?<num>[0-9]+)") | .num'
$ echo '"a1b22c333"' | jq -c '[match("[0-9]+"; "g").string]'

# Is a key present?
$ echo '{"a":1}' | jq 'has("a")'

# Is a value in an array?
$ echo '["a","b"]' | jq 'index("b")'
$ echo '[1,2,3]' | jq 'any(. > 2)'
$ echo '[1,2,3]' | jq 'all(. > 0)'
```

## 6. Strings

```console
# Upper / lower case
$ echo '"Hello"' | jq 'ascii_upcase'
$ echo '"Hello"' | jq 'ascii_downcase'

# Replace text
$ echo '"a-b-c"' | jq 'gsub("-"; "_")'

# Split and join
$ echo '"a,b,c"' | jq -c 'split(",")'
$ echo '["a","b","c"]' | jq -r 'join("-")'

# Trim spaces
$ echo '"  hi  "' | jq 'trim'

# Put values inside a string with \( )
$ echo '{"name":"Ada","age":36}' | jq -r '"\(.name) is \(.age) years old"'

# Length of a string
$ echo '"hello"' | jq 'length'
```

## 7. Changing things

```console
# Set a field
$ echo '{"a":1}' | jq -c '.b = 2'

# Update a field using its old value
$ echo '{"count":1}' | jq -c '.count |= . + 1'

# Delete a field
$ echo '{"a":1,"b":2}' | jq -c 'del(.a)'

# Update every item in an array
$ echo '[{"p":10},{"p":20}]' | jq -c 'map(.p *= 2)'

# Merge two objects (right side wins)
$ echo '{"a":1,"b":2}' | jq -c '. + {"b":99,"c":3}'

# Rename a key
$ echo '{"old":1}' | jq -c 'with_entries(if .key == "old" then .key = "new" else . end)'

# Add something to the end of an array
$ echo '[1,2]' | jq -c '. + [3]'
```

## 8. Keys and values

```console
# List the keys
$ echo '{"b":1,"a":2}' | jq -c 'keys'

# List the values
$ echo '{"b":1,"a":2}' | jq -c '[.[]]'

# Turn an object into key/value pairs and back
$ echo '{"a":1,"b":2}' | jq -c 'to_entries'
$ echo '[{"key":"a","value":1}]' | jq -c 'from_entries'

# Keep only some keys
$ echo '{"a":1,"b":2,"c":3}' | jq -c '{a, c}'
$ echo '{"a":1,"b":2,"c":3}' | jq -c 'with_entries(select(.key != "b"))'

# Find every path that leads to a value
$ echo '{"a":{"b":1}}' | jq -c '[paths]'

# Look at everything, however deep (recursive descent)
$ echo '{"a":{"b":1},"c":[{"b":2}]}' | jq -c '[.. | .b? // empty]'
```

## 9. Types and numbers

```console
# What type is it?
$ echo '[1,"a",null,true,[],{}]' | jq -c 'map(type)'

# Keep only the numbers
$ echo '[1,"a",2,null]' | jq -c 'map(numbers)'

# Convert between strings and numbers
$ echo '"42"' | jq 'tonumber'
$ echo '42' | jq 'tostring'

# Math
$ echo '7' | jq '. / 2'
$ echo '7.6' | jq 'floor'
$ echo '7.4' | jq 'round'
$ echo '[3,1,2]' | jq 'min, max'

# A range of numbers
$ jq -nc '[range(5)]'
$ jq -nc '[range(0; 10; 3)]'
```

## 10. Conditions

```console
# if / then / else
$ echo '5' | jq 'if . > 3 then "big" else "small" end'

# Handle null / false
$ echo 'null' | jq '. // "default"'

# Several outputs from one input (the comma)
$ echo '{"a":1,"b":2}' | jq -c '.a, .b'

# And / or / not
$ echo '{"a":true,"b":false}' | jq '.a and .b'
$ echo '{"a":true,"b":false}' | jq '.a or .b'
$ echo 'true' | jq 'not'

# Don't crash on bad input: ? swallows the error
$ echo '"text"' | jq '.[0]?'

# Or catch the error and show something else
$ echo '"text"' | jq 'try error("boom") catch "caught: \(.)"'
```

## 11. Arguments and variables

```console
# Pass a string in from the shell
$ echo '[{"n":"Ada"},{"n":"Bob"}]' | jq -c --arg who Bob 'map(select(.n == $who))'

# Pass a number or any JSON in
$ echo '[1,5,10]' | jq -c --argjson min 4 'map(select(. >= $min))'

# Pass JSON from a file (here we make a tiny file first)
$ echo '{"limit":5}' > /tmp/jq_eli5_cfg.json
$ echo '[1,5,10]' | jq -c --slurpfile cfg /tmp/jq_eli5_cfg.json 'map(select(. >= $cfg[0].limit))'

# Read environment variables
$ GREETING=hi jq -n 'env.GREETING'

# Store a value in a variable inside the program
$ echo '{"a":2,"b":3}' | jq '.a as $x | .b as $y | $x * $y'

# Make JSON from nothing (-n = no input)
$ jq -nc '{hello: "world", n: [1,2,3]}'
```

## 12. Many inputs, files, and big files

```console
# Several JSON values in one file/stream: jq handles each one
$ printf '{"a":1}\n{"a":2}\n{"a":3}\n' | jq -c '.a'

# Put them all in one array (-s = slurp)
$ printf '{"a":1}\n{"a":2}\n' | jq -sc .

# Run on a file
$ echo '{"a":[1,2,3]}' > /tmp/jq_eli5_file.json
$ jq '.a | length' /tmp/jq_eli5_file.json

# Several files at once, remembering which file each came from
$ echo '{"id":2}' > /tmp/jq_eli5_file2.json
$ jq -c '[input_filename, .]' /tmp/jq_eli5_file.json /tmp/jq_eli5_file2.json

# Read every line as plain text instead of JSON (-R = raw input)
$ printf 'one\ntwo\n' | jq -R .

# Turn text lines into a JSON array
$ printf 'one\ntwo\n' | jq -Rn '[inputs]' -c

# Count lines in a file of JSON lines
$ printf '{"a":1}\n{"a":2}\n' | jq -n '[inputs] | length'

# Only look at the first few results (stops early, great for huge data)
$ jq -nc 'first(range(1; 1000000))'
$ jq -nc '[limit(3; range(1; 1000000))]'

# Peek inside a huge file without printing all of it
$ echo '[1,2,3,4,5,6,7,8,9,10]' | jq -c 'first(.[]), length'

# Stream a giant file piece by piece instead of loading it all
$ echo '{"a":[1,2]}' | jq -c --stream .
```

Tip for big files: `jq . file.json` and `jq . < file.json` use the fast
yyjson path in this fork, and so do pipes (`cat file.json | jq .`). A pipe that
never ends (like `tail -f`) is only processed once it closes. For endless
streams use `--stream`, `--seq` or `-R`, which use the original incremental
parser.

## 13. Output formats

```console
# Rows of CSV
$ echo '[["a",1],["b",2]]' | jq -r '.[] | @csv'

# Tab separated
$ echo '[["a",1],["b",2]]' | jq -r '.[] | @tsv'

# A header row from the keys, then the rows
$ echo '[{"n":"Ada","a":36},{"n":"Bob","a":17}]' | jq -r '(.[0] | keys_unsorted) as $k | $k, (.[] | [.[$k[]]]) | @csv'

# URL-encode, base64, JSON-in-a-string
$ echo '"a b&c"' | jq -r '@uri'
$ echo '"hello"' | jq -r '@base64'
$ echo '"aGVsbG8="' | jq -r '@base64d'
$ echo '{"a":1}' | jq -c 'tojson'
$ echo '"{\"a\":1}"' | jq -c 'fromjson'

# Escape for HTML or shell
$ echo '"<b>hi</b>"' | jq -r '@html'
$ echo '"it'"'"'s"' | jq -r '@sh'

# Plain ASCII only (non-ASCII becomes \uXXXX)
$ echo '"café"' | jq -a .

# Colour on/off
$ echo '{"a":1}' | jq -C . | cat
$ echo '{"a":1}' | jq -M .

# Join with a separator, no trailing newline
$ echo '["a","b"]' | jq -rj '.[]'
```

## 14. Dates

```console
# A timestamp (seconds since 1970) to a readable date
$ echo '1700000000' | jq 'todate'

# And back
$ echo '"2023-11-14T22:13:20Z"' | jq 'fromdate'

# Pick your own format
$ echo '1700000000' | jq -r 'strftime("%Y-%m-%d")'
```

## 15. Real-world recipes

```console
# Pull one field out of every item of an API-like response
$ echo '{"items":[{"id":1,"tags":["x"]},{"id":2,"tags":["y","z"]}]}' | jq -c '.items[] | {id, first_tag: .tags[0]}'

# Flatten nested arrays
$ echo '[[1,2],[3,[4]]]' | jq -c 'flatten'

# Turn an array of objects into a lookup table keyed by id
$ echo '[{"id":"a","v":1},{"id":"b","v":2}]' | jq -c 'map({(.id): .v}) | add'

# Count how many of each value
$ echo '["a","b","a","c","a"]' | jq -c 'group_by(.) | map({(.[0]): length}) | add'

# Join two lists on a key
$ echo '{"users":[{"id":1,"n":"Ada"}],"orders":[{"uid":1,"total":9}]}' | jq -c '.users[] as $u | .orders[] | select(.uid == $u.id) | {name: $u.n, total}'

# Check that a field exists in every item (exit code 1 if not)
$ echo '[{"a":1},{"a":2}]' | jq -e 'all(.[]; has("a"))'

# Use jq as a grep for JSON lines
$ printf '{"lvl":"info"}\n{"lvl":"error","msg":"x"}\n' | jq -c 'select(.lvl == "error")'

# Pull every "file" entry out of a compile_commands.json style file
$ echo '[{"directory":"/d","command":"cc a.c","file":"a.c"},{"directory":"/d","command":"cc b.c","file":"b.c"}]' | jq -r '.[].file'

# Find entries whose command mentions a flag
$ echo '[{"file":"a.c","command":"cc -O2 a.c"},{"file":"b.c","command":"cc b.c"}]' | jq -r '.[] | select(.command | contains("-O2")) | .file'

# Does this file parse as JSON? (exit code tells you)
$ echo '{"a":1}' | jq -e . > /dev/null && echo valid
$ echo '{bad' | jq -e . > /dev/null 2>&1 || echo invalid
```

## 16. A few cheat-sheet flags

| Flag | Meaning |
|---|---|
| `-r` | print strings without quotes |
| `-c` | compact one-line output |
| `-n` | no input; start from `null` |
| `-s` | slurp all inputs into one array |
| `-R` | read raw text lines instead of JSON |
| `-e` | set exit status from the last output |
| `-S` | sort object keys |
| `-a` | ASCII-only output |
| `-C` / `-M` | force colour / no colour |
| `--arg k v` | set `$k` to the string `v` |
| `--argjson k v` | set `$k` to the JSON value `v` |
| `--slurpfile k f` | set `$k` to all JSON values in file `f` |
| `--stream` | emit `[path, leaf]` events for huge inputs |
| `--tab`, `--indent n` | choose indentation |
| `-f file.jq` | read the program from a file |

When something doesn't work: add `-c` to see the result on one line, break a
long filter into smaller pieces joined by `|`, and test each piece by itself.
