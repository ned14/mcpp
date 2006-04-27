## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

SUBDIRS = src
if MAINTAINER_MODE
SUBDIRS += tests .
endif

EXTRA_DIST = LICENSE INSTALL_* mcpp.1 config/mb_* \
    src/preproc.c src/noconfig.H src/*set_mcpp.sh tests/*_test.sh \
    noconfig cpp-test test-t test-c test-l tool doc doc.eng

install-data:
	@if test -d "$(prefix)/share/doc"; then :; else mkdir $(prefix)/share/doc; fi
	@if test -d "$(prefix)/share/doc/mcpp"; then :; else mkdir $(prefix)/share/doc/mcpp; fi
	cp -Rpf LICENSE cpp-test test-t test-c test-l tool doc doc.eng $(prefix)/share/doc/mcpp
	gzip -c mcpp.1 > $(prefix)/man/man1/mcpp.1.gz

uninstall-data:
	rm -r -f $(prefix)/share/doc/mcpp $(prefix)/man/man1/mcpp.1.gz

