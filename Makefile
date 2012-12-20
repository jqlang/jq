CC=gcc
CFLAGS=-Wextra -Wall -Wno-missing-field-initializers -Wno-unused-parameter -std=gnu99 -ggdb -Wno-unused-function $(EXTRA_CFLAGS)

prefix=/usr/local
mandir=$(prefix)/share/man


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

JQ_SRC=parser.gen.c lexer.gen.c opcode.c bytecode.c compile.c execute.c builtin.c jv.c jv_parse.c jv_print.c jv_dtoa.c jv_unicode.c jv_aux.c jv_alloc.c

jq_test: CFLAGS += -DJQ_DEBUG=1
jq_test: $(JQ_SRC) jq_test.c
	$(CC) $(CFLAGS) $(CFLAGS_DEBUG) -o $@ $^

jq: CFLAGS += -O -DJQ_DEBUG=0
jq: $(JQ_SRC) main.c
	$(CC) $(CFLAGS) $(CFLAGS_OPT) -o $@ $^

test: jq_test
	valgrind --error-exitcode=1 -q --leak-check=full ./jq_test >/dev/null

BINARIES=jq jq_test
PLATFORMS=linux32 linux64 osx32 osx64 win32 win64

build/linux32%: CC='x86_64-linux-gnu-gcc -m32'
build/linux64%: CC='x86_64-linux-gnu-gcc -m64'

# OS X cross compilers can be gotten from
# https://launchpad.net/~flosoft/+archive/cross-apple
build/osx32%:   CC='i686-apple-darwin10-gcc -m32'
build/osx64%:   CC='i686-apple-darwin10-gcc -m64'

# On Debian, you can get windows compilers in the
# gcc-mingw-w64-i686 and gcc-mingw-w64-x86-64 packages.
build/win32%:   CC='i686-w64-mingw32-gcc -m32'   EXTRA_CFLAGS=-DJQ_DEFAULT_ENABLE_COLOR=0
build/win64%:   CC='x86_64-w64-mingw32-gcc -m64' EXTRA_CFLAGS=-DJQ_DEFAULT_ENABLE_COLOR=0

ALL_BINARIES=$(foreach platform, $(PLATFORMS), $(foreach binary, $(BINARIES), build/$(platform)/$(binary)))

$(ALL_BINARIES): build/%:
	mkdir -p $(@D)
	make -B $(BINARIES) CC=$(CC)
	cp $(BINARIES) $(@D)

binaries: $(ALL_BINARIES)

clean:
	rm -rf build
	rm -f $(BINARIES) *.gen.*

releasedep: lexer.gen.c parser.gen.c jv_utf8_tables.gen.h

releasetag:
	git tag -s "jq-$$(cat VERSION)" -m "jq release $$(cat VERSION)"

docs/content/2.download/source/jq.tgz: jq
	mkdir -p `dirname $@`
	tar -czvf $@ `git ls-files; ls *.gen.*`

tarball: docs/content/2.download/source/jq.tgz

jq.1: docs/content/3.manual/manual.yml
	( cd docs; rake manpage; ) > $@

install: jq jq.1
	install -d -m 0755 $(prefix)/bin
	install -m 0755 jq $(prefix)/bin
	install -d -m 0755 $(mandir)/man1
	install -m 0755 jq.1 $(mandir)/man1

uninstall:
	rm -vf $(prefix)/bin/jq
	rm -vf $(mandir)/man1/jq.1

