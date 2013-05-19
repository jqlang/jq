### C source files to be built and distributed.

JQ_INCS = jq_parser.h builtin.h bytecode.h compile.h execute.h		\
  forkable_stack.h frame_layout.h jv.h jv_alloc.h jv_aux.h jv_dtoa.h	\
  jv_parse.h jv_unicode.h locfile.h opcode.h opcode_list.h parser.y	\
  jv_utf8_tables.h lexer.l

JQ_SRC = opcode.c bytecode.c compile.c execute.c builtin.c jv.c		\
  jv_parse.c jv_print.c jv_dtoa.c jv_unicode.c jv_aux.c jv_alloc.c	\
  jq_test.c ${JQ_INCS}


### C build options

AM_CFLAGS = -Wextra -Wall -Wno-missing-field-initializers	\
  -Wno-unused-parameter -Wno-unused-function


### Generating the lexer and parser

# While there is some autoconf macro support for lex/flex, it doesn't support
# header file creation so we'll use good old make
BUILT_SOURCES = lexer.h lexer.c parser.h parser.c
lexer.c: lexer.l
	flex -o lexer.c --header-file=lexer.h $<
lexer.h: lexer.c

# Tell YACC (bison) autoconf macros that you want a header file created.
# If the --warnings=all fails, you probably have an old version of bison
# OSX ships an old bison, so update with homebrew or macports
AM_YFLAGS = --warnings=all -d


### Building the jq binary

main.c: config.h

bin_PROGRAMS = jq
jq_SOURCES = ${JQ_SRC} main.c


### Tests (make check)

TESTS = tests/all.test
TEST_LOG_COMPILER = ${srcdir}/tests/run


### Building the manpage

# If ENABLE_DOCS is not set, just copy jq.1.default to jq.1
# The real_docs target succeeds (possibly after building jq.1) only if ENABLE_DOCS is set
# Thus, making "dist" depend on "real_docs" ensures that we never build a tarball with
# a stub manpage.

man_MANS = jq.1
.PHONY: real_docs
if ENABLE_DOCS
jq.1: $(srcdir)/docs/content/3.manual/manual.yml
	( cd ${abs_srcdir}/docs; '$(BUNDLER)' exec rake manpage ) > $@ || { rm -f $@; false; }
jq.1.default: $(srcdir)/docs/default_manpage.md
	( cd ${abs_srcdir}/docs; '$(BUNDLER)' exec rake manpage_default ) > $@ || { rm -f $@; false; }
real_docs: jq.1
	if cmp jq.1 $(srcdir)/jq.1.default > /dev/null; then\
	  rm -f jq.1; $(MAKE) $(AM_MAKEFLAGS) jq.1;\
	fi
else
jq.1: $(srcdir)/jq.1.default
	cp $^ $@
real_docs:
	@echo "Ruby dependencies not found, cannot build manpage." > /dev/stderr
	@echo "Follow the instructions in docs/README.md to install them" > /dev/stderr
	@echo "and then rerun ./configure" > /dev/stderr
	false
endif


### Packaging

docs/site.yml: configure.ac
	sed 's/^jq_version: .*/jq_version: "$(VERSION)"/' $@ > $@.new
	mv $@.new $@

install-binaries: $(BUILT_SOURCES)
	$(MAKE) $(AM_MAKEFLAGS) install-exec

# Ensure "make dist" fails when we can't build the real manpage
dist-hook: real_docs

DOC_FILES = docs/content docs/public docs/templates docs/site.yml	\
            docs/Gemfile docs/Gemfile.lock docs/Rakefile docs/README.md \
            docs/default_manpage.md jq.1.default

# setup is only used by distribution developers, not package developers.
# Still, as a matter of allowing patching, its not a bad idea to distribute 
# the developer setup script in the tarball.
EXTRA_DIST = config.h.in $(BUILT_SOURCES) $(man_MANS) $(TESTS)	\
	$(TEST_LOG_COMPILER) gen_utf8_tables.py jq.spec \
	$(DOC_FILES)

# README.md is expected in Github projects, good stuff in it, so we'll 
# distribute it and install it with the package in the doc directory.
docdir = ${datadir}/doc/${PACKAGE}
dist_doc_DATA = README.md COPYING AUTHORS README

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
