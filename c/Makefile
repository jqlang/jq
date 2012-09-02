CC=gcc -Wall -std=gnu99 -ggdb -Wno-unused-function

.PHONY: all clean
all: parsertest

clean:
	make -Bnd | grep 'Must remake target' | \
	sed 's/.*`\(.*\)'\''.*/\1/' | grep -v '^all$$' | \
	xargs rm


lexer.yy.c: lexer.l
	flex -o lexer.yy.c --header-file=lexer.yy.h lexer.l
lexer.yy.h: lexer.yy.c

parser.tab.c: parser.y lexer.yy.h
	bison -W -d parser.y -v --report-file=parser.info
parser.tab.h: parser.tab.c

parsertest: parser.tab.c lexer.yy.c main.c opcode.c bytecode.c compile.c execute.c builtin.c jv.c jv_parse.c jv_print.c jv_dtoa.c
	$(CC) -o $@ $^

jvtest: jvtest.c jv.c jv_print.c
	$(CC) -DNO_JANSSON -o $@ $^

jv_parse: jv_parse.c jv.c jv_print.c jv_dtoa.c
	$(CC) -DNO_JANSSON -o $@ $^ -DJV_PARSE_MAIN


test: jvtest
	valgrind --error-exitcode=1 -q --leak-check=full ./jvtest

