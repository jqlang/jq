### C source files to be built

JQ_INCS = jq_parser.h builtin.h bytecode.h compile.h execute.h		\
  forkable_stack.h frame_layout.h jv.h jv_alloc.h jv_aux.h jv_dtoa.h	\
  jv_parse.h jv_unicode.h locfile.h opcode.h opcode_list.h parser.y	\
  jv_utf8_tables.h

JQ_SRC = opcode.c bytecode.c compile.c execute.c builtin.c jv.c		\
  jv_parse.c jv_print.c jv_dtoa.c jv_unicode.c jv_aux.c jv_alloc.c	\
  lexer.c jq_test.c ${JQ_INCS}


### C build options

AM_CFLAGS = -Wextra -Wall -Wno-missing-field-initializers	\
  -Wno-unused-parameter -Wno-unused-function


### Generating the lexer and parser

# While there is some autoconf macro support for lex/flex, it doesn't support
# header file creation so we'll use good old make
BUILT_SOURCES = lexer.h lexer.c
lexer.c: lexer.l
	flex -o lexer.c --header-file=lexer.h lexer.l
lexer.h: lexer.c

# Tell YACC (bison) autoconf macros that you want a header file created.
# If the --warnings=all fails, you probably have an old version of bison
# OSX ships an old bison, so update with homebrew or macports
AM_YFLAGS = --warnings=all -d



### Building the jq binary

main.c: config.h

bin_PROGRAMS = jq
jq_SOURCES = ${JQ_SRC} main.c


### Tests

# this is what gets "make check" to work
TESTS = tests/all.test
TEST_LOG_COMPILER = tests/run



### Packaging

# setup is only used by distribution developers, not package developers.
# Still, as a matter of allowing patching, its not a bad idea to distribute 
# the developer setup script in the tarball.
EXTRA_DIST = setup.sh config.h.in ChangeLog VERSION lexer.l lexer.h	\
  gen_utf8_tables.py jq.spec testdata

# README.md is expected in Github projects, good stuff in it, so we'll 
# distribute it and install it with the package in the doc directory.
docdir = ${datadir}/doc/${PACKAGE}
dist_doc_DATA = README.md INSTALL COPYING AUTHORS README NEWS

man_MANS = jq.1

jq.1 :
	cd ${abs_srcdir}/docs; rake manpage > ${abs_builddir}/$@

ChangeLog:
	git log --name-status > $@ || echo "no changelog" > $@

releasetag:
	git tag -s "jq-$$(cat VERSION)" -m "jq release $$(cat VERSION)"

RELEASE ?= 1
rpm: jq
	@echo "Packaging jq as an RPM ..."
	mkdir -p rpm/SOURCES rpm/BUILD rpm/BUILDROOT rpm/RPMS rpm/SPECS
	cp jq-$$(cat VERSION).tar.gz rpm/SOURCES/
	rpmbuild -tb --define "_topdir ${PWD}/rpm" --define "_prefix /usr" --define "myver $$(cat VERSION)" --define "myrel ${RELEASE}" rpm/SOURCES/jq-$$(cat VERSION).tar.gz
	find rpm/RPMS/ -name "*.rpm" -exec mv {} ./ \;
	rm -rf rpm

dist-clean-local:
	rm -f ${BUILT_SOURCES}

# Not sure why this doesn't get cleaned up automatically, guess
# automake used to man pages which are hand coded?
clean-local:
	rm -f jq.1
