m4_define([jq_version],
          m4_esyscmd_s([(git rev-parse --verify -q jq-1.0 > /dev/null &&
                        (git describe --tags --dirty --match 'jq-*'|sed 's/^jq-//')) ||
                        echo `git rev-parse --abbrev-ref HEAD`-`git describe --always --dirty`])))

AC_INIT([jq], [jq_version], [https://github.com/stedolan/jq/issues],
             [jq], [https://stedolan.github.io/jq])

m4_include([m4/ax_compare_version.m4])
m4_include([m4/ax_prog_bison_version.m4])

dnl Created autoconf implementation thompson@dtosolutions, 26NOV12
AC_PREREQ([2.64])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.11.2 subdir-objects parallel-tests foreign -Wall])
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

  AC_CHECK_PROGS(LEX, flex lex)
fi

dnl Check for valgrind
AC_CHECK_PROGS(valgrind_cmd, valgrind)
if test "x$valgrind_cmd" = "x" ; then
    AC_MSG_WARN([valgrind is required to test jq.])
fi
AC_CHECK_FUNCS(memmem)
AC_CHECK_FUNCS(mkstemp)

AC_CHECK_HEADER("shlwapi.h",[have_win32=1;])
AM_CONDITIONAL([WIN32], [test "x$have_win32" = x1])

dnl Running tests with Valgrind is slow. It is faster to iterate on
dnl code without Valgrind until tests pass, then enable Valgrind and
dnl fix leaks.
AC_ARG_ENABLE([valgrind],
   AC_HELP_STRING([--disable-valgrind], [do not run tests under Valgrind]))

dnl Running tests with Valgrind is slow; address sanitizer (ASAN) is
dnl faster.
AC_ARG_ENABLE([asan],
   AC_HELP_STRING([--enable-asan], [enable address sanitizer]))

dnl Undefined Behavior Sanitizer
AC_ARG_ENABLE([ubsan],
   AC_HELP_STRING([--enable-ubsan], [enable undefined behavior sanitizer]))

dnl Code coverage
AC_ARG_ENABLE([gcov],
   AC_HELP_STRING([--enable-gcov], [enable gcov code coverage tool]))

dnl Don't attempt to build docs if python deps aren't installed
AC_ARG_ENABLE([docs],
   AC_HELP_STRING([--disable-docs], [don't build docs]))

dnl Don't attempt to build the error injection object (if there is no LD_PRELOAD support)
AC_ARG_ENABLE([error-injection],
   AC_HELP_STRING([--enable-error-injection], [build and test with error injection]))

dnl Enable building all static
AC_ARG_ENABLE([all-static],
   AC_HELP_STRING([--enable-all-static], [link jq with static libraries only]))

AS_IF([test "x$enable_docs" = "xyes"],[
   AC_CHECK_PROGS(pipenv_cmd, pipenv)

   AC_CACHE_CHECK([for Python dependencies], [jq_cv_python_deps],
     [jq_cv_python_deps=yes;
        AS_IF([test "x$pipenv_cmd" = "x" || \
        ! bmsg="`cd ${srcdir}/docs; LC_ALL=$LANG "$pipenv_cmd" check`"],[
           AC_MSG_ERROR([$bmsg])
           cat <<EOF
*****************************************************************
*  Python dependencies for building jq documentation not found. *
*  You can still build, install and hack on jq, but the manpage *
*  will not be rebuilt and some of the tests will not run.      *
*  See docs/README.md for how to install the docs dependencies. *
*****************************************************************
EOF
           jq_cv_python_deps=no
     ])])

   if test "x$jq_cv_python_deps" != "xyes"; then
     enable_docs=no
   fi
])

AM_CONDITIONAL([ENABLE_VALGRIND], [test "x$enable_valgrind" != xno])
AM_CONDITIONAL([ENABLE_ASAN], [test "x$enable_asan" = xyes])
AM_CONDITIONAL([ENABLE_UBSAN], [test "x$enable_ubsan" = xyes])
AM_CONDITIONAL([ENABLE_GCOV], [test "x$enable_gcov" = xyes])
AM_CONDITIONAL([ENABLE_DOCS], [test "x$enable_docs" != xno])
AM_CONDITIONAL([ENABLE_ERROR_INJECTION], [test "x$enable_error_injection" = xyes])
AM_CONDITIONAL([ENABLE_ALL_STATIC], [test "x$enable_all_static" = xyes])

AC_FUNC_ALLOCA

AC_FIND_FUNC([isatty], [c], [#include <unistd.h>], [0])
AC_FIND_FUNC([_isatty], [c], [#include <io.h>], [0])
AC_FIND_FUNC([strptime], [c], [#include <time.h>], [0, 0, 0])
AC_FIND_FUNC([strftime], [c], [#include <time.h>], [0, 0, 0, 0])
AC_FIND_FUNC([setenv], [c], [#include <stdlib.h>], [0, 0, 0])
AC_FIND_FUNC([timegm], [c], [#include <time.h>], [0])
AC_FIND_FUNC([gmtime_r], [c], [#include <time.h>], [0, 0])
AC_FIND_FUNC([gmtime], [c], [#include <time.h>], [0])
AC_FIND_FUNC([localtime_r], [c], [#include <time.h>], [0, 0])
AC_FIND_FUNC([localtime], [c], [#include <time.h>], [0])
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
AC_CHECK_MATH_FUNC(acos)
AC_CHECK_MATH_FUNC(acosh)
AC_CHECK_MATH_FUNC(asin)
AC_CHECK_MATH_FUNC(asinh)
AC_CHECK_MATH_FUNC(atan2)
AC_CHECK_MATH_FUNC(atan)
AC_CHECK_MATH_FUNC(atanh)
AC_CHECK_MATH_FUNC(cbrt)
AC_CHECK_MATH_FUNC(ceil)
AC_CHECK_MATH_FUNC(copysign)
AC_CHECK_MATH_FUNC(cos)
AC_CHECK_MATH_FUNC(cosh)
AC_CHECK_MATH_FUNC(drem)
AC_CHECK_MATH_FUNC(erf)
AC_CHECK_MATH_FUNC(erfc)
AC_CHECK_MATH_FUNC(exp10)
AC_CHECK_MATH_FUNC(exp2)
AC_CHECK_MATH_FUNC(exp)
AC_CHECK_MATH_FUNC(expm1)
AC_CHECK_MATH_FUNC(fabs)
AC_CHECK_MATH_FUNC(fdim)
AC_CHECK_MATH_FUNC(floor)
AC_CHECK_MATH_FUNC(fma)
AC_CHECK_MATH_FUNC(fmax)
AC_CHECK_MATH_FUNC(fmin)
AC_CHECK_MATH_FUNC(fmod)
AC_CHECK_MATH_FUNC(frexp)
AC_CHECK_MATH_FUNC(gamma)
AC_CHECK_MATH_FUNC(hypot)
AC_CHECK_MATH_FUNC(j0)
AC_CHECK_MATH_FUNC(j1)
AC_CHECK_MATH_FUNC(jn)
AC_CHECK_MATH_FUNC(ldexp)
AC_CHECK_MATH_FUNC(lgamma)
AC_CHECK_MATH_FUNC(log10)
AC_CHECK_MATH_FUNC(log1p)
AC_CHECK_MATH_FUNC(log2)
AC_CHECK_MATH_FUNC(log)
AC_CHECK_MATH_FUNC(logb)
AC_CHECK_MATH_FUNC(modf)
AC_CHECK_MATH_FUNC(lgamma_r)
AC_CHECK_MATH_FUNC(nearbyint)
AC_CHECK_MATH_FUNC(nextafter)
AC_CHECK_MATH_FUNC(nexttoward)
AC_CHECK_MATH_FUNC(pow10) # Not available with glibc version >= 2.27
AC_CHECK_MATH_FUNC(pow)
AC_CHECK_MATH_FUNC(remainder)
AC_CHECK_MATH_FUNC(rint)
AC_CHECK_MATH_FUNC(round)
AC_CHECK_MATH_FUNC(scalb)
AC_CHECK_MATH_FUNC(scalbln)
AC_CHECK_MATH_FUNC(significand)
AC_CHECK_MATH_FUNC(sin)
AC_CHECK_MATH_FUNC(sinh)
AC_CHECK_MATH_FUNC(sqrt)
AC_CHECK_MATH_FUNC(tan)
AC_CHECK_MATH_FUNC(tanh)
AC_CHECK_MATH_FUNC(tgamma)
AC_CHECK_MATH_FUNC(trunc)
AC_CHECK_MATH_FUNC(y0)
AC_CHECK_MATH_FUNC(y1)
AC_CHECK_MATH_FUNC(yn)

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

dnl Oniguruma
AC_ARG_WITH([oniguruma],
   [AS_HELP_STRING([--with-oniguruma=prefix],
      [try this for a non-standard install prefix of the oniguruma library])], ,
   [with_oniguruma=yes])

onig_CFLAGS=
onig_LDFLAGS=
build_oniguruma=no
AS_IF([test "x$with_oniguruma" != xno], [
   save_CFLAGS="$CFLAGS"
   save_LDFLAGS="$LDFLAGS"
   AS_IF([test "x$with_oniguruma" != xyes], [
      AS_IF([test "x$with_oniguruma" = xbuiltin], [
          build_oniguruma=yes
      ], [
          onig_CFLAGS="-I${with_oniguruma}/include"
          onig_LDFLAGS="-L${with_oniguruma}/lib"
      ])
   ])
   AS_IF([test "x$build_oniguruma" = xno], [
       # check for ONIGURUMA library, either in /usr or where requested
       CFLAGS="$CFLAGS $onig_CFLAGS"
       LDFLAGS="$LDFLAGS $onig_LDFLAGS"
       AC_CHECK_HEADER("oniguruma.h",
          AC_CHECK_LIB([onig],[onig_version]))
       # handle check results
       AS_IF([test "x$ac_cv_lib_onig_onig_version" != "xyes"], [
          build_oniguruma=yes
          AC_MSG_NOTICE([Oniguruma was not found. Will use the packaged oniguruma.])
       ])
   ])
   AS_IF([test "x$build_oniguruma" = xyes -a -f "${srcdir}/modules/oniguruma/configure.ac" ], [
       onig_CFLAGS="-I${srcdir}/modules/oniguruma/src"
       onig_LDFLAGS="-L${srcdir}/modules/oniguruma/src -Wl,-rpath,${libdir}"
       AC_CONFIG_SUBDIRS([modules/oniguruma])
       AC_DEFINE([HAVE_LIBONIG],1,[Define to 1 if the system includes libonig])
   ])
   CFLAGS="$save_CFLAGS"
   LDFLAGS="$save_LDFLAGS"
])
AC_SUBST(onig_CFLAGS)
AC_SUBST(onig_LDFLAGS)

AM_CONDITIONAL([BUILD_ONIGURUMA], [test "x$build_oniguruma" = xyes])
AC_SUBST([BUNDLER], ["$bundle_cmd"])

AC_CONFIG_MACRO_DIR([config/m4])
AC_CONFIG_FILES([Makefile libjq.pc])
AC_OUTPUT

