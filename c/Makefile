CC=gcc -Wall -std=gnu99 -ggdb

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
	bison -W -d parser.y
parser.tab.h: parser.tab.c

parsertest: parser.tab.c lexer.yy.c main.c opcode.c bytecode.c compile.c execute.c builtin.c
	$(CC) -o $@ $^ -ljansson
