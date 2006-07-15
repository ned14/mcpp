# makefile to compile MCPP version 2.6 for CygWIN / GCC / GNU make
# 2006/05     kmatsui
#
# First, you must edit GCCDIR, BINDIR, INCDIR, gcc_maj_ver and gcc_min_ver.
# To make stand-alone-build of MCPP do:
#       make
# To make GCC-specific-build of MCPP:
#       make COMPILER=GNUC
# To re-compile MCPP using GCC-specific-build of MCPP do:
#       make COMPILER=GNUC PREPROCESSED=1
# To link malloc() package of kmatsui do:
#       make [COMPILER=GNUC] [PREPROCESSED=1] MALLOC=KMMALLOC
# To compile cpp with C++, rename *.c other than lib.c and preproc.c to *.cc,
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
CFLAGS = -c -O2 -Wall   -v
CPPFLAGS =
#CPPFLAGS = -Wp,-vQW3
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
#BINDIR = /usr/lib/gcc-lib/i686-pc-cygwin/2.95.3-5
BINDIR = /usr/lib/gcc/i686-pc-cygwin/3.4.4
# INCDIR:   version specific include directory
#INCDIR = /usr/lib/gcc-lib/i686-pc-cygwin/2.95.3-5/include
INCDIR = /usr/lib/gcc/i686-pc-cygwin/3.4.4/include
# set GCC version
gcc_maj_ver = 3
gcc_min_ver = 4
ifeq ($(gcc_maj_ver), 2)
cpp_call = $(BINDIR)/cpp0.exe
else
cpp_call = $(BINDIR)/cc1.exe
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

OBJS = main.o control.o eval.o expand.o support.o system.o mbchar.o lib.o

$(NAME): $(OBJS)
	$(GCC) $(LINKFLAGS) $(OBJS)

PREPROCESSED = 0

ifeq	($(PREPROCESSED), 1)
CMACRO = -DPREPROCESSED
#CPPFLAGS += -std=c99
    # '-std=c99' option is nessecary to work around the confusion of headers
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
ifeq    ($(COMPILER), GNUC)
	$(GCC) -v -E -Wp,-b $(CPPFLAGS) $(CPPOPTS) -o mcpp.H $(preproc)
else
	@echo "Do 'sudo make COMPILER=GNUC install' prior to recompile."
	@echo "Then, do 'make COMPILER=GNUC PREPROCESSED=1'."
	@exit
endif
$(OBJS) : mcpp.H
else
CMACRO = $(CPPOPTS)
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
	install -s -b $(NAME).exe $(BINDIR)/$(NAME).exe
ifeq    ($(COMPILER), GNUC)
	./set_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)' \
            '$(cpp_call)' '$(CC)' '$(GPP)' 'x.exe' 'ln -s' '$(INCDIR)'
endif

clean	:
	-rm *.o mcpp.exe mcpp.H mcpp.err

uninstall:
	rm -f $(BINDIR)/$(NAME).exe
ifeq    ($(COMPILER), GNUC)
	./unset_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'   \
            '$(cpp_call)' '$(CC)' '$(GPP)' 'x.exe' '$(INCDIR)'
endif

