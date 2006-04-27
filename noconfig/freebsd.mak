# makefile to compile MCPP version 2.* for 4.4BSD / GNU C / UCB make
#		1998/08, 2003/11, 2004/02, 2004/11, 2005/03	kmatsui
# To compile MCPP using resident cpp do
#		make
# To re-compile MCPP using compiled MCPP do
#		make PREPROCESSED=1
# To generate MCPP of PRE_STANDARD mode do as
#		make MODE=PRE_STANDARD NAME=mcpp_prestd
# To link malloc() package of kmatsui do
#		make [PREPROCESSED=1] MALLOC=KMMALLOC
# To compile MCPP with C++, rename *.c other than lib.c and preproc.c to *.cc
#   then do
#		make CPLUS=1

NAME ?= mcpp_std
#CPP ?= cpp0		# for GCC 3.2 or former
CPP ?= cc1		# for GCC 3.3 or later
CC = gcc
GPP = g++
GCC = $(CC)
CPPOPTS = -v -Q
LINKFLAGS = -o $(NAME)

#CFLAGS = -c -O2 -I/usr/local/include -Wall
# for gcc 3.x to use MCPP
CFLAGS = -c -O2 -no-integrated-cpp -v -Wall

# Adjust to your system
BINDIR ?= /usr/libexec
#BINDIR ?= /usr/local/src/gcc-3.2-install/lib/gcc-lib/i386-unknown-freebsd4.7/3.2

# The directory 'gcc' (cc) command is located
GCCDIR ?= /usr/bin

CPLUS =
.if     $(CPLUS)
    GCC = $(GPP)
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
#	strip $(NAME)

PREPROCESSED ?= 0

.if $(PREPROCESSED) == 1
CMACRO =
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
	$(GCC) -E -Wp,-b $(CPPOPTS) $(MEM_MACRO) -o mcpp.H preproc.c
$(OBJS) : mcpp.H
.else
CMACRO = $(MEM_MACRO)
$(OBJS) : system.H noconfig.H
main.o control.o eval.o expand.o support.o system.o mbchar.o: internal.H
.endif

.if $(CPLUS)
.cc.o	:
	$(GPP) $(CFLAGS) $(CMACRO) -DPREPROCESSED=$(PREPROCESSED) $<
.c.o	:
	$(CC) $(CFLAGS) $(CMACRO) $<
.else
.c.o	:
	$(CC) $(CFLAGS) $(CMACRO) -DPREPROCESSED=$(PREPROCESSED) $<
.endif

# Set for your GCC version
gcc_maj_ver = 3
gcc_min_ver = 4

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

