lib_LTLIBRARIES = libwcjson.la
libwcjson_la_SOURCES = wcjson.c
libwcjson_la_LDFLAGS = -version-info 0:0:0
include_HEADERS = wcjson.h
dist_man_MANS = wcjson.1 wcjson.3

bin_PROGRAMS = wcjson
wcjson_SOURCES = wcjson_cli.c
wcjson_LDADD = libwcjson.la
