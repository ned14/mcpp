# makefile to compile MCPP version 2.6 for Borland C / BC make
#       2006/08 kmatsui
# You must first edit BINDIR according to your system.
# To make stand-alone-build of MCPP do:
#       make
# To make Visual-C-specific-build of MCPP do:
#       make -DCOMPILER=BORLANDC
# To re-compile MCPP using Visual-C-specific-build of MCPP do:
#       make -DCOMPILER=BORLANDC -DPREPROCESSED
# To link kmmalloc V.2.5.1 (malloc() package of kmatsui) or later do:
#       make [-DPREPROCESSED] -DKMMALLOC
# To compile MCPP with C++, rename *.c other than lib.c to *.cpp and do:
#       make -DCPLUS

NAME = mcpp

CC = bcc32
CFLAGS = $(CFLAGS) -c -a -d -f- -G -5 -DWIN32
    # -DWIN32 is nessecary to compile with bcc32
LINKFLAGS = -e$(NAME)

!if		$d( COMPILER)
CPPFLAGS = -DCOMPILER=BORLANDC
# BINDIR : Adjust to your system.
#	for Borland C V.5.5
CFLAGS = $(CFLAGS) -Oi
BINDIR = \PUBLIC\COMPILERS\BCC55\BIN
#	for Borland C V.4.0
#BINDIR = E:\BC4\BIN
!else
BINDIR = \PUBLIC\BIN
!endif

!if 	$d( CPLUS)
LANG = -P
CPP_LANG = -+
preproc = preproc.cc
!else
LANG =
CPP_LANG =
preproc = preproc.c
!endif

# '-N -D__BORLANDC__=0x0452' to work around bugs of bcc32 V.4.0
#CFLAGS = $(CFLAGS) -N -D__BORLANDC__=0x0452

!if 	$d( KMMALLOC)
MEM_MACRO = -DKMMALLOC=1 -D_MEM_DEBUG=1 -DXMALLOC=1
MEMLIB = kmmalloc_debug32.lib
!else
MEM_MACRO =
MEM_LIB =
!endif

OBJS = main.obj control.obj eval.obj expand.obj support.obj system.obj  \
        mbchar.obj lib.obj

$(NAME).exe : $(OBJS)
	$(CC) $(LINKFLAGS) $(OBJS) $(MEMLIB)

!if 	$d( PREPROCESSED)
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H  : system.H noconfig.H internal.H
	$(NAME) $(CPPFLAGS) $(CPP_LANG) $(MEM_MACRO) $(preproc) mcpp.H
$(OBJS) : mcpp.H
!else
main.obj control.obj eval.obj expand.obj support.obj system.obj mbchar.obj: \
        system.H internal.H noconfig.H
lib.obj : noconfig.H
!endif

!if 	$d( PREPROCESSED)
!if 	$d( CPLUS)
.cpp.obj:
	$(NAME) -DPREPROCESSED=1 -+ $< $(<B).i
	$(CC) $(CFLAGS) $(LANG) $(<B).i
.c.obj	:
	$(CC) $(CFLAGS) $(MEM_MACRO) $<
!else
.c.obj	:
	$(NAME) -DPREPROCESSED=1 $(CPPFLAGS) $< $(<B).i
	$(CC) $(CFLAGS) $(<B).i
!endif
!else
!if 	$d( CPLUS)
.cpp.obj:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(MEM_MACRO) $(LANG) $<
.c.obj	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(MEM_MACRO) $<
!else
.c.obj	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(MEM_MACRO) $<
!endif
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

