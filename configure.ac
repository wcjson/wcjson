#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.72])
AC_INIT([wcjson], [0.20], [cs@schulte.it], [wcjson], [http://wcjson.de])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects 1.16])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/wcjson.h])
AC_CONFIG_TESTDIR([tests])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_LIBOBJ_DIR([compat])
AC_CANONICAL_HOST
AC_LANG([C])

# Automake macros first
AM_PROG_AR
AM_MISSING_PROG([AUTOM4TE], [autom4te])

# Checks for programs.
AC_PROG_CXX
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
AC_FUNC_REALLOC
AC_CHECK_LIB(bsd, recallocarray)

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
AX_CHECK_COMPILE_FLAG([-Wextra], [CFLAGS="$CFLAGS -Wextra"])
AX_CHECK_COMPILE_FLAG([-Wstrict-prototypes], [CFLAGS="$CFLAGS -Wstrict-prototypes"])
AX_CHECK_COMPILE_FLAG([-Wmissing-prototypes], [CFLAGS="$CFLAGS -Wmissing-prototypes"])
AX_CHECK_COMPILE_FLAG([-Wmissing-declarations], [CFLAGS="$CFLAGS -Wmissing-declarations"])
AX_CHECK_COMPILE_FLAG([-Wshadow], [CFLAGS="$CFLAGS -Wshadow"])
AX_CHECK_COMPILE_FLAG([-Wpointer-arith], [CFLAGS="$CFLAGS -Wpointer-arith"])
AX_CHECK_COMPILE_FLAG([-Wcast-qual], [CFLAGS="$CFLAGS -Wcast-qual"])
AX_CHECK_COMPILE_FLAG([-Wsign-compare], [CFLAGS="$CFLAGS -Wsign-compare"])
AX_CHECK_COMPILE_FLAG([-Walloca], [CFLAGS="$CFLAGS -Walloca"])
AX_CHECK_COMPILE_FLAG([-Warray-parameter], [CFLAGS="$CFLAGS -Warray-parameter"])
AX_CHECK_COMPILE_FLAG([-Wbad-function-cast], [CFLAGS="$CFLAGS -Wbad-function-cast"])
AX_CHECK_COMPILE_FLAG([-Wbool-operation], [CFLAGS="$CFLAGS -Wbool-operation"])
AX_CHECK_COMPILE_FLAG([-Wcast-align], [CFLAGS="$CFLAGS -Wcast-align"])
AX_CHECK_COMPILE_FLAG([-Wcast-function-type], [CFLAGS="$CFLAGS -Wcast-function-type"])
AX_CHECK_COMPILE_FLAG([-Wcomment], [CFLAGS="$CFLAGS -Wcomment"])
AX_CHECK_COMPILE_FLAG([-Wconditional-uninitialized], [CFLAGS="$CFLAGS -Wconditional-uninitialized"])
AX_CHECK_COMPILE_FLAG([-Wconversion], [CFLAGS="$CFLAGS -Wconversion"])
AX_CHECK_COMPILE_FLAG([-Wdeprecated], [CFLAGS="$CFLAGS -Wdeprecated"])
AX_CHECK_COMPILE_FLAG([-Wfloat-conversion], [CFLAGS="$CFLAGS -Wfloat-conversion"])
AX_CHECK_COMPILE_FLAG([-Wformat-pedantic], [CFLAGS="$CFLAGS -Wformat-pedantic"])
AX_CHECK_COMPILE_FLAG([-Wformat-signedness], [CFLAGS="$CFLAGS -Wformat-signedness"])
AX_CHECK_COMPILE_FLAG([-Wno-unused-parameter], [CFLAGS="$CFLAGS -Wno-unused-parameter"])


AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile
                 tests/atlocal
                 compat/Makefile])
AC_OUTPUT
