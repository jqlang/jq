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


jq: $(JQ_SRC) main.c jq_test.c
	$(CC) $(CFLAGS) $(CFLAGS_OPT) -o $@ $^

test: jq
	valgrind --error-exitcode=1 -q --leak-check=full ./jq --run-tests >/dev/null

LIBRARIES=libjq
BINARIES=jq
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

BIN_SUFFIX_win32 = .exe
BIN_SUFFIX_win64 = .exe
LIB_SUFFIX_win32 = .dll
LIB_SUFFIX_win64 = .dll

LIB_SUFFIX_linux32 = .so
LIB_SUFFIX_linux64 = .so

LIB_SUFFIX_osx32 = .so
LIB_SUFFIX_osx64 = .so

ALL_BINARIES=\
  $(foreach platform, $(PLATFORMS), \
    $(foreach binary, $(BINARIES), \
      build/$(platform)/$(binary)$(BIN_SUFFIX_$(platform))))

$(ALL_BINARIES): build/%:
	mkdir -p $(@D)
	echo $(dir $*)
	make -B $(BINARIES) CC=$(CC)
	$(foreach binary, $(BINARIES), cp $(binary) $(@D)/$(binary)$(suffix $*);)

libjq: CFLAGS += -fPIC
libjq: $(JQ_SRC)
	$(CC) -shared -Wl,-soname,libjq.so.1 $(CFLAGS) $(CFLAGS_OPT) -o $@ $^

ALL_LIBRARIES=\
  $(foreach platform, $(PLATFORMS), \
    $(foreach library, $(LIBRARIES), \
      build/$(platform)/$(library)$(LIB_SUFFIX_$(platform))))

$(ALL_LIBRARIES): build/%:
	mkdir -p $(@D)
	echo $(dir $*)
	make -B $(LIBRARIES) CC=$(CC)
	$(foreach library, $(LIBRARIES), cp $(library) $(@D)/$(library)$(suffix $*);)

binaries: $(ALL_BINARIES)
libraries: $(ALL_LIBRARIES)

clean:
	rm -rf build
	rm -f $(LIBRARIES) $(BINARIES) *.gen.*

releasedep: lexer.gen.c parser.gen.c jv_utf8_tables.gen.h

releasetag:
	git tag -s "jq-$$(cat VERSION)" -m "jq release $$(cat VERSION)"

docs/content/2.download/source/jq.tgz: jq
	mkdir -p `dirname $@`
	tar -czvf $@ `git ls-files; ls *.gen.*`

tarball: docs/content/2.download/source/jq.tgz

jq.1: docs/content/3.manual/manual.yml
	( cd docs; bundle exec rake manpage; ) > $@

install: jq jq.1 libjq
	install -d -m 0755 $(prefix)/bin
	install -d -m 0755 $(prefix)/lib
	install -d -m 0755 $(prefix)/include
	install -d -m 0755 $(prefix)/include/jq
	install -m 0755 jq $(prefix)/bin
	ln libjq libjq.so.1
	install -m 0755 execute.h $(prefix)/include/jq
	install -m 0755 compile.h $(prefix)/include/jq
	install -m 0755 jv.h $(prefix)/include/jq
	install -m 0755 jv_parse.h $(prefix)/include/jq
	install -m 0755 jv_alloc.h $(prefix)/include/jq
	install -m 0755 libjq.so.1 $(prefix)/bin
	install -m 0644 execute.h $(prefix)/bin
	install -d -m 0755 $(mandir)/man1
	install -m 0644 jq.1 $(mandir)/man1

uninstall:
	rm -vf $(prefix)/bin/jq
	rm -vf $(prefix)/lib/libjq.so.1
	rm -vf $(mandir)/man1/jq.1


www: docs/output www_binaries

docs/output:
	cd docs; rake build

www_binaries: docs/output binaries
	$(foreach platform, $(PLATFORMS), \
	  mkdir -p docs/output/download/$(platform); \
	  cp build/$(platform)/* docs/output/download/$(platform)/; )
