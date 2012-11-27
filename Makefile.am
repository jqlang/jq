# Converted to automake by thompson@dtosolutions.com 26NOV12

# setup is only used by distribution developers, not package developers.
# Still, as a matter of allowing patching, its not a bad idea to distribute 
# the developer setup script in the tarball.
EXTRA_DIST = setup.sh config.h.in ChangeLog

# README.md is expected in Github projects, good stuff in it, so we'll 
# distribute it and install it with the package in the doc directory.
docdir = ${datadir}/doc/${PACKAGE}
dist_doc_DATA = README.md INSTALL COPYING AUTHORS README NEWS

JQ_SRC = opcode.c bytecode.c compile.c execute.c builtin.c jv.c jv_parse.c jv_print.c jv_dtoa.c jv_unicode.c
# If we distribute the generated files in the source release, packagers won't have to have
# python, bison, and flex
DIST_JQ_SRC = parser.gen.c lexer.gen.c jv_utf8_tables.gen.h parser.gen.h lexer.gen.h

bin_PROGRAMS = jq jq_test
if DEVCFLAGS_ENABLED
DEVCFLAGS = -Wextra -Wall -Wno-missing-field-initializers -Wno-unused -parameter -std=gnu99 -ggdb -Wno-unused-function
AM_CPPFLAGS = $(DEVCFLAGS)
endif

jq_SOURCES = $(JQ_SRC) main.c
dist_jq_SOURCES = $(DIST_JQ_SRC)
jq_CPPFLAGS = -O $(DEVCFLAGS) -DJQ_DEBUG=0


TESTS = jq_test
jq_test_SOURCES = $(JQ_SRC) jq_test.c
dist_jq_test_SOURCES = $(DIST_JQ_SRC)
jq_test_CPPFLAGS = $(DEVCFLAGS) -DJQ_DEBUG=1
LOG_COMPILER = valgrind
AM_LOG_FLAGS = --error-exitcode=1 -q --leak-check=full

lexer.gen.c: lexer.l
	flex -o lexer.gen.c --header-file=lexer.gen.h lexer.l
lexer.gen.h: lexer.gen.c

# if this fails on OSX, install bison from macports or homebrew.  OSX comes
# with bison 2.3 which is too old
parser.gen.c: parser.y lexer.gen.h
	bison --warnings=all -d parser.y -v --report-file=parser.gen.info -o $@
parser.gen.h: parser.gen.c

jv_utf8_tables.gen.h: gen_utf8_tables.py
	python $^ > $@
jv_unicode.c: jv_utf8_tables.gen.h

ChangeLog:
	git log --name-status > $@

main.c: config.h

releasedep: lexer.gen.c parser.gen.c jv_utf8_tables.gen.h

releasetag:
	git tag -s "jq-$$(cat VERSION)" -m "jq release $$(cat VERSION)"

# can probably delete and have RPM created off autoconf generated tarball releases
RELEASE ?= 1
rpmbuild: jq
	@echo "Packaging jq as an RPM ..."
	mkdir -p rpm/SOURCES rpm/BUILD rpm/BUILDROOT rpm/RPMS
	rm -rf jq-$$(cat VERSION)
	mkdir -p jq-$$(cat VERSION)/bin
	cp jq jq-$$(cat VERSION)/bin
	tar -zcf rpm/SOURCES/jq-$$(cat VERSION).tgz jq-$$(cat VERSION)/bin/jq
	rpmbuild --target $$(uname -m) --buildroot ${PWD}/rpm/BUILDROOT/jq-$$(cat VERSION)-${RELEASE}.noarch --define "_topdir ${PWD}/rpm" --define "version $$(cat VERSION)" --define "release ${RELEASE}" -bb --clean rpm/SPECS/jq.spec
