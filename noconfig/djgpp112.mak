# makefile to compile MCPP version 2.* for DJGPP / GNU make
#		1998/08, 2003/11, 2004/02	  kmatsui
# To compile MCPP using resident cpp do
#		make
# To re-compile MCPP using compiled MCPP do
#		make PREPROCESSED=1
# To generate MCPP of modes other than STANDARD mode do as
#		make MODE=POST_STANDARD NAME=cpp_post
# To link malloc() package of kmatsui do
#		make [PREPROCESSED=1] MALLOC=KMMALLOC
# To compile MCPP with C++, rename *.c other than lib.c and preproc.c to *.cc
#   then do
#		make CPLUS=1

NAME = cpp_std
CPP= cpp_std
CC = gcc
CFLAGS = -c -O2 -Wall
LINKFLAGS = -o $(NAME)
# Adjust to your system.
DJGDIR = E:\DJ112

ifneq	($(MALLOC), )
ifeq	($(MALLOC), KMMALLOC)
	MEMLIB = -lkmm_d
	MEM_MACRO = -D_MEM_DEBUG -DXMALLOC
endif
	MEM_MACRO += -D$(MALLOC)
else
	MEMLIB =
	MEM_MACRO =
endif

OBJS = main.o control.o eval.o expand.o support.o system.o mbchar.o lib.o

$(NAME) : $(OBJS)
	$(CC) $(LINKFLAGS) $(OBJS) $(MEMLIB)
#	strip $(NAME)
	coff2exe $(NAME)

CPPFLAGS =

ifdef	MODE
CPPFLAGS += -DMODE=$(MODE)
endif

PREPROCESSED = 0

ifeq	($(PREPROCESSED), 1)
CMACRO =
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
cpp.H	: system.H noconfig.H internal.H
ifeq	($(CPLUS), 1)
	$(CC) -E -Wp,-b $(CPPFLAGS) $(MEM_MACRO) -o cpp.H preproc.cc
else
	$(CC) -E -Wp,-b $(CPPFLAGS) $(MEM_MACRO) -o cpp.H preproc.c
endif
$(OBJS) : cpp.H
else
CMACRO = $(MEM_MACRO)
$(OBJS) : system.H noconfig.H
main.o control.o eval.o expand.o support.o system.o mbchar.o: internal.H
endif

CPPFLAGS += -DPREPROCESSED=$(PREPROCESSED)

ifeq	($(CPLUS), 1)
.cc.o	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CMACRO) $<
.c.o	:
	$(CC) $(CFLAGS) $(CMACRO) $<
else
.c.o	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CMACRO) $<
endif

install :
	copy $(NAME).exe $(DJGDIR)\BIN\$(NAME).exe

clean	:
	-del *.o
	-del *.bak
	-del $(NAME)
	-del $(NAME).exe
	-del cpp.H

