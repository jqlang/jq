CC=gcc -Wextra -Wall -Wno-missing-field-initializers -Wno-unused-parameter -std=gnu99 -ggdb -Wno-unused-function
prefix=/usr/local

.PHONY: all clean releasedep tarball install uninstall test releasetag
all: jq


lexer.gen.c: lexer.l
	flex -o lexer.gen.c --header-file=lexer.gen.h lexer.l
lexer.gen.h: lexer.gen.c

parser.gen.c: parser.y lexer.gen.h
	bison -W -d parser.y -v --report-file=parser.gen.info -o $@
parser.gen.h: parser.gen.c

jv_utf8_tables.gen.h: gen_utf8_tables.py
	python $^ > $@
jv_unicode.c: jv_utf8_tables.gen.h

version.gen.h: VERSION
	sed 's/.*/#define JQ_VERSION "&"/' $^ > $@
main.c: version.gen.h

JQ_SRC=parser.gen.c lexer.gen.c opcode.c bytecode.c compile.c execute.c builtin.c jv.c jv_parse.c jv_print.c jv_dtoa.c jv_unicode.c jv_aux.c


jq_test: $(JQ_SRC) jq_test.c
	$(CC) -DJQ_DEBUG=1 -o $@ $^

jq: $(JQ_SRC) main.c
	$(CC) -O -DJQ_DEBUG=0 -o $@ $^

test: jq_test
	valgrind --error-exitcode=1 -q --leak-check=full ./jq_test >/dev/null


releasedep: lexer.gen.c parser.gen.c jv_utf8_tables.gen.h

releasetag:
	git tag -s "jq-$$(cat VERSION)" -m "jq release $$(cat VERSION)"

docs/content/2.download/source/jq.tgz: jq
	mkdir -p `dirname $@`
	tar -czvf $@ `git ls-files; ls *.gen.*`

tarball: docs/content/2.download/source/jq.tgz

install: jq
	install -d -m 0755 $(prefix)/bin
	install -m 0755 jq $(prefix)/bin

uninstall:
	test -d $(prefix)/bin && \
	cd $(prefix)/bin && \
	rm -f jq

