#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.72])
AC_INIT([wcjson], [0.1], [cs@schulte.it])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects 1.16])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/wcjson.h])
AC_CONFIG_TESTDIR([tests])
AC_CONFIG_HEADERS([src/config.h])
AC_LANG([C])

# Automake macros first
AM_PROG_AR
AM_MISSING_PROG([AUTOM4TE], [autom4te])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AR
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

LT_INIT

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdint.h
                  unistd.h
                  wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_CHECK_SIZEOF([wchar_t])

# Checks for library functions.
AC_CHECK_FUNCS([setlocale])
AC_FUNC_MALLOC
AC_FUNC_REALLOC

# Defaults

AC_ARG_WITH([default-memory-limit],
  [AS_HELP_STRING([--with-default-memory-limit],
    [Default value in bytes for the -m command line optionn (default 16k)])],
  [case $with_default_memory_limit in
    yes) AC_MSG_ERROR([[--with-default-memory-limit needs an argument]])
    ;;
    no) AC_MSG_ERROR([[--without-default-memory-limit not supported]])
    ;;
    [[0-9]]*)
    ;;
    *) AC_MSG_ERROR([[default memory limit must be numeric bytes]])
    ;;
   esac],
  [with_default_memory_limit=16384])

AC_DEFINE_UNQUOTED(CLI_DEFAULT_LIMIT, [$with_default_memory_limit],
  [Default memory limit value.])

AC_ARG_WITH([nesting-limit],
  [AS_HELP_STRING([--with-nesting-limit],
    [Nesting limit used to prevent stack overflows (default 256)])],
  [case $with_nesting_limit in
    yes) AC_MSG_ERROR([[--with-nesting-limit needs an argument]])
    ;;
    no) AC_MSG_ERROR([[--without-nesting-limit not supported]])
    ;;
    [[0-9]]*)
    ;;
    *) AC_MSG_ERROR([[nesting limit must be numeric]])
  esac],
  [with_nesting_limit=256])

AC_DEFINE_UNQUOTED(DEFAULT_NESTING_LIMIT, [$with_nesting_limit],
  [Default nesting level value.])

AC_CACHE_CHECK([whether the wide literal encoding is UTF-32],
               wcjson_cv_wchar_t_is_utf32,
               [AC_RUN_IFELSE(
  [AC_LANG_PROGRAM([
#ifdef HAVE_WCHAR_H
#include<wchar.h>
#endif
wchar_t *s = L"\U0010FFFF";],
  [return sizeof(wchar_t) >= 4 && wcslen(s) == 1 && *s == 0x10FFFF ? 0 : 1;])],
              [wcjson_cv_wchar_t_is_utf32="yes"],
              [wcjson_cv_wchar_t_is_utf32="no"])])

if test $wcjson_cv_wchar_t_is_utf32 = "yes"; then
  AC_DEFINE_UNQUOTED(WCHAR_T_UTF32, [1], [wchar_t is UTF-32])
fi

AC_CACHE_CHECK([whether the wide literal encoding is UTF-16],
               wcjson_cv_wchar_t_is_utf16,
               [AC_RUN_IFELSE(
  [AC_LANG_PROGRAM([
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
wchar_t *s = L"\U0010FFFF";],
  [return sizeof(wchar_t) == 2 && wcslen(s) == 2 && s[[0]] == 0xdbff
    && s[[1]] == 0xdfff ? 0 : 1;])],
                [wcjson_cv_wchar_t_is_utf16="yes"],
                [wcjson_cv_wchar_t_is_utf16="no"])])

if test $wcjson_cv_wchar_t_is_utf16 = "yes"; then
  AC_DEFINE_UNQUOTED(WCHAR_T_UTF16, [1], [wchar_t is UTF-16])
fi

AC_CACHE_CHECK([whether the wide literal encoding is UTF-8],
               wcjson_cv_wchar_t_is_utf8,
               [AC_RUN_IFELSE(
  [AC_LANG_PROGRAM([
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
wchar_t *s = L"\U0010FFFF";],
  [return sizeof(wchar_t) == 1 && wcslen(s) == 4 && s[[0]] == 0xf4
    && s[[1]] == 0x8f && s[[2]] == 0xbf && s[[3]] == 0xbf ? 0 : 1;])],
                 [wcjson_cv_wchar_t_is_utf8="yes"],
                 [wcjson_cv_wchar_t_is_utf8="no"])])

if test $wcjson_cv_wchar_t_is_utf8 = "yes"; then
  AC_DEFINE_UNQUOTED(WCHAR_T_UTF8, [1], [wchar_t is UTF-8])
fi

AX_CHECK_COMPILE_FLAG([-Werror], [CFLAGS="$CFLAGS -Werror"])
AX_CHECK_COMPILE_FLAG([-Wall], [CFLAGS="$CFLAGS -Wall"])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile
                 tests/atlocal])
AC_OUTPUT
