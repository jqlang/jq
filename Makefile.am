# Converted to automake by thompson@dtosolutions.com 26NOV12

# setup is only used by distribution developers, not package developers.
# Still, as a matter of allowing patching, its not a bad idea to distribute 
# the developer setup script in the tarball.
EXTRA_DIST = setup.sh config.h.in ChangeLog VERSION lexer.l lexer.h gen_utf8_tables.py jq.spec testdata

# README.md is expected in Github projects, good stuff in it, so we'll 
# distribute it and install it with the package in the doc directory.
docdir = ${datadir}/doc/${PACKAGE}
dist_doc_DATA = README.md INSTALL COPYING AUTHORS README NEWS

JQ_INCS = \
  jq_parser.h \
  builtin.h \
  bytecode.h \
  compile.h \
  execute.h \
  forkable_stack.h \
  frame_layout.h \
  jv.h \
  jv_alloc.h \
  jv_aux.h \
  jv_dtoa.h \
  jv_parse.h \
  jv_unicode.h \
  locfile.h \
  opcode.h \
  opcode_list.h \
  parser.y \
  jv_utf8_tables.gen.h

JQ_SRC = \
  opcode.c \
  bytecode.c \
  compile.c \
  execute.c \
  builtin.c \
  jv.c \
  jv_parse.c \
  jv_print.c \
  jv_dtoa.c \
  jv_unicode.c \
  jv_aux.c \
  jv_alloc.c \
  lexer.c \
  ${JQ_INCS}

if DEVELOPMENT_ENABLED
DEVCFLAGS = -Wextra -Wall -Wno-missing-field-initializers -Wno-unused-parameter -std=gnu99 -ggdb -Wno-unused-function -O0 -DJQ_DEBUG=1
RPMBINS = jq jq_test
else
DEVCFLAGS = -DJQ_DEBUG=0
RPMBINS = jq
endif

# Tell YACC (bison) autoconf macros that you want a header file created.
# If the --warnings=all fails, you probably have an old version of bison
# OSX ships an old bison, so update with homebrew or macports
AM_YFLAGS = --warnings=all -d

bin_PROGRAMS = jq
BUILT_SOURCES = jv_utf8_tables.gen.h lexer.h lexer.c
jq_SOURCES = ${JQ_SRC} main.c
jq_CPPFLAGS = ${DEVCFLAGS}

# always test, but conditionally distribute to /usr/libexec/jq/jq_test
if DEVELOPMENT_ENABLED
  pkglibexec_PROGRAMS = jq_test
  # automake doesn't like putting data in libexec (pkglibexec_DATA)
  # so we'll create our own macro and distribute testdata
  mydatadir = ${libexecdir}/${PACKAGE}
  mydata_DATA = testdata

  DEVBUILD=yes
else
  noinst_PROGRAMS = jq_test

  DEVBUILD=no
endif

jq_test_SOURCES = ${JQ_SRC} jq_test.c
jq_test_CPPFLAGS = ${DEVCFLAGS}

# this is what gets "make check" to work
TESTS = jq_test
LOG_COMPILER = valgrind
AM_LOG_FLAGS = --error-exitcode=1 -q --leak-check=full

# nasty circular dependency on header files between the generated C files
parser.o: lexer.c

lexer.o: parcer.c

lexer.c: parser.h

# While there is some autoconf macro support for lex/flex, it doesn't support
# header file creation so we'll use good old make
lexer.c: lexer.l
	flex -o lexer.c --header-file=lexer.h lexer.l
lexer.h: lexer.c

jv_utf8_tables.gen.h: gen_utf8_tables.py
	python $^ > $@

jv_unicode.c: jv_utf8_tables.gen.h

ChangeLog:
	git log --name-status > $@ || echo "no changelog" > $@

main.c: config.h

releasetag:
	git tag -s "jq-$$(cat VERSION)" -m "jq release $$(cat VERSION)"

RELEASE ?= 1
rpm: ${RPMBINS}
	@echo "Packaging jq as an RPM ..."
	mkdir -p rpm/SOURCES rpm/BUILD rpm/BUILDROOT rpm/RPMS rpm/SPECS
	cp jq-$$(cat VERSION).tar.gz rpm/SOURCES/
	rpmbuild -tb --define "_topdir ${PWD}/rpm" --define "_prefix /usr" --define "myver $$(cat VERSION)" --define "myrel ${RELEASE}" --define "devbuild ${DEVBUILD}" rpm/SOURCES/jq-$$(cat VERSION).tar.gz
	find rpm/RPMS/ -name "*.rpm" -exec mv {} ./ \;
	rm -rf rpm

dist-clean-local:
	rm -f ${BUILT_SOURCES}
