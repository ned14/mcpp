# makefile to compile MCPP version 2.6 for Linux / GCC / GNU make
#       2006/05     kmatsui
#
# First, you must edit GCCDIR, BINDIR, INCDIR, gcc_maj_ver and gcc_min_ver.
# To make stand-alone-build of MCPP:
#       make
# To make GCC-specific-build of MCPP:
#       make COMPILER=GNUC
# To re-compile MCPP using GCC-specific-build of MCPP do:
#       make COMPILER=GNUC PREPROCESSED=1
# To link malloc() package of kmatsui do:
#       make [COMPILER=GNUC] [PREPROCESSED=1] MALLOC=KMMALLOC
# To compile cpp with C++, rename *.c other than lib.c to *.cc,
#   and do:
#       make CPLUS=1

# COMPILER: Specify whether make a stand-alone-build or GCC-specific-build
# stand-alone-build:    empty
# compiler-specific-build:  GNUC

# NAME: name of mcpp executable
NAME = mcpp

# CC:   name of gcc executable
#       e.g. cc, gcc, gcc-2.95.3, i686-pc-linux-gnu-gcc-3.4.3
CC = gcc
GPP = g++
CFLAGS = -c -O2 -Wall   # -v
CPPFLAGS =
#CPPFLAGS = -Wp,-v,-Q,-W3
    # for MCPP to output a bit verbose diagnosis to "mcpp.err"

LINKFLAGS = -o $(NAME)

ifeq    ($(COMPILER), )
# stand-alone-build
CPPOPTS =
# BINDIR:   /usr/bin or /usr/local/bin
BINDIR = /usr/local/bin
# INCDIR:   empty
INCDIR =
else
# compiler-specific-build:  Adjust for your system

ifeq    ($(COMPILER), GNUC)
CPPOPTS = -DCOMPILER=$(COMPILER)
# BINDIR:   the directory where cpp0 or cc1 resides
#BINDIR = /usr/lib/gcc-lib/i386-redhat-linux/2.95.3
#BINDIR = /usr/local/gcc-3.2/lib/gcc-lib/i686-pc-linux-gnu/3.2
BINDIR = /usr/lib/gcc-lib/i386-vine-linux/3.3.2
#BINDIR = /usr/local/libexec/gcc/i686-pc-linux-gnu/3.4.3
#BINDIR = /usr/lib/gcc/i586-suse-linux/4.0.2
# INCDIR:   version specific include directory
#INCDIR = /usr/lib/gcc-lib/i386-redhat-linux/2.95.3/include
#INCDIR = /usr/local/gcc-3.2/lib/gcc-lib/i686-pc-linux-gnu/3.2/include
INCDIR = /usr/lib/gcc-lib/i386-vine-linux/3.3.2/include
#INCDIR = /usr/local/lib/gcc/i686-pc-linux-gnu/3.4.3/include
#INCDIR = /usr/lib/gcc/i586-suse-linux/4.0.2/include
# set GCC version
gcc_maj_ver = 3
gcc_min_ver = 3
ifeq ($(gcc_maj_ver), 2)
cpp_call = $(BINDIR)/cpp0
else
cpp_call = $(BINDIR)/cc1
endif
endif
endif

# The directory where 'gcc' (cc) command is located
GCCDIR = /usr/bin
#GCCDIR = /usr/local/bin

CPLUS =
ifeq	($(CPLUS), 1)
	GCC = $(GPP)
	preproc = preproc.cc
else
	GCC = $(CC)
	preproc = preproc.c
endif

ifneq	($(MALLOC), )
ifeq	($(MALLOC), KMMALLOC)
	MEMLIB = /usr/local/lib/libkmmalloc_debug.a   # -lkmmalloc_debug
	MEM_MACRO = -D_MEM_DEBUG -DXMALLOC
endif
	MEM_MACRO += -D$(MALLOC)
else
	MEMLIB =
	MEM_MACRO =
endif

OBJS = main.o control.o eval.o expand.o support.o system.o mbchar.o lib.o

$(NAME): $(OBJS)
	$(GCC) $(LINKFLAGS) $(OBJS) $(MEMLIB)

PREPROCESSED = 0

ifeq	($(PREPROCESSED), 1)
CMACRO = -DPREPROCESSED
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
ifeq    ($(COMPILER), GNUC)
	$(GCC) -E -Wp,-b  $(CPPFLAGS) $(CPPOPTS) $(MEM_MACRO) -o mcpp.H $(preproc)
else
	@echo "Do 'sudo make COMPILER=GNUC install' prior to recompile."
	@echo "Then, do 'make COMPILER=GNUC PREPROCESSED=1'."
	@exit
endif
$(OBJS) : mcpp.H
else
CMACRO = $(MEM_MACRO) $(CPPOPTS)
$(OBJS) : noconfig.H
main.o control.o eval.o expand.o support.o system.o mbchar.o:   \
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
	install -s $(NAME) $(BINDIR)/$(NAME)
ifeq    ($(COMPILER), GNUC)
	@./set_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)' \
            '$(cpp_call)' '$(CC)' '$(GPP)' 'x' 'ln -s' '$(INCDIR)'
endif

clean	:
	-rm *.o mcpp mcpp.H mcpp.err

uninstall:
	rm -f $(BINDIR)/$(NAME)
ifeq    ($(COMPILER), GNUC)
	@./unset_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'   \
            '$(cpp_call)' '$(CC)' '$(GPP)' 'x' '$(INCDIR)'
endif

