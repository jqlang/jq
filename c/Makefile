CC=gcc -Werror -Wextra -Wall -Wno-unused-parameter -std=gnu99 -ggdb -Wno-unused-function

.PHONY: all clean
all: parsertest

clean:
	make -Bnd | grep 'Must remake target' | \
	sed 's/.*`\(.*\)'\''.*/\1/' | grep -v '^all$$' | \
	xargs rm

jv_utf8_tables.h: gen_utf8_tables.py
	python $^ > $@

lexer.yy.c: lexer.l
	flex -o lexer.yy.c --header-file=lexer.yy.h lexer.l
lexer.yy.h: lexer.yy.c

parser.tab.c: parser.y lexer.yy.h
	bison -W -d parser.y -v --report-file=parser.info
parser.tab.h: parser.tab.c

jv_unicode.c: jv_utf8_tables.h

parsertest: parser.tab.c lexer.yy.c main.c opcode.c bytecode.c compile.c execute.c builtin.c jv.c jv_parse.c jv_print.c jv_dtoa.c jv_unicode.c
	$(CC) -DJQ_DEBUG=1 -o $@ $^

jq: parser.tab.c lexer.yy.c main.c opcode.c bytecode.c compile.c execute.c builtin.c jv.c jv_parse.c jv_print.c jv_dtoa.c jv_unicode.c
	$(CC) -DJQ_DEBUG=0 -o $@ $^

jv_test: jv_test.c jv.c jv_print.c jv_dtoa.c jv_unicode.c
	$(CC) -DNO_JANSSON -o $@ $^

jv_parse: jv_parse.c jv.c jv_print.c jv_dtoa.c
	$(CC) -DNO_JANSSON -o $@ $^ -DJV_PARSE_MAIN


test: jv_test
	valgrind --error-exitcode=1 -q --leak-check=full ./jv_test

