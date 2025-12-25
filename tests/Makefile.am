AM_CPPFLAGS=-I$(top_builddir)/src -I$(top_srcdir)/src
check_PROGRAMS=test-cli
test_cli_SOURCES=test-cli.c
test_cli_LDADD=$(abs_top_builddir)/src/libwcjson.la

# The ':;' works around a Bash 3.2 bug when the output is not writable.
$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	:;{ \
	  echo '# Signature of the current package.' && \
	  echo 'm4_define([AT_PACKAGE_NAME],' && \
	  echo '  [$(PACKAGE_NAME)])' && \
	  echo 'm4_define([AT_PACKAGE_TARNAME],' && \
	  echo '  [$(PACKAGE_TARNAME)])' && \
	  echo 'm4_define([AT_PACKAGE_VERSION],' && \
	  echo '  [$(PACKAGE_VERSION)])' && \
	  echo 'm4_define([AT_PACKAGE_STRING],' && \
	  echo '  [$(PACKAGE_STRING)])' && \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT],' && \
	  echo '  [$(PACKAGE_BUGREPORT)])'; \
	  echo 'm4_define([AT_PACKAGE_URL],' && \
	  echo '  [$(PACKAGE_URL)])'; \
	} >'$(srcdir)/package.m4'

DISTCLEANFILES=${srcdir}/package.m4 atconfig
EXTRA_DIST=testsuite.at $(srcdir)/package.m4 $(TESTSUITE) atlocal.in
TESTSUITE=$(srcdir)/testsuite
TESTSUITEFLAGS=AUTOTEST_PATH=$(abs_top_builddir)/src:$(abs_top_builddir)/tests

check-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)

installcheck-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(bindir)' \
	  $(TESTSUITEFLAGS)

clean-local:
	test ! -f '$(TESTSUITE)' || \
	  $(SHELL) '$(TESTSUITE)' --clean

AUTOTEST=$(AUTOM4TE) --language=autotest
$(TESTSUITE): $(srcdir)/testsuite.at $(srcdir)/package.m4
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@
