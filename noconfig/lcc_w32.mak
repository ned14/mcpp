# makefile to compile MCPP version 2.* for LCC-Win32 / LCC make
#		2002/08, 2003/11, 2004/02, 2005/03 	kmatsui
# To compile MCPP do
#		make
# To re-compile MCPP using compiled MCPP, edit this makefile and do
#		make
# To link malloc() package of kmatsui, edit this makefile and do
#		make

NAME = mcpp_std.exe
CPP = mcpp_std.exe
CC = lcc
CFLAGS = -A
LINKFLAGS = -s -o $(NAME)
BINDIR = \LCC\bin
# Adjust for your system

# To link kmatsui's malloc()
#MEMLIB = kmmalloc_debug.lib
#MEM_MACRO = -DKMMALLOC -D_MEM_DEBUG -DXMALLOC
# else
MEMLIB =
MEM_MACRO =

OBJS = main.obj control.obj eval.obj expand.obj support.obj system.obj  \
    mbchar.obj lib.obj

$(NAME) : $(OBJS)
	lcclnk $(LINKFLAGS) *.obj $(MEMLIB)

PREPROCESSED = 0
CMACRO = $(MEM_MACRO)
$(OBJS) : system.H noconfig.H
main.obj control.obj eval.obj expand.obj support.obj system.obj mbchar.obj: \
    internal.H
# To make MCPP using MCPP itself, comment out the above 4 lines and
#		uncomment the next 5 lines.
#PREPROCESSED = 1
#CMACRO =
#mcpp.H : system.H noconfig.H internal.H
#	$(BINDIR)\$(CPP) $(CPPFLAGS) $(MEM_MACRO) preproc.c mcpp.H
#$(OBJS) : mcpp.H

CPPFLAGS = -DPREPROCESSED=$(PREPROCESSED) -DMODE=STANDARD
.c.obj	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CMACRO) $<
# To make MCPP using MCPP itself, comment out the above line and
#		uncomment the next 2 lines.
#	$(CPP) $(CPPFLAGS) $< _$<
#	$(CC) $(CFLAGS) $(CMACRO) _$<

install :
	copy $(NAME) $(BINDIR)\$(NAME)

clean	:
	-del *.obj
	-del *.exe
	-del mcpp.H
	-del _*.c

