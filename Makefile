CC=gcc -Wextra -Wall -Wno-missing-field-initializers -Wno-unused-parameter -std=gnu99 -ggdb -Wno-unused-function

.PHONY: all clean
all: parsertest

clean:
	make -Bnd | grep 'Must remake target' | \
	sed 's/.*`\(.*\)'\''.*/\1/' | grep -v '^all$$' | \
	xargs rm

jv_utf8_tables.gen.h: gen_utf8_tables.py
	python $^ > $@

lexer.gen.c: lexer.l
	flex -o lexer.gen.c --header-file=lexer.gen.h lexer.l
lexer.gen.h: lexer.gen.c

parser.gen.c: parser.y lexer.gen.h
	bison -W -d parser.y -v --report-file=parser.gen.info -o $@
parser.gen.h: parser.gen.c

jv_unicode.c: jv_utf8_tables.gen.h

parsertest: parser.gen.c lexer.gen.c main.c opcode.c bytecode.c compile.c execute.c builtin.c jv.c jv_parse.c jv_print.c jv_dtoa.c jv_unicode.c
	$(CC) -DJQ_DEBUG=1 -o $@ $^

jq: parser.gen.c lexer.gen.c main.c opcode.c bytecode.c compile.c execute.c builtin.c jv.c jv_parse.c jv_print.c jv_dtoa.c jv_unicode.c
	$(CC) -DJQ_DEBUG=0 -o $@ $^

jv_test: jv_test.c jv.c jv_print.c jv_dtoa.c jv_unicode.c
	$(CC) -DNO_JANSSON -o $@ $^

jv_parse: jv_parse.c jv.c jv_print.c jv_dtoa.c
	$(CC) -DNO_JANSSON -o $@ $^ -DJV_PARSE_MAIN


test: jv_test
	valgrind --error-exitcode=1 -q --leak-check=full ./jv_test

