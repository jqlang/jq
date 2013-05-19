AC_INIT([jq], [1.3], [mu@netsoc.tcd.ie],
             [jq], [http://stedolan.github.com/jq/])

dnl Created autoconf implementation thompson@dtosolutions, 26NOV12
AC_PREREQ([2.61])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([parallel-tests foreign])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_CPP_WERROR
AC_PROG_YACC

AM_PROG_CC_C_O

dnl couldn't use AM_PROG_LEX as it doesn't support header files like the
dnl AC_PROG_YACC macros...

dnl
dnl these program checks should probably be deleted
dnl

AC_PROG_LEX
if test "x$LEX" != xflex; then
  LEX="$SHELL $missing_dir/missing flex"
fi


dnl Check for valgrind
AC_CHECK_PROGS(valgrind_cmd, valgrind)
if test "x$valgrind_cmd" = "x" ; then
    AC_MSG_WARN([valgrind is required to test jq.])
fi


dnl Don't attempt to build docs if there's no Ruby lying around
AC_ARG_ENABLE([docs],
   AC_HELP_STRING([--disable-docs], [don't build docs]))

AS_IF([test "x$enable_docs" != "xno"],[
   AC_CHECK_PROGS(bundle_cmd, bundle)
   AS_IF([test "x$bundle_cmd" = "x" || \
      ! ( cd ${srcdir}/docs; "$bundle_cmd" check 2>/dev/null )],[
         AC_MSG_WARN([no bundler])
         cat <<EOF
*****************************************************************
*  Ruby dependencies for building jq documentation not found.   *
*  You can still build, install and hack on jq, but the manpage *
*  will not be rebuilt and some of the tests won't run.         *
*  See docs/README.md for how to install the docs dependencies. *
*****************************************************************
EOF
         enable_docs=no
   ])
])

AM_CONDITIONAL([ENABLE_DOCS], [test "x$enable_docs" != xno])
AC_SUBST([BUNDLER], ["$bundle_cmd"])

dnl AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE([-Wall])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT


