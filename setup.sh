#!/usr/bin/env bash

# RUN ME after pulling the code from git!
#
# This script is handy after pulling the code out of git without having
# to remember all the typing needed to build up and tear down all the
# crap that autotools puts in your source repo.  The intent is to keep
# all generated files out of git.

if [ "clean" == "$1" ]; then
  make distclean || echo "continuing..."
  find config/m4 -type f \( -name libtool\* -o -name lt\* \) | xargs rm -f
  find config -name m4 -prune -o -type f -print | xargs rm -f
  rm -rf tmp autom4te.cache
  rm -f INSTALL Makefile.in aclocal.m4 configure config.h.in ChangeLog
  rm -f jv_utf8_tables.gen.h lexer.c lexer.h parser.c parser.h
elif [ "superclean" == "$1" ]; then
  # if autoconf errors during distcheck, it leaves files that need chmod'ing
  ver=$(scripts/version|tr -d '\n')
  if [ "x${ver}" != "x" ]; then
    if [ -d jq-${ver} ]; then
      chmod -R u+w jq-${ver}
      rm -rf jq-${ver}
    fi
    rm -f jq-${ver}*
  fi
else
  autoreconf --install
  ./configure --prefix=/opt/junk
  make check
  [ -d tmp ] && mv tmp tmp-
  mkdir tmp
  make DESTDIR=./tmp install
  make distcheck
fi
