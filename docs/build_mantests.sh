#!/bin/sh
mkdir -p tests
pandoc content/manual/dev/manual.md --lua-filter filters/tests.lua | split --separator='\0' -l1 - tests/
REGEX="test|match|capture|scan|split|splits|sub|gsub"
grep -L -E $REGEX tests/* | xargs cat | sed 's/\x0/\n/g' > ../tests/man.test
grep -l -E $REGEX tests/* | xargs cat | sed 's/\x0/\n/g' > ../tests/manonig.test
rm -r tests
