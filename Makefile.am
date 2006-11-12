## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

SUBDIRS = src
if REPLACE_CPP
SUBDIRS += tests .
man_page = mcpp-gcc
else
man_page = mcpp
endif

EXTRA_DIST = $(top_srcdir)/LICENSE $(top_srcdir)/INSTALL*   \
  $(top_srcdir)/ChangeLog.old $(top_srcdir)/NEWS    \
  $(top_srcdir)/mcpp.1 $(top_srcdir)/mcpp-gcc.1 $(top_srcdir)/config/mb_*   \
  $(top_srcdir)/src/preproc.c $(top_srcdir)/src/noconfig.H  \
  $(top_srcdir)/src/cc1.c $(top_srcdir)/src/*set_mcpp.sh 	\
  $(top_srcdir)/src/testmain.c	\
  $(top_srcdir)/tests/*_test.sh $(top_srcdir)/noconfig  \
  $(top_srcdir)/cpp-test $(top_srcdir)/test-t    \
  $(top_srcdir)/test-c $(top_srcdir)/test-l $(top_srcdir)/tool  \
  $(top_srcdir)/doc $(top_srcdir)/doc-jp

install-data:
	$(top_srcdir)/config/install-sh -d "$(DESTDIR)$(prefix)/share/doc/mcpp"
	cp -Rpf $(top_srcdir)/cpp-test \
            $(top_srcdir)/test-t $(top_srcdir)/test-c $(top_srcdir)/test-l  \
            $(top_srcdir)/tool $(top_srcdir)/doc $(top_srcdir)/doc-jp   \
            $(DESTDIR)$(prefix)/share/doc/mcpp

uninstall-data:
	rm -r -f $(DESTDIR)$(prefix)/share/doc/mcpp/cpp-test
	rm -r -f $(DESTDIR)$(prefix)/share/doc/mcpp/test-t
	rm -r -f $(DESTDIR)$(prefix)/share/doc/mcpp/test-c
	rm -r -f $(DESTDIR)$(prefix)/share/doc/mcpp/test-l
	rm -r -f $(DESTDIR)$(prefix)/share/doc/mcpp/tool
	rm -r -f $(DESTDIR)$(prefix)/share/doc/mcpp/doc
	rm -r -f $(DESTDIR)$(prefix)/share/doc/mcpp/doc-jp

