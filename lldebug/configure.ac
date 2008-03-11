dnl 
dnl $Id: configure.ac,v 1.4 2008-03-11 11:23:48 cielacanth Exp $
dnl Process this file with autoconf to produce a configure script.
dnl

AC_INIT([lldebug], [0.1.0])
AC_CONFIG_AUX_DIR([build/build-scripts])
AC_CONFIG_SRCDIR([src/lldebug.cpp])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AC_PATH_PROG(WXCONFIG, wx-config, :, /usr/local/bin:/usr/bin:/bin)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdint.h stdtypes.h])
AC_CHECK_HEADERS([stddef.h stdio.h stdlib.h])
AC_CHECK_HEADERS([malloc.h string.h memory.h assert.h])

# Checks for functions.
AC_FUNC_MALLOC
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([memcpy memmove memset memcmp])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

AC_ARG_ENABLE(debug,
	[ --enable-debug    Turn on Debugging],
	[case "${enableval}" in
		yes) debug=true ;;
		no)  debug=false ;;
		*)   AC_MSG_ERROR(bad value ${enableval} for --enable debug) ;;
	esac],
	[debug=false])

if [ test x$debug = xtrue ]; then
	wxoption="--unicode=yes --debug=yes"
else
	wxoption="--unicode=yes --debug=yes"
fi

WXCPPFLAGS=`$WXCONFIG $wxoption --cppflags`
WXCXXFLAGS=`$WXCONFIG $wxoption --cxxflags`
WXCFLAGS=`$WXCONFIG $wxoption --cflags`
WXLIBS=`$WXCONFIG $wxoption --libs`
AC_SUBST(WXCPPFLAGS)
AC_SUBST(WXCXXFLAGS)
AC_SUBST(WXCFLAGS)
AC_SUBST(WXLIBS)

AC_CONFIG_FILES([
	Makefile
	build/Makefile
	build/treelistctrl/Makefile
	build/wxscintilla/Makefile
	build/boost_system/Makefile
	build/lldebug/Makefile
	build/lldebug_frame/Makefile
	build/echo_server/Makefile
	build/echo_client/Makefile
	build/test/Makefile
	])

AC_OUTPUT

