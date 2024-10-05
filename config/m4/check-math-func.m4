dnl AC_CHECK_MATH_FUNC(func)
AC_DEFUN([AC_CHECK_MATH_FUNC], [
  AC_LANG(C)
  AC_CHECK_LIB([m],[$1],[
    m4_define([ac_tr_func], [HAVE_]m4_toupper($1))
    AC_DEFINE(ac_tr_func, 1, [Define if you have the $1 math function.])
  ],[
  ])
])
