# makefile to compile MCPP version 2.6 for FreeBSD / GCC / UCB make
#       2006/05     kmatsui
#
# First, you must edit GCCDIR, BINDIR, INCDIR, gcc_maj_ver and gcc_min_ver.
# To make stand-alone-build of MCPP do:
#       make
# To make GCC-specific-build of MCPP do:
#       make COMPILER=GNUC
# To re-compile MCPP using GCC-specific-build of MCPP do:
#       make COMPILER=GNUC PREPROCESSED=1
# To link malloc() package of kmatsui do:
#       make [COMPILER=GNUC] [PREPROCESSED=1] MALLOC=KMMALLOC
# To compile MCPP with C++, rename *.c other than lib.c and preproc.c to *.cc
#   then do:
#		make CPLUS=1

# COMPILER: Specify whether make a stand-alone-build or GCC-specific-build
# stand-alone-build:    empty
# compiler-specific-build:  GNUC

# NAME: name of mcpp executable
NAME ?= mcpp
CC = gcc
GPP = g++
CFLAGS = -c -O2 -Wall   # -v
CPPFLAGS =
#CPPFLAGS = -Wp,-v,-Q,-W3
    # for MCPP to output a bit verbose diagnosis to "mcpp.err"

LINKFLAGS = -o $(NAME)

.if     empty(COMPILER)
# stand-alone-build
CPPOPTS =
# BINDIR:   /usr/bin or /usr/local/bin
BINDIR = /usr/local/bin
# INCDIR:   empty
INCDIR =
.else
# compiler-specific-build:  Adjust for your system

.if     $(COMPILER) == GNUC
CPPOPTS = -DCOMPILER=$(COMPILER)
# BINDIR:   the directory where cpp0 or cc1 resides
BINDIR ?= /usr/libexec
#BINDIR ?= /usr/local/src/gcc-3.2-install/lib/gcc-lib/i386-unknown-freebsd4.7/3.2
# INCDIR:   the compiler's version specific include directory, if it exists,
#       /usr/include, if it does not exist
INCDIR = /usr/include
#INCDIR ?= /usr/local/src/gcc-3.2-install/lib/gcc-lib/i386-unknown-freebsd4.7/3.2/include
# Set GCC version
gcc_maj_ver = 3
gcc_min_ver = 4
.if $(gcc_maj_ver) == 2
cpp_call = $(BINDIR)/cpp0
.else
cpp_call = $(BINDIR)/cc1
.endif
.endif
.endif

# The directory 'gcc' (cc) command is located (/usr/bin or /usr/local/bin)
GCCDIR ?= /usr/bin

CPLUS =
.if     $(CPLUS)
    GCC = $(GPP)
    preproc = preproc.cc
.else
    GCC = $(CC)
    preproc = preproc.c
.endif

MALLOC =
.if		!empty(MALLOC)
.if		$(MALLOC) == KMMALLOC
    MEMLIB = /usr/local/lib/libkmmalloc_debug.a	# -lkmmalloc_debug
    MEM_MACRO = -D_MEM_DEBUG -DXMALLOC
.endif
    MEM_MACRO += -D$(MALLOC)
.else
    MEMLIB =
    MEM_MACRO =
.endif

OBJS = main.o control.o eval.o expand.o support.o system.o mbchar.o lib.o

$(NAME) : $(OBJS)
	$(GCC) $(LINKFLAGS) $(OBJS) $(MEMLIB)

PREPROCESSED ?= 0

.if $(PREPROCESSED) == 1
CMACRO = -DPREPROCESSED
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
.if $(COMPILER) == GNUC
	$(GCC) -E -Wp,-b $(CPPFLAGS) $(CPPOPTS) $(MEM_MACRO) -o mcpp.H $(preproc)
.else
    @echo "Do 'sudo make COMPILER=GNUC install' prior to recompile."
    @echo "Then, do 'make COMPILER=GNUC PREPROCESSED=1'."
    @exit
.endif
$(OBJS) : mcpp.H
.else
CMACRO = $(MEM_MACRO) $(CPPOPTS)
$(OBJS) : noconfig.H
main.o control.o eval.o expand.o support.o system.o mbchar.o:   \
        system.H internal.H
.endif

.if $(CPLUS)
.cc.o	:
    $(GPP) $(CFLAGS) $(CMACRO) $(CPPFLAGS) $<
.c.o    :
    $(CC) $(CFLAGS) $(CMACRO) $(CPPFLAGS) $<
.else
.c.o    :
    $(CC) $(CFLAGS) $(CMACRO) $(CPPFLAGS) $<
.endif

install :
	install -s $(NAME) $(BINDIR)/$(NAME)
.if $(COMPILER) == GNUC
	./set_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)' \
            '$(cpp_call)' '$(CC)' '$(GPP)' 'x' 'ln -s' '$(INCDIR)'
.endif

clean	:
	-rm *.o mcpp mcpp.H mcpp.err

uninstall:
	rm -f $(BINDIR)/$(NAME)
.if $(COMPILER) == GNUC
	./unset_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'   \
            '$(cpp_call)' '$(CC)' '$(GPP)' 'x' '$(INCDIR)'
.endif

