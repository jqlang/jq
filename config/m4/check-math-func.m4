dnl AC_FIND_FUNC(func, arguments)
AC_DEFUN([AC_CHECK_MATH_FUNC], [
  AC_FIND_FUNC_NO_LIBS([$1], [m], [#include <math.h>], [$2])
])
