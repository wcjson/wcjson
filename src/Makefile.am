lib_LTLIBRARIES = libwcjson.la
libwcjson_la_SOURCES = wcjson.c wcjson-document.c optparse.h
libwcjson_la_LDFLAGS = -version-info 0:0:0
include_HEADERS = wcjson.h wcjson-document.h
dist_man_MANS = wcjson.1 wcjson.3 wcjson-document.3

bin_PROGRAMS = wcjson
wcjson_SOURCES = wcjson-cli.c
wcjson_LDADD = libwcjson.la

AM_CPPFLAGS=	-I$(top_srcdir)/compat
