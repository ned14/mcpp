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
  $(top_srcdir)/mcpp.1 $(top_srcdir)/mcpp-gcc.1 $(top_srcdir)/config/mb_*   \
  $(top_srcdir)/src/preproc.c $(top_srcdir)/src/noconfig.H  \
  $(top_srcdir)/src/*set_mcpp.sh $(top_srcdir)/tests/*_test.sh  \
  $(top_srcdir)/noconfig $(top_srcdir)/cpp-test $(top_srcdir)/test-t    \
  $(top_srcdir)/test-c $(top_srcdir)/test-l $(top_srcdir)/tool  \
  $(top_srcdir)/doc $(top_srcdir)/doc-jp

install-data:
	$(top_srcdir)/config/install-sh -d "$(DESTDIR)$(prefix)/share/doc/mcpp"
	$(top_srcdir)/config/install-sh -d "$(DESTDIR)$(prefix)/man/man1"
	cp -Rpf $(top_srcdir)/LICENSE $(top_srcdir)/cpp-test \
            $(top_srcdir)/test-t $(top_srcdir)/test-c $(top_srcdir)/test-l  \
            $(top_srcdir)/tool $(top_srcdir)/doc $(top_srcdir)/doc-jp   \
            $(DESTDIR)$(prefix)/share/doc/mcpp
	gzip -c $(top_srcdir)/$(man_page).1 >   \
            $(DESTDIR)$(prefix)/man/man1/$(man_page).1.gz

uninstall-data:
	rm -r -f $(DESTDIR)$(prefix)/share/doc/mcpp \
            $(DESTDIR)$(prefix)/man/man1/$(man_page).1.gz

