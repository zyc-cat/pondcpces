AC_INIT
AM_INIT_AUTOMAKE(pond,2.0)
AC_PROG_CC

AC_C_CONST

AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_YACC
AC_HEADER_STDC


AM_PROG_LEX

#AC_LIBTOOL_WIN32_DLL
AC_DISABLE_SHARED
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_SRCDIR(src)
AC_CHECK_HEADERS([stddef.h stdlib.h string.h strings.h sstream  ])
#AC_CHECK_LIB([cudd], [Cudd_Init], [], [AC_MSG_ERROR([cudd was not found])])

# Set the language.
AC_LANG(C++)
AC_CONFIG_FILES(Makefile src/Makefile src/ppddl/Makefile src/search/Makefile src/cudd/Makefile src/lug/Makefile src/heuristics/Makefile src/simulator/Makefile) 
AC_OUTPUT

AC_DEFINE(SIZEOF_LONG, 16)
AC_DEFINE(SIZEOF_VOID_P, 16)
