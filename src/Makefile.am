lib_LTLIBRARIES = libwcjson.la libwcjson-document.la
libwcjson_la_SOURCES = wcjson.c
libwcjson_la_LDFLAGS = -version-info 0:1:0
libwcjson_document_la_SOURCES = wcjson-document.c
libwcjson_document_la_LDFLAGS = -version-info 0:0:0
include_HEADERS = wcjson.h wcjson-document.h
dist_man_MANS = wcjson.1 wcjson.3 wcjson-document.3

bin_PROGRAMS = wcjson
wcjson_SOURCES = wcjson-cli.c
wcjson_LDADD = libwcjson.la libwcjson-document.la
