# libutf8proc Makefile

# programs
AR?=ar
CC?=gcc
INSTALL=install
FIND=find
PERL=perl

# compiler settings
CFLAGS ?= -O2
PICFLAG = -fPIC
C99FLAG = -std=c99
WCFLAGS = -Wsign-conversion -Wall -Wextra -pedantic
UCFLAGS = $(CPPFLAGS) $(CFLAGS) $(PICFLAG) $(C99FLAG) $(WCFLAGS) -DUTF8PROC_EXPORTS $(UTF8PROC_DEFINES)
LDFLAG_SHARED = -shared
SOFLAG = -Wl,-soname

# shared-library version MAJOR.MINOR.PATCH ... this may be *different*
# from the utf8proc version number because it indicates ABI compatibility,
# not API compatibility: MAJOR should be incremented whenever *binary*
# compatibility is broken, even if the API is backward-compatible.
# The API version number is defined in utf8proc.h.
# Be sure to also update these ABI versions in MANIFEST and CMakeLists.txt!
MAJOR=2
MINOR=6
PATCH=0

OS := $(shell uname)
ifeq ($(OS),Darwin) # MacOS X
  SHLIB_EXT = dylib
  SHLIB_VERS_EXT = $(MAJOR).dylib
else # GNU/Linux, at least (Windows should probably use cmake)
  SHLIB_EXT = so
  SHLIB_VERS_EXT = so.$(MAJOR).$(MINOR).$(PATCH)
endif

# installation directories (for 'make install')
prefix=/usr/local
libdir=$(prefix)/lib
includedir=$(prefix)/include
pkgconfigdir=$(libdir)/pkgconfig

pkglibdir=$(libdir:$(prefix)/%=%)
pkgincludedir=$(includedir:$(prefix)/%=%)

# meta targets

.PHONY: all clean data update manifest install

all: libutf8proc.a libutf8proc.$(SHLIB_EXT)

distdir:
	mkdir -p ${distdir}
	rm -f /tmp/utf8proc.distdir.tar
	tar cf /tmp/utf8proc.distdir.tar `git ls-files`
	(cd ${distdir} && tar xf /tmp/utf8proc.distdir.tar)
	rm -f /tmp/utf8proc.distdir.tar

clean:
	rm -f utf8proc.o libutf8proc.a libutf8proc.$(SHLIB_VERS_EXT) libutf8proc.$(SHLIB_EXT)
	rm -f libutf8proc.pc
ifneq ($(OS),Darwin)
	rm -f libutf8proc.so.$(MAJOR)
endif
	rm -f test/tests.o test/normtest test/graphemetest test/printproperty test/charwidth test/valid test/iterate test/case test/custom test/misc test/iscase
	rm -rf MANIFEST.new tmp
	$(MAKE) -C bench clean
	$(MAKE) -C data clean

data: data/utf8proc_data.c.new

update: data/utf8proc_data.c.new
	cp -f data/utf8proc_data.c.new utf8proc_data.c

manifest: MANIFEST.new

# real targets

data/utf8proc_data.c.new: libutf8proc.$(SHLIB_EXT) data/data_generator.rb data/charwidths.jl
	$(MAKE) -C data utf8proc_data.c.new

utf8proc.o: utf8proc.h utf8proc.c utf8proc_data.c
	$(CC) $(UCFLAGS) -c -o utf8proc.o utf8proc.c

libutf8proc.a: utf8proc.o
	rm -f libutf8proc.a
	$(AR) crs libutf8proc.a utf8proc.o

libutf8proc.so.$(MAJOR).$(MINOR).$(PATCH): utf8proc.o
	$(CC) $(LDFLAGS) $(LDFLAG_SHARED) -o $@ $(SOFLAG) -Wl,libutf8proc.so.$(MAJOR) utf8proc.o
	chmod a-x $@

libutf8proc.so: libutf8proc.so.$(MAJOR).$(MINOR).$(PATCH)
	ln -f -s libutf8proc.so.$(MAJOR).$(MINOR).$(PATCH) $@
	ln -f -s libutf8proc.so.$(MAJOR).$(MINOR).$(PATCH) $@.$(MAJOR)

libutf8proc.$(MAJOR).dylib: utf8proc.o
	$(CC) $(LDFLAGS) -dynamiclib -o $@ $^ -install_name $(libdir)/$@ -Wl,-compatibility_version -Wl,$(MAJOR) -Wl,-current_version -Wl,$(MAJOR).$(MINOR).$(PATCH)

libutf8proc.dylib: libutf8proc.$(MAJOR).dylib
	ln -f -s libutf8proc.$(MAJOR).dylib $@

libutf8proc.pc: libutf8proc.pc.in
	sed \
		-e 's#PREFIX#$(prefix)#' \
		-e 's#LIBDIR#$(pkglibdir)#' \
		-e 's#INCLUDEDIR#$(pkgincludedir)#' \
		-e 's#VERSION#$(MAJOR).$(MINOR).$(PATCH)#' \
		libutf8proc.pc.in > libutf8proc.pc

