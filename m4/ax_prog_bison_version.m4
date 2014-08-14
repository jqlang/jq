# ===========================================================================
#   Modified from
#   http://www.gnu.org/software/autoconf-archive/ax_prog_perl_version.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_PROG_BISON_VERSION([VERSION],[ACTION-IF-TRUE],[ACTION-IF-FALSE])
#
# DESCRIPTION
#
#   Makes sure that bison supports the version indicated. If true the shell
#   commands in ACTION-IF-TRUE are executed. If not the shell commands in
#   ACTION-IF-FALSE are run. Note if $PERL is not set the macro will fail.
#
#   Example:
#
#     AC_PROG_YACC
#     AX_PROG_BISON_VERSION([3.0.0],[ ... ],[ ... ])
#
#   This will check to make sure that the bison you have supports at least
#   version 3.0.0.
#
#   NOTE: This macro uses the $YACC variable to perform the check.
#   AX_WITH_YACC can be used to set that variable prior to running this
#   macro. The $BISON_VERSION variable will be set with the detected
#   version.
#
# LICENSE
#
#   Copyright (c) 2009 Francesco Salvestrini <salvestrini@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 12

AC_DEFUN([AX_PROG_BISON_VERSION],[
    AC_REQUIRE([AC_PROG_SED])
    AC_REQUIRE([AC_PROG_GREP])

    AS_IF([test -n "$YACC"],[
        ax_bison_version="$1"

        AC_MSG_CHECKING([for bison version])
        changequote(<<,>>)
        bison_version=`$YACC --version 2>&1 \
          | $GREP bison \
          | $SED -e 's/^[^0-9]*//' -e 's/[^0-9]*$//'`
        changequote([,])
        AC_MSG_RESULT($bison_version)

	AC_SUBST([BISON_VERSION],[$bison_version])

        AX_COMPARE_VERSION([$ax_bison_version],[le],[$bison_version],[
	    :
            $2
        ],[
	    :
            $3
        ])
    ],[
        AC_MSG_WARN([could not find bison])
        $3
    ])
])
