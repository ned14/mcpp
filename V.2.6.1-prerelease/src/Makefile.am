## Process this file with automake to produce Makefile.in

bin_PROGRAMS = mcpp
mcpp_SOURCES = system.H configed.H internal.H main.c control.c eval.c expand.c mbchar.c support.c system.c lib.c

main.o control.o eval.o expand.o mbohar.o support.o system.o    : system.H internal.H
lib.o       :   configed.H
system.H    :   configed.H

if REPLACE_CPP
if GNUC

if MINGW
cc1_PROGRAMS = cc1
cc1_SOURCES = cc1.c
cc1dir = $(bindir)
endif

install-exec-hook:
	@if test x"$(gcc_path)" = x""; then	exit 1; fi
	@$(srcdir)/set_mcpp.sh '$(gcc_path)' $(gcc_maj_ver) $(gcc_min_ver) '$(cpp_call)' '$(CC)' '$(CXX)' 'x$(EXEEXT)' '$(LN_S)' '$(inc_dir)'
uninstall-hook:
	@$(srcdir)/unset_mcpp.sh '$(gcc_path)' $(gcc_maj_ver) $(gcc_min_ver) '$(cpp_call)' '$(CC)' '$(CXX)' 'x$(EXEEXT)' '$(LN_S)' '$(inc_dir)'
endif
endif
