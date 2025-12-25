AM_CPPFLAGS=-I$(top_builddir)/src -I$(top_srcdir)/src

lib_LTLIBRARIES=libwcjson.la
libwcjson_la_SOURCES=wcjson.c wcjson-document.c optparse.h
libwcjson_la_LDFLAGS=-version-info @wcjson_version_info@
pkginclude_HEADERS=wcjson.h wcjson-document.h
dist_man_MANS=wcjson.1 wcjson.3 wcjson-document.3

bin_PROGRAMS=wcjson
wcjson_SOURCES=wcjson-cli.c
wcjson_LDADD=libwcjson.la

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=wcjson.pc 

DISTCLEANFILES=wcjson.pc
