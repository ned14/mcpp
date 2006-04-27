# makefile to compile MCPP version 2.* for CygWIN / GNU C / GNU make
#		2002/08, 2003/11, 2004/02, 2005/03	kmatsui
# To compile MCPP using resident cpp do
#		make
# To re-compile MCPP using compiled MCPP do
#		make PREPROCESSED=1
# To generate MCPP of PRE_STANDARD mode do as
#		make MODE=PRE_STANDARD NAME=mcpp_prestd
# To compile MCPP with C++, rename *.c other than lib.c and preproc.c to *.cc
#   then do
#		make CPLUS=1

NAME = mcpp_std
CPP = cpp0
CC = gcc
GPP = c++
GCC = $(CC)
CFLAGS = -c -O2 -Wall

# To use MCPP on GCC 3.x
#CFLAGS = -c -O2 -Wall -no-integrated-cpp

LINKFLAGS = -o $(NAME)
BINDIR ?= /usr/lib/gcc-lib/i686-pc-cygwin/2.95.3-5
# Adjust for your system

CPLUS =
ifeq	($(CPLUS), 1)
	GCC = $(GPP)
endif

OBJS = main.o control.o eval.o expand.o support.o system.o mbchar.o lib.o

$(NAME) : $(OBJS)
	$(GCC) $(LINKFLAGS) $(OBJS)

CPPFLAGS =
ifdef	MODE
CPPFLAGS += -DMODE=$(MODE)
endif

PREPROCESSED = 0

ifeq	($(PREPROCESSED), 1)
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
ifeq	($(CPLUS), 1)
	$(GCC) -E -Wp,-b -o mcpp.H preproc.cc
else
	$(GCC) -E -Wp,-b -o mcpp.H preproc.c
endif

$(OBJS) : mcpp.H
else
$(OBJS) : system.H noconfig.H
main.o control.o eval.o expand.o support.o system.o mbchar.o: internal.H
endif

CPPFLAGS += -DPREPROCESSED=$(PREPROCESSED)

ifeq	($(CPLUS), 1)
.cc.o	:
	$(GPP) $(CFLAGS) $(CPPFLAGS) $<
.c.o	:
	$(CC) $(CFLAGS) $(CMACRO) $<
else
.c.o	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $<
endif

install :
	install -s -b -o root -g Administrators $(NAME).exe $(BINDIR)/$(NAME).exe
# Do backup GNU C / cpp.exe (cpp0.exe) before executing the following command.
#ifneq	($(NAME), $(CPP))
#	ln -f -s $(BINDIR)/$(NAME).exe $(BINDIR)/$(CPP).exe
#endif

clean	:
	-rm *.o *.exe mcpp.H
