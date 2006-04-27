# makefile to compile MCPP version 2.* for LSI C-86 with LSI make
#		1998/08, 2003/11, 2004/02		kmatsui
# To compile MCPP using resident cpp do
#		make
# To re-compile MCPP using compiled MCPP, edit this makefile and do
#		make
# To link malloc() package of kmatsui, edit this makefile and do
#		make
# $(NAME), $(CPP) can be over-ridden by command-line as
#		make NAME=cpp_std

NAME = cpp_std
CPP = cpp
CC = lcc
LINKFLAGS = -k'-s 2000' -o$(NAME).exe -ltinymain.obj

# BINDIR: Adjust to your system.
BINDIR = \LSIC86\BIN
CFLAGS = -c -O -DANSI
BC =

# To install cpp for Borland C
# BINDIR = \BC4\BIN
# BC = -D__BORLANDC__
# CFLAGS = -c -O $(BC)

# To link kmatsui's malloc()
# MEMLIB = -lkmmall_d
# MEM_MACRO = -DKMMALLOC -D_MEM_DEBUG -DXMALLOC
# else
MEMLIB =
MEM_MACRO =

OBJS = main.obj control.obj eval.obj expand.obj support.obj system.obj  \
    mbchar.obj lib.obj

$(NAME).exe	: $(OBJS)
	$(CC) $(OBJS) $(MEMLIB) $(LINKFLAGS)

MODE = STANDARD
CPPFLAGS = -DMODE=$(MODE)

# To make cpp using resident cpp
CMACRO = $(MEM_MACRO) -D__SMALL__
PREPROCESSED = 0
$(OBJS)	: system.H noconfig.H
main.obj control.obj eval.obj expand.obj support.obj system.obj	mbchar.obj: \
    internal.H
# To make MCPP using MCPP itself, comment out the above 4 lines and
#		uncomment the next 5 lines.
# CMACRO =
# PREPROCESSED = 1
# cpp.H	: system.H noconfig.H internal.H
# 	$(CPP) -b -S1 $(CPPFLAGS) $(BC) $(MEM_MACRO) preproc.c cpp.H
# $(OBJS)	: cpp.H

CPPFLAGS = -DMODE=$(MODE) -DPREPROCESSED=$(PREPROCESSED)

.c.obj	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CMACRO) $<

install	:
	copy $(NAME).exe $(BINDIR)\$(NAME).exe

clean	:
	-del *.obj
	-del $(NAME).exe
	-del *.bak
	-del cpp.H

