#!/usr/bin/env bash

# RUN ME after pulling the code from git!
if [ "clean" == "$1" ]; then
  make distclean || echo "continuing..."
  rm -rf tmp autom4te.cache config
  rm -f INSTALL Makefile.in aclocal.m4 configure config.h.in ChangeLog
  rm -f jv_utf8_tables.gen.h lexer.c lexer.h parser.c parser.h
elif [ "superclean" == "$1" ]; then
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
  ./configure --prefix=/opt/junk
  mkdir tmp
  make DESTDIR=./tmp install
  make distcheck
fi
