# makefile to compile MCPP version 2.6 for LCC-Win32 / LCC make
#       2006/11 kmatsui
# You must first edit BINDIR according to your system.
# To compile MCPP do:
#		make
# To re-compile MCPP using compiled MCPP, edit this makefile and do:
#		make
# To link malloc() package of kmatsui, edit this makefile and do:
#		make

NAME = mcpp.exe
CC = lcc
CFLAGS = -A
LINKFLAGS = -s -o $(NAME)
#BINDIR = \PUBLIC\COMPILERS\LCC\bin
    # Adjust for your system
BINDIR = \PUBLIC\BIN

#CPPOPTS= -DCOMPILER=LCC
    # LCC-specific-build

# To link kmatsui's malloc()
#MEMLIB = kmmalloc_debug.lib
#MEM_MACRO = -DKMMALLOC -D_MEM_DEBUG -DXMALLOC
# else
MEMLIB =
MEM_MACRO =

OBJS = main.obj directive.obj eval.obj expand.obj support.obj system.obj  \
    mbchar.obj lib.obj

# To use mcpp as a subroutine from testmain.c,
# uncomment the following 4 lines.
#OBJS = main.obj directive.obj eval.obj expand.obj support.obj system.obj  \
    mbchar.obj lib.obj testmain.obj
#CFLAGS = -A -DMCPP_LIB
#NAME = testmain.exe

$(NAME) : $(OBJS)
	lcclnk $(LINKFLAGS) *.obj $(MEMLIB)

PREPROCESSED = 0
CMACRO = $(MEM_MACRO)
$(OBJS) : noconfig.H
main.obj directive.obj eval.obj expand.obj support.obj system.obj mbchar.obj: \
        system.H internal.H
# To make MCPP using MCPP itself, comment out the above 4 lines and
#		uncomment the next 5 lines.
#PREPROCESSED = 1
#CMACRO =
#mcpp.H : system.H noconfig.H internal.H
#	$(BINDIR)\$(NAME) $(CPPOPTS) $(MEM_MACRO) preproc.c mcpp.H
#$(OBJS) : mcpp.H

CPPFLAGS = -DPREPROCESSED=$(PREPROCESSED)
.c.obj	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPOPTS) $(CMACRO) $<
# To make MCPP using MCPP itself, comment out the above line and
#		uncomment the next 2 lines.
#	$(NAME) $(CPPFLAGS) $< _$<
#	$(CC) $(CFLAGS) $(CMACRO) _$<

install :
	copy $(NAME) $(BINDIR)\$(NAME)

clean	:
	-del *.obj
	-del $(NAME)
	-del mcpp.H
	-del _*.c

