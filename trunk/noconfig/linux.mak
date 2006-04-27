# makefile to compile MCPP version 2.* for Linux / GNU C / GNU make
#		2002/08, 2003/11, 2004/02, 2004/11, 2005/03     kmatsui
# To compile MCPP using resident cpp do
#		make
# To re-compile MCPP using compiled MCPP do
#		make PREPROCESSED=1
# To generate MCPP of PRE_STANDARD mode do as
#		make MODE=PRE_STANDARD NAME=mcpp_prestd
# To link malloc() package of kmatsui do
#		make [PREPROCESSED=1] MALLOC=KMMALLOC
# To compile cpp with C++, rename *.c other than lib.c and preproc.c to *.cc,
#   and do
#       make CPLUS=1
# $(NAME), $(CPP) can be over-ridden by command-line as
#		make NAME=mcpp_std CPP=cpp0

NAME = mcpp_std
#CPP = cpp0
CPP = cc1   # for GCC 3.3 or later
CC = gcc
GPP = g++
GCC = $(CC)
#CFLAGS = -c -O2 -Wall
CPPOPTS =
#CPPOPTS = -Wp,-v,-Q,-W3            # for MCPP

# for gcc 3.x to use MCPP
CFLAGS = -c -O2 -no-integrated-cpp -Wall

# Don't use -ansi -pedantic to compile eval.c when OK_SIZE is TRUE
LINKFLAGS = -o $(NAME)

# Adjust for your system
#BINDIR = /usr/lib/gcc-lib/i386-redhat-linux/2.95.3
#BINDIR = /usr/local/gcc-3.2/lib/gcc-lib/i686-pc-linux-gnu/3.2
BINDIR = /usr/lib/gcc-lib/i368-vine-linux/3.3.2

# The directory 'gcc' (cc) command is located
GCCDIR = /usr/bin

ifeq    ($(MODE), )
    MODE = STANDARD
endif
CPPFLAGS = -DMODE=$(MODE)

CPLUS =
ifeq	($(CPLUS), 1)
	GCC = $(GPP)
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

$(NAME) : $(OBJS)
	$(GCC) $(LINKFLAGS) $(OBJS) $(MEMLIB)

PREPROCESSED = 0

ifeq	($(PREPROCESSED), 1)
CMACRO = -DPREPROCESSED
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
	$(GCC) -E -Wp,-b  $(CPPOPTS) $(CPPFLAGS) $(MEM_MACRO) -o mcpp.H preproc.c
$(OBJS) : mcpp.H
else
CMACRO = $(MEM_MACRO) $(CPPFLAGS)
$(OBJS) : system.H noconfig.H
main.o control.o eval.o expand.o support.o system.o mbchar.o: internal.H
endif

ifeq	($(CPLUS), 1)
.cc.o	:
	$(CC) $(CFLAGS) $(CMACRO) $(CPPOPTS) $<
.c.o	:
	$(CC) $(CFLAGS) $(CMACRO) $(CPPOPTS) $<
else
.c.o	:
	$(CC) $(CFLAGS) $(CMACRO) $(CPPOPTS) $<
endif

# set GCC version
gcc_maj_ver = 3
gcc_min_ver = 3

install :
	install $(NAME) $(BINDIR)/$(NAME)
	install $(NAME) /usr/local/bin/$(NAME)
# for GCC 3.3 or later
	echo '' | $(CC) -E -xc -dM - | sort | grep ' *#define *_' > mcpp_gcc$(gcc_maj_ver)$(gcc_min_ver)_predef_std.h
	-echo '' | $(CC) -E -xc -dM - | sort | grep -E ' *#define *[A-Za-z]+' > mcpp_gcc$(gcc_maj_ver)$(gcc_min_ver)_predef_old.h
	echo '' | $(GPP) -E -xc++ -dM - | sort | grep ' *#define *_' > mcpp_gxx$(gcc_maj_ver)$(gcc_min_ver)_predef_std.h
	-echo '' | $(GPP) -E -xc++ -dM - | sort | grep -E ' *#define *[A-Za-z]+' > mcpp_gxx$(gcc_maj_ver)$(gcc_min_ver)_predef_old.h
	./set_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)' '$(BINDIR)/cc1' '$(CC)' '$(GPP)' 'x' 'ln -s' '/usr/local/include'

clean	:
	-rm *.o mcpp.H mcpp.err

uninstall:
#	rm -f $(BINDIR)/$(NAME)
# for GCC 3.3 or later
	./unset_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)' '$(BINDIR)/cc1' '$(CC)' '$(GPP)' 'x' '/usr/local/include'

