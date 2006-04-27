# makefile to compile MCPP version 2.* for Turbo C, Borland C / BC make
#		1998/08, 2003/11, 2004/02, 2005/03 	kmatsui
# To compile MCPP using resident cpp do
#		make
# To re-compile MCPP using compiled MCPP do
#		make -DPREPROCESSED
# To specify the preprocessor to compile MCPP
#		make -DCPP=mcpp32_std
# To generate MCPP of PRE_STANDARD mode do as
#		make -DMODE=PRE_STANDARD -DNAME=mcpp32_prestd
# To link malloc() package of kmatsui do
#		make [-DPREPROCESSED] -DKMMALLOC
# To compile MCPP with C++, rename *.c other than lib.c and preproc.c to *.cpp,
#	then do
#		make -DCPLUS
# To compile with bcc32
#		make -DWIN32
# To compile MCPP for MS-DOS / Borland C by WIN32 / Borland C
#		make -DWIN32

!if 	!$d( NAME)
!if 	$d( WIN32)
NAME = mcpp32_std
!else
NAME = mcpp_std
!endif
!endif

!if 	!$d( CPP)
!if 	$d( WIN32)
CPP = mcpp32_std
!else
CPP = mcpp_std
!endif
!endif

CFLAGS = $(CFLAGS) -c -a -d -f- -G # -5
	# Don't use -A to compile eval.c when OK_SIZE is set to TRUE
LINKFLAGS = -e$(NAME)

# BINDIR : Adjust to your system.
#	for Borland C V.5.5
CFLAGS = $(CFLAGS) -Oi
BINDIR = \BCC55\BIN
#	for Borland C V.4.0
# BINDIR = \BC4\BIN
#	to make mcpp for LSI C-86 V.3.3		# Don't use -DPREPROCESSED
# BINDIR = \LSIC86\BIN

!if 	$d( CPLUS)
LANG = -+
!else
LANG = -S1
	# Don't use -S1 to compile eval.c when OK_SIZE is set to TRUE
!endif

!if 	$d( WIN32)
# '-N -D__BORLANDC__=0x0452' to work around bugs of bcc32 V.4.0
CC = bcc32
# CFLAGS = $(CFLAGS) -N -D__BORLANDC__=0x0452
MM = 32
MEM =
!else
CC = bcc
# CC = tcc
MM = l
MEM = -m$(MM)
!endif

!if		$d( MODE)
CPPFLAGS = -DMODE=$(MODE)
!else
CPPFLAGS =
!endif

!if 	$d( KMMALLOC)
MEM_MACRO = -DKMMALLOC=1 -D_MEM_DEBUG=1 -DXMALLOC=1
!if 	$d( WIN32)
MEMLIB = kmmalloc_debug32.lib
!else
MEMLIB = kmma_d_$(MM).lib
!endif
!else
MEM_MACRO =
MEMLIB =
!endif

OBJS = main.obj control.obj eval.obj expand.obj support.obj system.obj	\
	mbchar.obj lib.obj

$(NAME).exe : $(OBJS)
	$(CC) $(MEM) $(LINKFLAGS) $(OBJS) $(MEMLIB)

!if 	$d( PREPROCESSED)
CMACRO =
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
	$(CPP) $(LANG) $(CPPFLAGS) $(MEM_MACRO) $(MEM) preproc.c mcpp.H
$(OBJS) : mcpp.H
!else
CMACRO = $(MEM_MACRO)
main.obj control.obj eval.obj expand.obj support.obj system.obj mbchar.obj:	\
	system.H noconfig.H internal.H
lib.obj : system.H noconfig.H
!endif

!if 	$d( PREPROCESSED)
!if 	$d( CPLUS)
.cpp.obj:
	$(CPP) $(LANG) $(MEM) -DPREPROCESSED=1 $< $(<B).i
	$(CC) $(CFLAGS) $(MEM) $(<B).i
.c.obj	:
	$(CC) $(CFLAGS) $(MEM) $(CMACRO) $<
!else
.c.obj	:
	$(CPP) $(LANG) $(MEM) -DPREPROCESSED=1 $< $(<B).i
	$(CC) $(CFLAGS) $(MEM) $(<B).i
!endif
!else
.cpp.obj:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(MEM) $(CMACRO) $<
.c.obj	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(MEM) $(CMACRO) $<
!endif

install :
	copy /b $(NAME).exe $(BINDIR)\$(NAME).exe

clean	:
	-del *.obj
	-del $(NAME).exe
	-del *.bak
	-del mcpp.H
	-del *.i
	-del *.tds

