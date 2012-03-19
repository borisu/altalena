#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([ivrworx], [2.0], [borisusun@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([iw_core/UIDOwner.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit gethostbyname gethostname inet_ntoa memset socket strchr strdup])

AC_OUTPUT(Makefile)
