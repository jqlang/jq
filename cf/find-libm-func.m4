dnl
dnl Shamelessly stolen from Heimdal
dnl
dnl     http://h5l.org/
dnl     https://github.com/heimdal/heimdal/
dnl
dnl Look for function in any of the specified libraries
dnl

dnl AC_FIND_LIBM_FUNC(func, arguments)
AC_DEFUN([AC_FIND_LIBM_FUNC], [
AC_FIND_FUNC_NO_LIBS([$1], [m], [
#include <math.h>
], [$2], [], [])])
