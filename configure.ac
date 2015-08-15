m4_define([jq_version], m4_esyscmd_s([git describe --tags --dirty --match 'jq-*'|sed 's/^jq-//']))

AC_INIT([jq], [jq_version], [https://github.com/stedolan/jq/issues],
             [jq], [https://stedolan.github.io/jq])

m4_include([m4/ax_compare_version.m4])
m4_include([m4/ax_prog_bison_version.m4])

dnl Created autoconf implementation thompson@dtosolutions, 26NOV12
AC_PREREQ([2.64])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.11.2 parallel-tests foreign -Wall])
AM_SILENT_RULES([yes])
AM_PROG_AR
AM_MAINTAINER_MODE([enable])
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_CPP_WERROR
AC_PROG_YACC
AC_OBJEXT
AC_EXEEXT
LT_INIT([shared static win32-dll])
AM_PROG_CC_C_O


dnl couldn't use AM_PROG_LEX as it doesn't support header files like the
dnl AC_PROG_YACC macros...

dnl check bison version

if test "$USE_MAINTAINER_MODE" = yes; then
  if test "$YACC" != "bison -y"; then
    AC_MSG_CHECKING([bison version])
    AC_MSG_RESULT([not bison])
  else
    AX_PROG_BISON_VERSION([3],
      [],
      [AC_MSG_ERROR([You need bison version 3.0 or greater, or use --disable-maintainer-mode.])])
  fi

  AC_PROG_LEX
  if test "x$LEX" != xflex; then
    LEX="$SHELL $missing_dir/missing flex"
  fi
fi


##########################################################################
# check for ONIGURUMA library
##########################################################################

AC_ARG_WITH([oniguruma],
    [AS_HELP_STRING([--with-oniguruma=prefix],
        [try this for a non-standard install prefix of the oniguruma library])],
    [ONIGURUMAPATHSET=1],
    [ONIGURUMAPATHSET=0])

if test $ONIGURUMAPATHSET = 1; then
  CFLAGS="$CFLAGS -I${with_oniguruma}/include"
  LDFLAGS="$LDFLAGS -L${with_oniguruma}/lib"
fi

# check for ONIGURUMA library
HAVE_ONIGURUMA=0
AC_CHECK_HEADER("oniguruma.h",
    AC_CHECK_LIB([onig],[onig_version],[LIBS="$LIBS -lonig"; HAVE_ONIGURUMA=1;]))

# handle check results
if test $HAVE_ONIGURUMA != 1; then
    AC_MSG_NOTICE([Oniguruma was not found.])
    AC_MSG_NOTICE([ Try setting the location using '--with-oniguruma=PREFIX' ])
else
    AC_DEFINE([HAVE_ONIGURUMA],1,[Define to 1 if Oniguruma is installed])
fi


dnl Check for valgrind
AC_CHECK_PROGS(valgrind_cmd, valgrind)
if test "x$valgrind_cmd" = "x" ; then
    AC_MSG_WARN([valgrind is required to test jq.])
fi
AC_CHECK_FUNCS(memmem)
AC_CHECK_FUNCS(mkstemp)


dnl Running tests with Valgrind is slow. It is faster to iterate on
dnl code without Valgrind until tests pass, then enable Valgrind and
dnl fix leaks.
AC_ARG_ENABLE([valgrind],
   AC_HELP_STRING([--disable-valgrind], [do not run tests under Valgrind]))

dnl Code coverage
AC_ARG_ENABLE([gcov],
   AC_HELP_STRING([--enable-gcov], [enable gcov code coverage tool]))

dnl Don't attempt to build docs if there's no Ruby lying around
AC_ARG_ENABLE([docs],
   AC_HELP_STRING([--disable-docs], [don't build docs]))

dnl Don't attempt to build the error injection object (if there is no LD_PRELOAD support)
AC_ARG_ENABLE([error-injection],
   AC_HELP_STRING([--enable-error-injection], [build and test with error injection]))

dnl Enable building all static
AC_ARG_ENABLE([all-static],
   AC_HELP_STRING([--enable-all-static], [link jq with static libraries only]))

AS_IF([test "x$enable_docs" != "xno"],[
   AC_CHECK_PROGS(bundle_cmd, bundle)

   AC_CACHE_CHECK([for Ruby dependencies], [jq_cv_ruby_deps],
     [jq_cv_ruby_deps=yes;
        AS_IF([test "x$bundle_cmd" = "x" || \
        ! bmsg="`cd ${srcdir}/docs; "$bundle_cmd" check 2>/dev/null`"],[
           AC_MSG_WARN([$bmsg])
           cat <<EOF
*****************************************************************
*  Ruby dependencies for building jq documentation not found.   *
*  You can still build, install and hack on jq, but the manpage *
*  will not be rebuilt and some of the tests won't run.         *
*  See docs/README.md for how to install the docs dependencies. *
*****************************************************************
EOF
           jq_cv_ruby_deps=no
     ])])

   if test "x$jq_cv_ruby_deps" != "xyes"; then
     enable_docs=no
   fi
])

AM_CONDITIONAL([ENABLE_VALGRIND], [test "x$enable_valgrind" != xno])
AM_CONDITIONAL([ENABLE_GCOV], [test "x$enable_gcov" = xyes])
AM_CONDITIONAL([ENABLE_DOCS], [test "x$enable_docs" != xno])
AM_CONDITIONAL([ENABLE_ERROR_INJECTION], [test "x$enable_error_injection" = xyes])
AM_CONDITIONAL([ENABLE_ALL_STATIC], [test "x$enable_all_static" = xyes])

AC_FUNC_ALLOCA

AC_FIND_FUNC([isatty], [c], [#include <unistd.h>], [0])
AC_FIND_FUNC([_isatty], [c], [#include <io.h>], [0])
AC_FIND_FUNC([strptime], [c], [#include <time.h>], [0, 0, 0])
AC_FIND_FUNC([strftime], [c], [#include <time.h>], [0, 0, 0, 0])
AC_FIND_FUNC([timegm], [c], [#include <time.h>], [0])
AC_FIND_FUNC([gmtime_r], [c], [#include <time.h>], [0, 0])
AC_FIND_FUNC([gmtime], [c], [#include <time.h>], [0])
AC_FIND_FUNC([gettimeofday], [c], [#include <time.h>], [0, 0])
AC_CHECK_MEMBER([struct tm.tm_gmtoff], [AC_DEFINE([HAVE_TM_TM_GMT_OFF],1,[Define to 1 if the system has the tm_gmt_off field in struct tm])],
                [], [[#include <time.h>]])
AC_CHECK_MEMBER([struct tm.__tm_gmtoff], [AC_DEFINE([HAVE_TM___TM_GMT_OFF],1,[Define to 1 if the system has the __tm_gmt_off field in struct tm])],
                [], [[#include <time.h>]])

AC_ARG_ENABLE([pthread-tls],
              [AC_HELP_STRING([--enable-pthread-tls],
                              [Enable use of pthread thread local storage])],
              [],
              [enable_pthread_tls=no])

if test $enable_pthread_tls = yes; then
    AC_FIND_FUNC([pthread_key_create], [pthread], [#include <pthread.h>], [NULL, NULL])
    AC_FIND_FUNC([pthread_once], [pthread], [#include <pthread.h>], [NULL, NULL])
    AC_FIND_FUNC([atexit], [pthread], [#include <stdlib.h>], [NULL])
fi

dnl libm math.h functions
AC_CHECK_MATH_FUNC(acos, [.5])
AC_CHECK_MATH_FUNC(acosh, [.5])
AC_CHECK_MATH_FUNC(asin, [.5])
AC_CHECK_MATH_FUNC(asinh, [.5])
AC_CHECK_MATH_FUNC(atan, [.5])
AC_CHECK_MATH_FUNC(atanh, [.5])
AC_CHECK_MATH_FUNC(cbrt, [.5])
AC_CHECK_MATH_FUNC(cos, [.5])
AC_CHECK_MATH_FUNC(cosh, [.5])
AC_CHECK_MATH_FUNC(exp2, [.5])
AC_CHECK_MATH_FUNC(exp, [.5])
AC_CHECK_MATH_FUNC(floor, [.5])
AC_CHECK_MATH_FUNC(j0, [.5])
AC_CHECK_MATH_FUNC(j1, [.5])
AC_CHECK_MATH_FUNC(log10, [.5])
AC_CHECK_MATH_FUNC(log2, [.5])
AC_CHECK_MATH_FUNC(log, [.5])
AC_CHECK_MATH_FUNC(sin, [.5])
AC_CHECK_MATH_FUNC(sinh, [.5])
AC_CHECK_MATH_FUNC(sqrt, [.5])
AC_CHECK_MATH_FUNC(tan, [.5])
AC_CHECK_MATH_FUNC(tanh, [.5])
AC_CHECK_MATH_FUNC(tgamma, [.5])
AC_CHECK_MATH_FUNC(y0, [.5])
AC_CHECK_MATH_FUNC(y1, [.5])
AC_CHECK_MATH_FUNC(pow, [2,2])
AC_CHECK_MATH_FUNC(atan2, [.5,.5])
AC_CHECK_MATH_FUNC(hypot, [.5,.5])
AC_CHECK_MATH_FUNC(remainder, [3,2])

dnl Thread local storage
have___thread=no
AC_MSG_CHECKING(for thread-local storage)
AC_LINK_IFELSE([AC_LANG_SOURCE([
static __thread int x ;
int main () { x = 123; return x; }
])], have___thread=yes)
if test $have___thread = yes; then 
   AC_DEFINE([HAVE___THREAD],1,[Define to 1 if the system supports __thread])
fi
AC_MSG_RESULT($have___thread)

AC_C_BIGENDIAN(
   AC_DEFINE([IEEE_MC68k], 1, [machine is bigendian]),
   AC_DEFINE([IEEE_8087], 1, [machine is littleendian]),
   AC_MSG_ERROR(unknown endianess),
   AC_MSG_ERROR(universial endianess not supported)
)

AC_SUBST([BUNDLER], ["$bundle_cmd"])

AC_CONFIG_MACRO_DIR([config/m4])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT


