dnl Shamelessly stolen from Heimdal
dnl
dnl Look for function in any of the specified libraries
dnl

dnl AC_FIND_FUNC_NO_LIBS2(func, libraries, includes, arguments, extra libs, extra args)
AC_DEFUN([AC_FIND_FUNC_NO_LIBS2], [

AC_MSG_CHECKING([for $1])
AC_CACHE_VAL(ac_cv_funclib_$1,
[
if eval "test \"\$ac_cv_func_$1\" != yes" ; then
	ac_save_LIBS="$LIBS"
	for ac_lib in $2; do
		case "$ac_lib" in
		"") ;;
		yes) ac_lib="" ;;
		no) continue ;;
		-l*) ;;
		*) ac_lib="-l$ac_lib" ;;
		esac
		LIBS="$6 $ac_lib $5 $ac_save_LIBS"
		AC_LINK_IFELSE([AC_LANG_PROGRAM([[$3]],[[$1($4)]])],[eval "if test -n \"$ac_lib\";then ac_cv_funclib_$1=$ac_lib; else ac_cv_funclib_$1=yes; fi";break])
	done
	eval "ac_cv_funclib_$1=\${ac_cv_funclib_$1-no}"
	LIBS="$ac_save_LIBS"
fi
])

eval "ac_res=\$ac_cv_funclib_$1"

if false; then
	AC_CHECK_FUNCS($1)
dnl	AC_CHECK_LIBS($2, foo)
fi
# $1
eval "ac_tr_func=HAVE_[]upcase($1)"
eval "ac_tr_lib=HAVE_LIB[]upcase($ac_res | sed -e 's/-l//')"
eval "LIB_$1=$ac_res"

case "$ac_res" in
	yes)
	eval "ac_cv_func_$1=yes"
	eval "LIB_$1="
	AC_DEFINE_UNQUOTED($ac_tr_func)
	AC_MSG_RESULT([yes])
	;;
	no)
	eval "ac_cv_func_$1=no"
	eval "LIB_$1="
	AC_MSG_RESULT([no])
	;;
	*)
	eval "ac_cv_func_$1=yes"
	eval "ac_cv_lib_`echo "$ac_res" | sed 's/-l//'`=yes"
	AC_DEFINE_UNQUOTED($ac_tr_func)
	AC_DEFINE_UNQUOTED($ac_tr_lib)
	AC_MSG_RESULT([yes, in $ac_res])
	;;
esac
AC_SUBST(LIB_$1)
])
