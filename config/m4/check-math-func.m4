dnl AC_CHECK_MATH_FUNC(func)
AC_DEFUN([AC_CHECK_MATH_FUNC], [
  AC_LANG(C)
  AC_MSG_CHECKING(for $1)
  AC_LINK_IFELSE([AC_LANG_SOURCE([
    #include <math.h>

    int main() {
      void *x = $1;
      return !x;
    }
  ])], [
    AC_MSG_RESULT(yes)
    eval "ac_tr_func=HAVE_[]upcase($1)"
    AC_DEFINE_UNQUOTED($ac_tr_func)
  ],[
    AC_MSG_RESULT(no)
  ])
])
