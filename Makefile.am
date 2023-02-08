
### C source files to be built and distributed.

LIBJQ_INCS = src/builtin.h src/bytecode.h src/compile.h                 \
        src/exec_stack.h src/jq_parser.h src/jv_alloc.h src/jv_dtoa.h   \
        src/jv_unicode.h src/jv_utf8_tables.h src/lexer.l src/libm.h    \
        src/linker.h src/locfile.h src/opcode_list.h src/parser.y       \
        src/util.h src/decNumber/decContext.h src/decNumber/decNumber.h \
        src/decNumber/decNumberLocal.h src/jv_dtoa_tsd.h src/jv_thread.h \
        src/jv_type_private.h

LIBJQ_SRC = src/builtin.c src/bytecode.c src/compile.c src/execute.c    \
        src/jq_test.c src/jv.c src/jv_alloc.c src/jv_aux.c              \
        src/jv_dtoa.c src/jv_file.c src/jv_parse.c src/jv_print.c       \
        src/jv_unicode.c src/linker.c src/locfile.c src/util.c          \
        src/decNumber/decContext.c src/decNumber/decNumber.c            \
        src/jv_dtoa_tsd.c                                               \
        ${LIBJQ_INCS}

### C build options

AM_CFLAGS = -Wextra -Wall -Wno-missing-field-initializers               \
        -Wno-unused-parameter -Wno-unused-function

if WIN32
AM_CFLAGS += -municode
endif

ACLOCAL_AMFLAGS = -I config/m4

### Generating the lexer and parser

# While there is some autoconf macro support for lex/flex, it doesn't support
# header file creation so we'll use good old make
if MAINTAINER_MODE
BUILT_SOURCES = src/lexer.h src/lexer.c src/parser.h src/parser.c 	\
		src/builtin.inc src/version.h
src/lexer.c: src/lexer.l
	$(AM_V_LEX) flex -o src/lexer.c --header-file=src/lexer.h $<
src/lexer.h: src/lexer.c
else
BUILT_SOURCES = src/builtin.inc src/version.h
.y.c:
	$(AM_V_YACC) echo "NOT building parser.c!"
.l.c:
	$(AM_V_LEX) echo "NOT building lexer.c!"
endif

# Tell YACC (bison) autoconf macros that you want a header file created.
# If the --warnings=all fails, you probably have an old version of bison
# OSX ships an old bison, so update with homebrew or macports
AM_YFLAGS = --warnings=all -d

### libjq

lib_LTLIBRARIES = libjq.la
libjq_la_SOURCES = ${LIBJQ_SRC}
libjq_la_LIBADD = -lm
libjq_la_LDFLAGS = $(onig_LDFLAGS) -export-symbols-regex '^j[qv]_' -version-info 1:4:0

if WIN32
libjq_la_LIBADD += -lshlwapi
libjq_la_LDFLAGS += -no-undefined
endif

include_HEADERS = src/jv.h src/jq.h

if ENABLE_UBSAN
AM_CFLAGS += -fsanitize=undefined
endif

AM_CPPFLAGS = -I$(srcdir)/src

### Running tests under Valgrind

if ENABLE_ASAN
AM_CFLAGS += -fsanitize=address
NO_VALGRIND = 1
else
if ENABLE_VALGRIND
NO_VALGRIND =
else
NO_VALGRIND = 1
endif
endif

### Code coverage with gcov

if ENABLE_GCOV
AM_CFLAGS += --coverage --no-inline
endif

### Error injection for testing

if ENABLE_ERROR_INJECTION
lib_LTLIBRARIES += libinject_errors.la
libinject_errors_la_SOURCES = src/inject_errors.c
libinject_errors_la_LIBADD = -ldl
libinject_errors_la_LDFLAGS = -module
endif

### Building the jq binary

# Remake the version.h header file if, and only if, the git ID has changed
.PHONY: .FORCE
.FORCE:
generate_ver = ver="`{ $(srcdir)/scripts/version || echo '$(VERSION)' ; } | sed 's/.*/\#define JQ_VERSION \"&\"/'`"
.remake-version-h: .FORCE
	@ $(generate_ver); test "x`cat src/version.h 2>/dev/null`" = "x$$ver" || touch .remake-version-h
src/version.h: .remake-version-h
	mkdir -p src
	$(AM_V_GEN) $(generate_ver); echo "$$ver" > $@
src/main.c: src/version.h

src/builtin.inc: $(srcdir)/src/builtin.jq
	mkdir -p src
	$(AM_V_GEN) sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/^/"/' -e 's/$$/\\n"/' $(srcdir)/src/builtin.jq > $@
src/builtin.o: src/builtin.inc

bin_PROGRAMS = jq
jq_SOURCES = src/main.c src/version.h
jq_LDFLAGS = -static-libtool-libs
jq_LDADD = libjq.la -lm

if WIN32
jq_LDADD += -lshlwapi
endif

if ENABLE_ALL_STATIC
jq_LDFLAGS += -all-static
endif

### Tests (make check)

TESTS = tests/optionaltest tests/mantest tests/jqtest tests/onigtest tests/shtest tests/utf8test tests/base64test
TESTS_ENVIRONMENT = NO_VALGRIND=$(NO_VALGRIND)

