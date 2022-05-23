# Process this file with autoconf to produce a configure script.
AC_INIT([mist],[1.1])
AC_CONFIG_SRCDIR([src/mist.c])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h stdarg.h limits.h string.h sys/types.h getopt.h malloc.h stddef.h stdlib.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gettimeofday, strerror])

# Compiler flag
CFLAGS="-O3 -Wall"
#CFLAGS="-g -D VERBOSE"
#YFLAGS="$YFLAGS -d"

#AC_CONFIG_FILES()
AC_SUBST(CFLAGS)
AC_SUBST(YFLAGS)
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
