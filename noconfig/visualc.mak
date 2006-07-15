# makefile to compile MCPP version 2.6 for Visual C / nmake
#		2006/05	kmatsui
# You must first edit BINDIR according to your system.
# To make stand-alone-build of MCPP do:
#		nmake
# To make Visual-C-specific-build of MCPP do:
#		nmake COMPILER=MSC
# To re-compile MCPP using Visual-C-specific-build of MCPP do:
#		nmake COMPILER=MSC PREPROCESSED=1
# To link kmmalloc V.2.5.1 (malloc() package of kmatsui) or later do:
#   (Note: Visual C 2005 cannot coexist with kmmalloc)
#		nmake [PREPROCESSED=1] KMMALLOC=1
# To compile MCPP with C++, rename *.c other than lib.c to *.cpp and do:
#		nmake CPLUS=1

NAME = mcpp

CC = cl
CFLAGS = $(CFLAGS) -Za -c   # -Zi
	# Add -Zi for debugging on Visual C / IDE
LINKFLAGS = -Fe$(NAME)  # -Zi
CPPFLAGS = $(CPPFLAGS) -D_CRT_SECURE_NO_DEPRECATE
	# -D_CRT_SECURE_NO_DEPRECATE for Visual C 2005

!if "$(COMPILER)"=="MSC"
CPPFLAGS = $(CPPFLAGS) -DCOMPILER=MSC
# BINDIR : Adjust to your system.
#	for Visual C 2003
#BINDIR = "$(MSVCDIR)"\bin
#	for Visual C 2005
BINDIR = "$(VCINSTALLDIR)"\bin
!else
# stand-alone-build: use compiler-independent directory
BINDIR = \PUBLIC\bin
!endif

!ifdef CPLUS
LANG = -TP
preproc = preproc.cpp
!else
LANG = -TC
preproc = preproc.c
!endif

!ifdef KMMALLOC
MEM_MACRO = -DKMMALLOC -D_MEM_DEBUG -DXMALLOC
MEMLIB = kmmalloc_debug.lib
!else
MEM_MACRO =
MEMLIB =
!endif

OBJS = main.obj control.obj eval.obj expand.obj support.obj system.obj	\
	mbchar.obj lib.obj

$(NAME).exe : $(OBJS)
	$(CC) $(LINKFLAGS) $(OBJS) $(MEMLIB)
#install :
# Visual C++ / IDE "makefile project" does not have "make install" command
	copy /b $(NAME).exe $(BINDIR)\$(NAME).exe

!ifdef PREPROCESSED
# make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H internal.H
	$(NAME) $(CPPFLAGS) $(LANG) $(MEM_MACRO) $(preproc) mcpp.H
$(OBJS) : mcpp.H
system.H	: noconfig.H
!else
$(OBJS) : noconfig.H
main.obj control.obj eval.obj expand.obj support.obj system.obj mbchar.obj:	\
	    system.H internal.H
!endif

!ifdef PREPROCESSED
!ifdef CPLUS
.cpp.obj:
	$(NAME) -DPREPROCESSED $(LANG) $< $(<B).i
	$(CC) $(CFLAGS) $(LANG) $(<B).i
.c.obj	:
	$(NAME) $(CPPFLAGS) $< $(<B).i
	$(CC) $(CFLAGS) -TC $(<B).i
!else
.c.obj	:
	$(NAME) -DPREPROCESSED $(CPPFLAGS) $< $(<B).i
	$(CC) $(CFLAGS) $(LANG) $(<B).i
!endif
!else
!ifdef CPLUS
.cpp.obj:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(MEM_MACRO) $(LANG) $<
.c.obj	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(MEM_MACRO) -TC $<
!else
.c.obj	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(MEM_MACRO) $(LANG) $<
!endif
!endif

clean	:
	-del *.obj
	-del *.i
	-del mcpp.H
	-del $(NAME).exe
