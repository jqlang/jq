dnl AC_CHECK_MATH_FUNC(func)
AC_DEFUN([AC_CHECK_MATH_FUNC], [
  AC_LANG(C)
  AC_CHECK_LIB([m],[$1],[
    eval "ac_tr_func=HAVE_[]upcase($1)"
    AC_DEFINE_UNQUOTED($ac_tr_func)
  ],[
  ])
])
