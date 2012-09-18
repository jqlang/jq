CC=gcc -Wextra -Wall -Wno-missing-field-initializers -Wno-unused-parameter -std=gnu99 -ggdb -Wno-unused-function

.PHONY: all clean
all: parsertest

clean:
	make -Bnd | grep 'Must remake target' | \
	sed 's/.*`\(.*\)'\''.*/\1/' | grep -v '^all$$' | \
	xargs rm


lexer.gen.c: lexer.l
	flex -o lexer.gen.c --header-file=lexer.gen.h lexer.l
lexer.gen.h: lexer.gen.c

parser.gen.c: parser.y lexer.gen.h
	bison -W -d parser.y -v --report-file=parser.gen.info -o $@
parser.gen.h: parser.gen.c

jv_utf8_tables.gen.h: gen_utf8_tables.py
	python $^ > $@
jv_unicode.c: jv_utf8_tables.gen.h

JQ_SRC=parser.gen.c lexer.gen.c opcode.c bytecode.c compile.c execute.c builtin.c jv.c jv_parse.c jv_print.c jv_dtoa.c jv_unicode.c


jq_test: $(JQ_SRC) jq_test.c
	$(CC) -DJQ_DEBUG=1 -o $@ $^

jq: $(JQ_SRC) main.c
	$(CC) -DJQ_DEBUG=0 -o $@ $^


test: jq_test
	valgrind --error-exitcode=1 -q --leak-check=full ./jq_test >/dev/null