install: libutf8proc.a libutf8proc.$(SHLIB_EXT) libutf8proc.$(SHLIB_VERS_EXT) libutf8proc.pc
	mkdir -m 755 -p $(DESTDIR)$(includedir)
	$(INSTALL) -m 644 utf8proc.h $(DESTDIR)$(includedir)
	mkdir -m 755 -p $(DESTDIR)$(libdir)
	$(INSTALL) -m 644 libutf8proc.a $(DESTDIR)$(libdir)
	$(INSTALL) -m 755 libutf8proc.$(SHLIB_VERS_EXT) $(DESTDIR)$(libdir)
	mkdir -m 755 -p $(DESTDIR)$(pkgconfigdir)
	$(INSTALL) -m 644 libutf8proc.pc $(DESTDIR)$(pkgconfigdir)/libutf8proc.pc
	ln -f -s libutf8proc.$(SHLIB_VERS_EXT) $(DESTDIR)$(libdir)/libutf8proc.$(SHLIB_EXT)
ifneq ($(OS),Darwin)
	ln -f -s libutf8proc.$(SHLIB_VERS_EXT) $(DESTDIR)$(libdir)/libutf8proc.so.$(MAJOR)
endif

MANIFEST.new:
	rm -rf tmp
	$(MAKE) install prefix=/usr DESTDIR=$(PWD)/tmp
	$(FIND) tmp/usr -mindepth 1 -type l -printf "%P -> %l\n" -or -type f -printf "%P\n" -or -type d -printf "%P/\n" | LC_ALL=C sort > $@
	rm -rf tmp

# Test programs

data/NormalizationTest.txt:
	$(MAKE) -C data NormalizationTest.txt

data/GraphemeBreakTest.txt:
	$(MAKE) -C data GraphemeBreakTest.txt

data/Lowercase.txt:
	$(MAKE) -C data Lowercase.txt

data/Uppercase.txt:
	$(MAKE) -C data Uppercase.txt

test/tests.o: test/tests.c test/tests.h utf8proc.h
	$(CC) $(UCFLAGS) -c -o test/tests.o test/tests.c

test/normtest: test/normtest.c test/tests.o utf8proc.o utf8proc.h test/tests.h
	$(CC) $(UCFLAGS) $(LDFLAGS) test/normtest.c test/tests.o utf8proc.o -o $@

test/graphemetest: test/graphemetest.c test/tests.o utf8proc.o utf8proc.h test/tests.h
	$(CC) $(UCFLAGS) $(LDFLAGS) test/graphemetest.c test/tests.o utf8proc.o -o $@

test/printproperty: test/printproperty.c test/tests.o utf8proc.o utf8proc.h test/tests.h
	$(CC) $(UCFLAGS) $(LDFLAGS) test/printproperty.c test/tests.o utf8proc.o -o $@

test/charwidth: test/charwidth.c test/tests.o utf8proc.o utf8proc.h test/tests.h
	$(CC) $(UCFLAGS) $(LDFLAGS) test/charwidth.c test/tests.o utf8proc.o -o $@

test/valid: test/valid.c test/tests.o utf8proc.o utf8proc.h test/tests.h
	$(CC) $(UCFLAGS) $(LDFLAGS) test/valid.c test/tests.o utf8proc.o -o $@

test/iterate: test/iterate.c test/tests.o utf8proc.o utf8proc.h test/tests.h
	$(CC) $(UCFLAGS) $(LDFLAGS) test/iterate.c test/tests.o utf8proc.o -o $@

test/iscase: test/iscase.c test/tests.o utf8proc.o utf8proc.h test/tests.h
	$(CC) $(UCFLAGS) $(LDFLAGS) test/iscase.c test/tests.o utf8proc.o -o $@

test/case: test/case.c test/tests.o utf8proc.o utf8proc.h test/tests.h
	$(CC) $(UCFLAGS) $(LDFLAGS) test/case.c test/tests.o utf8proc.o -o $@

test/custom: test/custom.c test/tests.o utf8proc.o utf8proc.h test/tests.h
	$(CC) $(UCFLAGS) $(LDFLAGS) test/custom.c test/tests.o utf8proc.o -o $@

test/misc: test/misc.c test/tests.o utf8proc.o utf8proc.h test/tests.h
	$(CC) $(UCFLAGS) $(LDFLAGS) -DUNICODE_VERSION='"'`$(PERL) -ne "/^UNICODE_VERSION=/ and print $$';" data/Makefile`'"' test/misc.c test/tests.o utf8proc.o -o $@

# check: test/normtest data/NormalizationTest.txt data/Lowercase.txt data/Uppercase.txt test/graphemetest data/GraphemeBreakTest.txt test/printproperty test/case test/iscase test/custom test/charwidth test/misc test/valid test/iterate bench/bench.c bench/util.c bench/util.h utf8proc.o
check: test/normtest test/graphemetest test/printproperty test/case test/iscase test/custom test/charwidth test/misc test/valid test/iterate bench/bench.c bench/util.c bench/util.h utf8proc.o
	$(MAKE) -C bench
#	test/normtest data/NormalizationTest.txt
#	test/graphemetest data/GraphemeBreakTest.txt
	test/charwidth
	test/misc
	test/valid
	test/iterate
	test/case
#	test/iscase data/Lowercase.txt data/Uppercase.txt
	test/custom
