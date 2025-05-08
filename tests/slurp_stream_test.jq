
#!/usr/bin/env jq -n -f

# Test cases for slurp+stream with and without trailing newlines

def test_case($name; $input):
  "TEST: \($name)",
  ("Input: \($input | @json)",
   "Slurp+stream output:",
   ($input | jq -c -s --stream .));

# Test cases
test_case("Array with newline"; "[1]\n"),
test_case("Array without newline"; "[1]"),
test_case("Empty array with newline"; "[]\n"), 
test_case("Empty array without newline"; "[]"),
test_case("Object with newline"; "{\"a\":1}\n"),
test_case("Object without newline"; "{\"a\":1}")
