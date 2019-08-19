#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([wabi],[0.0.1],[frbracch@gmail.com])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_HEADERS([config.h])

## LIBUTOOL stuff
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_PREREQ([2.4.6])
LT_INIT([disable-shared])

# Checks for programs.
AC_PROG_CC

# Checks for header files.

AC_CHECK_HEADERS([stdint.h stdlib.h ev.h])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 test/Makefile])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC

# Check for EV
AC_CHECK_LIB([ev], [ev_run], [LIBEV=-lev], [AC_MSG_ERROR(Unable to find libev library)])
AC_SUBST([LIBEV])

PKG_CHECK_MODULES([CHECK], [check >= 0.9.10])

AC_SYS_LARGEFILE

AC_OUTPUT