# This is a magic make variable that causes it to treat tests/man.test as a
# DATA-type dependency for the check target. As a result, it will attempt to
# run any defined targets for tests/man.test as a dependency for check. This
# allows us to ensure that the tests are up-to-date if the manual has been updated
check_DATA = tests/man.test

### Building the man tests

# We use the examples in the manual as additional tests, to ensure they always work.
# As a result, we need to rebuild the tests if the manual has been updated.
# Making changes to the manpage without having the python deps means your
# tests won't run. If you aren't making changes to the examples, you probably
# don't care. But if you are, then you need to run the tests anyway.
tests/man.test: $(srcdir)/docs/content/manual/manual.yml
if ENABLE_DOCS
	$(AM_V_GEN) ( cd ${abs_srcdir}/docs; $(PIPENV) run python build_mantests.py ) > $@
else
	@echo Changes to the manual.yml require docs to be enabled to run the tests
	@false
endif

### Building the manpage

# We build the docs from the manpage yml. If no changes have been made to the
# manpage, then we'll end up using the cached version. Otherwise, we need to
# rebuild it.
man_MANS = jq.1
jq.1.prebuilt: $(srcdir)/docs/content/manual/manual.yml
if ENABLE_DOCS
	$(AM_V_GEN) ( cd ${abs_srcdir}/docs; $(PIPENV) run python build_manpage.py ) > $@ || { rm -f $@; false; }
else
	@echo Changes to the manual.yml require docs to be enabled to generate an updated manpage
	@echo As a result, your manpage is out of date.
endif

jq.1: jq.1.prebuilt
	$(AM_V_GEN) cp $(srcdir)/jq.1.prebuilt $@

### Build oniguruma

if BUILD_ONIGURUMA
libjq_la_LIBADD += modules/oniguruma/src/.libs/libonig.la
SUBDIRS = modules/oniguruma
endif

AM_CFLAGS += $(onig_CFLAGS)

### Packaging

docs/site.yml: configure.ac
	sed 's/^jq_version: .*/jq_version: "$(VERSION)"/' $@ > $@.new
	mv $@.new $@

install-binaries: $(BUILT_SOURCES)
	$(MAKE) $(AM_MAKEFLAGS) install-exec

DOC_FILES = docs/content docs/public docs/templates docs/site.yml       \
        docs/Pipfile docs/Pipfile.lock docs/build_manpage.py            \
        docs/build_manpage.py docs/README.md jq.1.prebuilt

EXTRA_DIST = $(DOC_FILES) $(man_MANS) $(TESTS) $(TEST_LOG_COMPILER)     \
        jq.1.prebuilt jq.spec src/lexer.c src/lexer.h src/parser.c      \
        src/parser.h src/version.h src/builtin.jq scripts/version       \
        libjq.pc                                                        \
        tests/base64.test tests/jq-f-test.sh tests/jq.test              \
        tests/modules/a.jq tests/modules/b/b.jq tests/modules/c/c.jq    \
        tests/modules/c/d.jq tests/modules/data.json                    \
        tests/modules/home1/.jq tests/modules/home2/.jq/g.jq            \
        tests/modules/lib/jq/e/e.jq tests/modules/lib/jq/f.jq           \
        tests/modules/shadow1.jq tests/modules/shadow2.jq               \
        tests/modules/syntaxerror/syntaxerror.jq                        \
        tests/modules/test_bind_order.jq                                \
        tests/modules/test_bind_order0.jq                               \
        tests/modules/test_bind_order1.jq                               \
        tests/modules/test_bind_order2.jq                               \
        tests/onig.supp tests/local.supp                                \
        tests/onig.test tests/setup tests/torture/input0.json		\
        tests/optional.test tests/optionaltest				\
        tests/utf8-truncate.jq tests/utf8test				\
        tests/base64.test tests/base64test 				\
        tests/jq-f-test.sh tests/shtest

DISTCHECK_CONFIGURE_FLAGS=--disable-maintainer-mode --with-oniguruma=builtin

# README.md is expected in Github projects, good stuff in it, so we'll
# distribute it and install it with the package in the doc directory.
docdir = ${datadir}/doc/${PACKAGE}
dist_doc_DATA = README.md COPYING AUTHORS README

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libjq.pc

RELEASE ?= 1
rpm: dist jq.spec
	@echo "Packaging jq as an RPM ..."
	mkdir -p rpm/SOURCES rpm/BUILD rpm/BUILDROOT rpm/RPMS rpm/SPECS
	cp jq-$(VERSION).tar.gz rpm/SOURCES/
	rpmbuild -tb --define "_topdir ${PWD}/rpm" --define "_prefix /usr" --define "myver $(VERSION)" --define "myrel ${RELEASE}" rpm/SOURCES/jq-$(VERSION).tar.gz
	find rpm/RPMS/ -name "*.rpm" -exec mv {} ./ \;
	rm -rf rpm

dist-clean-local:
	rm -f ${BUILT_SOURCES}

# Not sure why this doesn't get cleaned up automatically, guess
# automake used to man pages which are hand coded?
# 'make clean' doesn't delete the manpage if it can't be rebuilt
clean-local-docs:
if ENABLE_DOCS
	rm -f jq.1
endif

clean-local-gcov:
	rm -f src/*.gcno src/*.gcda src/*.gcov

clean-local: clean-local-docs clean-local-gcov
	rm -f src/version.h .remake-version-h

.PHONY: clean-local-docs clean-local-gcov
