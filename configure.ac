#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(get-manip, 1.1, bradt@nscl.msu.edu)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([get-manip/])

ACLOCAL_AMFLAGS = -I ./boost.m4

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.
BOOST_REQUIRE

BOOST_DATE_TIME
BOOST_LOG
BOOST_SYSTEM
BOOST_PROGRAM_OPTIONS
BOOST_FILESYSTEM


# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT64_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([floor])

AC_CONFIG_FILES([Makefile
                 get-manip/Makefile])
AC_OUTPUT
