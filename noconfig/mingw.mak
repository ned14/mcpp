# makefile to compile MCPP version 2.6.* for MinGW / GCC / GNU make
#   2006/11   kmatsui
#
# First, you must edit GCCDIR, BINDIR, INCDIR, gcc_maj_ver and gcc_min_ver.
# To make compiler-independent-build of MCPP do:
#       make; make install
# To make GCC-specific-build of MCPP:
#       make COMPILER=GNUC; make COMPILER=GNUC install
# To re-compile MCPP using GCC-specific-build of MCPP do:
#       make COMPILER=GNUC PREPROCESSED=1
# To link malloc() package of kmatsui do:
#       make [COMPILER=GNUC] [PREPROCESSED=1] MALLOC=KMMALLOC
# To compile cpp with C++, rename *.c other than lib.c and preproc.c to *.cc,
#   and do:
#       make CPLUS=1

# COMPILER:
#   Specify whether make a compiler-independent-build or GCC-specific-build
# compiler-independent-build:   empty
# compiler-specific-build:      GNUC

# NAME: name of mcpp executable
NAME = mcpp

# CC:   name of gcc executable
#       e.g. gcc, mingw32-gcc
CC = gcc
GPP = g++
CFLAGS = -c -O2 -Wall   #-v 
CPPFLAGS =

LINKFLAGS = -o $(NAME)

ifeq    ($(COMPILER), )
# compiler-independent-build
CPPOPTS =
# BINDIR:   /usr/bin or /usr/local/bin
BINDIR = /usr/local/bin
# INCDIR:   empty
INCDIR =
else
# compiler-specific-build:  Adjust for your system

ifeq    ($(COMPILER), GNUC)
CPPOPTS = -DCOMPILER=$(COMPILER)
# BINDIR:   the directory where cc1 resides
BINDIR = /mingw/libexec/gcc/mingw32/3.4.5
# INCDIR:   version specific include directory
INCDIR = /mingw/lib/gcc/mingw32/3.4.5/include
# set GCC version
gcc_maj_ver = 3
gcc_min_ver = 4
cpp_call = $(BINDIR)/cc1.exe
endif
endif

# The directory where 'gcc' (cc) command is located
GCCDIR = /mingw/bin

CPLUS =
ifeq	($(CPLUS), 1)
	GCC = $(GPP)
	preproc = preproc.cc
else
	GCC = $(CC)
	preproc = preproc.c
endif

ifneq   ($(MALLOC), )
ifeq    ($(MALLOC), KMMALLOC)
        MEMLIB = /mingw/lib/libkmmalloc_debug.a   # -lkmmalloc_debug
        MEM_MACRO = -D_MEM_DEBUG -DXMALLOC
endif
        MEM_MACRO += -D$(MALLOC)
else
        MEMLIB =
        MEM_MACRO =
endif

OBJS = main.o directive.o eval.o expand.o support.o system.o mbchar.o lib.o

ifeq    ($(COMPILER), )
ifeq    ($(MCPP_LIB), 1)
# compiler-independent-build and MCPP_LIB is specified:
# use mcpp as a subroutine from testmain.c
OBJS += testmain.o
CFLAGS += -DMCPP_LIB
NAME = testmain
endif
endif

$(NAME): $(OBJS)
	$(GCC) $(LINKFLAGS) $(OBJS)
ifeq    ($(COMPILER), GNUC)
	$(GCC) cc1.c -o cc1.exe $(MEMLIB)
endif

PREPROCESSED = 0

ifeq	($(PREPROCESSED), 1)
CMACRO = -DPREPROCESSED
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
ifeq    ($(COMPILER), GNUC)
	$(GCC) -v -E -Wp,-b $(CPPFLAGS) $(CPPOPTS) $(MEM_MACRO) -o mcpp.H $(preproc)
else
	@echo "Do 'sudo make COMPILER=GNUC install' prior to recompile."
	@echo "Then, do 'make COMPILER=GNUC PREPROCESSED=1'."
	@exit
endif
$(OBJS) : mcpp.H
else
CMACRO = $(CPPOPTS) $(MEM_MACRO)
$(OBJS) : noconfig.H
main.o directive.o eval.o expand.o support.o system.o mbchar.o:   \
        system.H internal.H
endif

ifeq	($(CPLUS), 1)
.cc.o	:
	$(GPP) $(CFLAGS) $(CMACRO) $(CPPFLAGS) $<
.c.o	:
	$(CC) $(CFLAGS) $(CMACRO) $(CPPFLAGS) $<
else
.c.o	:
	$(CC) $(CFLAGS) $(CMACRO) $(CPPFLAGS) $<
endif

install :
	install -s -b $(NAME).exe $(BINDIR)/$(NAME).exe
ifeq    ($(COMPILER), GNUC)
	./set_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)' \
            '$(cpp_call)' '$(CC)' '$(GPP)' 'x.exe' 'ln -s' '$(INCDIR)'
endif

clean	:
	-rm *.o $(NAME).exe cc1.exe mcpp.H mcpp.err

uninstall:
	rm -f $(BINDIR)/$(NAME).exe
ifeq    ($(COMPILER), GNUC)
	./unset_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'   \
            '$(cpp_call)' '$(CC)' '$(GPP)' 'x.exe' 'ln -s' '$(INCDIR)'
endif

