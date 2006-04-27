# makefile to compile MCPP version 2.* for Visual C / nmake
#		2003/11, 2004/02, 2005/03 	kmatsui
# To compile MCPP using resident cpp do
#		nmake
# To re-compile MCPP using compiled MCPP do
#		nmake PREPROCESSED=1
# To specify the preprocessor to compile MCPP
#		nmake CPP=mcpp32_std
# To generate MCPP of PRE_STANDARD mode do as
#		nmake MODE=PRE_STANDARD NAME=mcpp32_prestd
# To link kmmalloc V.2.5 (malloc() package of kmatsui) or later do
#		nmake [PREPROCESSED=1] KMMALLOC=1
# To compile MCPP with C++, rename *.c other than lib.c to *.cpp and do
#		nmake CPLUS=1
# $(NAME), $(CPP) can be specified by command-line as
#		nmake NAME=mcpp32_prestd CPP=mcpp32_std
# WARNING: If you build by "makefile project" on Visual C IDE, you must
#       specify CPP=mcpp* option, otherwise IDE will define CPP as 'cl'.

!ifndef NAME
NAME = mcpp32_std
!endif

!ifndef CPP
CPP = mcpp32_std
!endif

CFLAGS = $(CFLAGS) -Za -c	# -Zi # for debugging on Visual C / IDE
	# Don't use -Za to compile eval.c when OK_SIZE is set to TRUE
LINKFLAGS = -Fe$(NAME)		# -Zi

# to "pre-preprocess" with MCPP
!ifndef MODE
MODE = STANDARD
!endif
CPPFLAGS = -DMODE=$(MODE)

!ifndef PREPROCESSED
PREPROCESSED = 0
!endif
CPPFLAGS = $(CPPFLAGS) -DPREPROCESSED=$(PREPROCESSED)

# BINDIR : Adjust to your system.
#	for Visual C++ .net 2003
BINDIR = "$(MSVCDIR)"\bin
#	to make MCPP for LSI C-86 V.3.3		# Don't use -DPREPROCESSED
# BINDIR = \LSIC86\BIN

!ifdef CPLUS
LANG = -Tp
!else
LANG = -Tc
	# Don't use -S1 to compile eval.c when OK_SIZE is set to TRUE
!endif

CC = cl

!ifdef KMMALLOC
MEM_MACRO = -DKMMALLOC -D_MEM_DEBUG -DXMALLOC
MEMLIB = kmmalloc_debug.lib
!else
MEM_MACRO =
MEMLIB =
!endif

OBJS = main.obj control.obj eval.obj expand.obj support.obj system.obj  \
    mbchar.obj lib.obj

$(NAME).exe : $(OBJS)
	$(CC) $(LINKFLAGS) $(OBJS) $(MEMLIB)
#install :
# Visual C++ / IDE "makefile project" do not has "make install" command
	copy /b $(NAME).exe $(BINDIR)\$(NAME).exe

!if $(PREPROCESSED)
CMACRO =
# make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H internal.H
	$(CPP) $(CPPFLAGS) $(LANG) $(MEM_MACRO) preproc.c mcpp.H
$(OBJS) : mcpp.H
system.H	: noconfig.H
!else
CMACRO = $(MEM_MACRO)
main.obj control.obj eval.obj expand.obj support.obj system.obj mbchar.obj:	\
	system.H internal.H
lib.obj : system.H
system.H	: noconfig.H
!endif

!if $(PREPROCESSED)
!ifdef CPLUS
.cpp.obj:
	$(CPP) $(CPPFLAGS) $(LANG) $< $(<B).i
	$(CC) $(CFLAGS) $(CMACRO) $(LANG) $(<B).i
.c.obj	:
	$(CPP) $< $(<B).i
	$(CC) $(CFLAGS) $(CMACRO) -Tc $(<B).i
!else
.c.obj	:
	$(CPP) $(CPPFLAGS) $(LANG) $< $(<B).i
	$(CC) $(CFLAGS) $(CMACRO) $(LANG) $(<B).i
!endif
!else
!ifdef CPLUS
.cpp.obj:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CMACRO) $(LANG) $<
.c.obj	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CMACRO) -Tc $<
!else
.c.obj	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CMACRO) $(LANG) $<
!endif
!endif

clean	:
	-del *.obj
	-del *.i
	-del mcpp.H

