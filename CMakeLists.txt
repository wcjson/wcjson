#
# Copyright (c) 2025 Christian Schulte <cs@schulte.it>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
cmake_minimum_required(VERSION 3.31)

project(
	wcjson
	VERSION 0.0.0
	DESCRIPTION "Wide Character JSON for C"
	HOMEPAGE_URL "http://wcjson.de"
	LANGUAGES
	    C
)

include(CheckCSourceRuns)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Christian Schulte")
set(CPACK_PACKAGE_CONTACT "cs@schulte.it")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};wcjson")
set(CPACK_NSIS_MENU_LINKS
	"http://wcjson.de/wcjson.1.html" "Command Documentation"
	"http://wcjson.de/wcjson.3.html" "Parser Documentation"
	"http://wcjson.de/wcjson-document.3.html" "Document Documentation"
)

include(CPack)

check_c_source_runs([[
	#include <wchar.h>
	int main(int argc, char *argv[]) {
		wchar_t *s = L"\\U0010FFFF";
		return sizeof(wchar_t) >= 4 && wcslen(s) == 1
		    && *s == 0x10FFFF ? 0 : -1;
	}
]] WCHAR_T_UTF32)

check_c_source_runs([[
	#include <wchar.h>
	int main(int argc, char *argv[]) {
		wchar_t *s = L"\\U0010FFFF";
		return sizeof(wchar_t) == 2 && wcslen(s) == 2
		    && s[0] == 0xdbff && s[1] == 0xdfff ? 0 : -1;
	}
]] WCHAR_T_UTF16)

check_c_source_runs([[
	#include <wchar.h>
	int main(int argc, char *argv[]) {
		wchar_t *s = L"\\U0010FFFF";
		return sizeof(wchar_t) == 1 && wcslen(s) == 4
		    && s[0] == 0xf4 && s[1] == 0x8f && s[2] == 0xbf
		    && s[3] == 0xbf ? 0 : -1;
	}
]] WCHAR_T_UTF8)

check_c_source_runs([[
	#include <locale.h>
	int main(int argc, char *argv[]) {
		return setlocale(LC_CTYPE, "C") != NULL ? 0 : -1;
	}
]] HAVE_SETLOCALE)

if(WCHAR_T_UTF32)
	add_compile_definitions(WCHAR_T_UTF32)
endif()

if(WCHAR_T_UTF16)
	add_compile_definitions(WCHAR_T_UTF16)
endif()

if(WCHAR_T_UTF8)
	add_compile_definitions(WCHAR_T_UTF8)
endif()

if(HAVE_SETLOCALE)
	add_compile_definitions(HAVE_SETLOCALE)
endif()

add_library(libwcjson)
add_executable(wcjson)

if(BUILD_SHARED_LIBS)
	generate_export_header(
		libwcjson
		BASE_NAME WCJSON
		EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/wcjson-target.h
	)

	target_compile_definitions(
		libwcjson
		PUBLIC
		    WCJSON_TARGET_H
	)

	target_compile_definitions(
		wcjson
		PRIVATE
		    WCJSON_TARGET_H
	)

	target_include_directories(
		libwcjson
		PUBLIC
		    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	)

	install(
		FILES
		    ${CMAKE_CURRENT_BINARY_DIR}/wcjson-target.h
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	)
endif()

target_sources(
	libwcjson
	PRIVATE
	    src/wcjson.c
	    src/wcjson-document.c
	PUBLIC
	    FILE_SET HEADERS
	    BASE_DIRS
	        src
	    FILES
	        src/wcjson.h
	        src/wcjson-document.h
)

target_sources(
	wcjson
	PRIVATE
	    src/wcjson-cli.c
)

target_link_libraries(
	wcjson
	PRIVATE
	    libwcjson
)

set_target_properties(
	libwcjson
	PROPERTIES
	    OUTPUT_NAME wcjson
	    VERSION ${PROJECT_VERSION}
	    SOVERSION ${PROJECT_VERSION_MAJOR}
)

set_target_properties(
	wcjson
	PROPERTIES
	    OUTPUT_NAME wcjson
	    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
	    VERSION ${PROJECT_VERSION}
)

install(
	TARGETS
	    libwcjson
	    wcjson
	EXPORT wcjsonTargets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILE_SET HEADERS
)

configure_package_config_file(
	cmake/wcjsonConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/wcjsonConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/wcjson
	NO_SET_AND_CHECK_MACRO
	NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/wcjsonConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(
	FILES
	    ${CMAKE_CURRENT_BINARY_DIR}/wcjsonConfig.cmake
	    ${CMAKE_CURRENT_BINARY_DIR}/wcjsonConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/wcjson
)

install(
	EXPORT wcjsonTargets
	FILE wcjsonTargets.cmake
	NAMESPACE wcjson::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/wcjson
)

