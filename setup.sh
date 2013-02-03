#!/usr/bin/env bash

# RUN ME after pulling the code from git!
#
# This script is handy after pulling the code out of git without having
# to remember all the typing needed to build up and tear down all the
# crap that autotools puts in your source repo.  The intent is to keep
# all generated files out of git.

if [ "clean" == "$1" ]; then
  make distclean || echo "continuing..."
  rm -rf tmp autom4te.cache config
  rm -f INSTALL Makefile.in aclocal.m4 configure config.h.in ChangeLog
  rm -f jv_utf8_tables.gen.h lexer.c lexer.h parser.c parser.h
elif [ "superclean" == "$1" ]; then
  # if autoconf errors during distcheck, it leaves files that need chmod'ing
  ver=$(tr -d '\n' <VERSION)
  if [ "x${ver}" != "x" ]; then
    if [ -d jq-${ver} ]; then
      chmod -R u+w jq-${ver}
      rm -rf jq-${ver}
    fi
    rm -f jq-${ver}*
  fi
else
  autoreconf --install
  ./configure --enable-devel --prefix=/opt/junk
  make check
  mkdir tmp
  make DESTDIR=./tmp install
  make distcheck
fi
