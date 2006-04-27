## Process this file with automake to produce Makefile.in

bin_PROGRAMS = mcpp_std
libexec_PROGRAMS = mcpp_std
mcpp_std_SOURCES = system.H configed.H internal.H main.c control.c eval.c expand.c support.c system.c mbchar.c lib.c

if  GNUC
DISTCLEANFILES = mcpp_g*$(gcc_maj_ver)$(gcc_min_ver)_predef_*.h
endif

if MAINTAINER_MODE

bin_PROGRAMS += mcpp_prestd
libexec_PROGRAMS += mcpp_prestd
mcpp_prestd_SOURCES = $(mcpp_std_SOURCES)
mcpp_prestd_CPPFLAGS = -DMODE=PRE_STANDARD

if  GNUC
install-exec-hook:
	@if test x"$(gcc_path)" = x""; then	exit 1; fi
	@./set_mcpp.sh '$(gcc_path)' $(gcc_maj_ver) $(gcc_min_ver) '$(cpp_call)' '$(CC)' '$(CXX)' 'x$(EXEEXT)' '$(LN_S)' '$(inc_dir)'
uninstall-hook:
	@./unset_mcpp.sh '$(gcc_path)' $(gcc_maj_ver) $(gcc_min_ver) '$(cpp_call)' '$(CC)' '$(CXX)' 'x$(EXEEXT)' '$(inc_dir)'
endif

else

if  GNUC
install-exec-hook:
	cp -f mcpp_g*$(gcc_maj_ver)$(gcc_min_ver)_predef_*.h $(inc_dir)
uninstall-hook:
	rm -f $(inc_dir)/mcpp_g*$(gcc_maj_ver)$(gcc_min_ver)_predef_*.h
endif

endif

