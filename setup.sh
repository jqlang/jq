#!/usr/bin/env bash

# RUN ME after pulling the code from git!
if [ "clean" == "$1" ]; then
  make distclean
  rm -rf tmp autom4te.cache config
  rm -f INSTALL Makefile.in aclocal.m4 configure config.h.in *.gen.c *.gen.h ChangeLog
else
  autoreconf --install
  ./configure --prefix=/opt/junk
  mkdir tmp
  make DESTDIR=./tmp install
  make distcheck
fi
