AC_INIT([jq], m4_esyscmd([tr -d '\n' <VERSION]), [mu@netsoc.tcd.ie],
             [jq], [http://stedolan.github.com/jq/])

dnl Created autoconf implementation thompson@dtosolutions, 26NOV12
AC_PREREQ([2.61])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([parallel-tests])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_CPP_WERROR
AC_PROG_YACC

AM_PROG_CC_C_O

dnl couldn't use AM_PROG_LEX as it doesn't support header files like the
dnl AC_PROG_YACC macros...

dnl   CFLAGS is concidered a packagers variable, not a developer variable
dnl   in Auto-tools.  For developer ease of use, CFLAGS are set how the
dnl   developers like them by default but the packager can disable if so
dnl   desired.  This is useful if something other than gcc is needed or
dnl   for distros that need to heavily tweek the compiler (gentoo, ARM cpu
dnl   linux's, etc)
AC_ARG_ENABLE(
  [devel],
  [AC_HELP_STRING(
    [--enable-devel],
    [Build the tests and enable the same CFLAGS used by jq developers @<:@default=no@:>@])],
  [],
  [enable_devel=no]
)
AC_MSG_RESULT($enable_devel)
AM_CONDITIONAL([DEVELOPMENT_ENABLED], [test x$enable_devel = xyes])

dnl
dnl these program checks should probably be deleted
dnl

dnl Check for sed
AC_CHECK_PROGS(regex_cmd, sed)
if test x$regex_cmd = "x" ; then
    AC_MSG_NOTICE([sed is required to patch jq.])
fi

dnl Check for flex
AC_CHECK_PROGS(flex_cmd, flex)
if test x$flex_cmd = "x" ; then
    AC_MSG_NOTICE([flex is required to patch jq.])
fi

dnl Check for bison
AC_CHECK_PROGS(bison_cmd, bison)
if test x$bison_cmd = "x" ; then
    AC_MSG_NOTICE([bison is required to patch jq.])
fi

dnl Check for python
AC_CHECK_PROGS(python_cmd, python)
if test x$python_cmd = "x" ; then
    AC_MSG_NOTICE([python is required to patch jq.])
fi

dnl Check for valgrind
AC_CHECK_PROGS(valgrind_cmd, valgrind)
if test x$valgrind_cmd = "x" ; then
    AC_MSG_NOTICE([valgrind is required to test jq.])
fi

dnl AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE([-Wall dist-bzip2 dist-zip])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
