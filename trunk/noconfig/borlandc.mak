# makefile to compile MCPP version 2.6.3 for Borland C / BC make
#		2007/03 kmatsui
# You must first edit BINDIR, LIBDIR and LINKER according to your system.
# To make compiler-independent-build of MCPP do:
#		make
#		make install
# To make Borland-C-specific-build of MCPP do:
#		make -DCOMPILER=BORLANDC
#		make -DCOMPILER=BORLANDC install
# To re-compile MCPP using Borland-C-specific-build of MCPP do:
#		make -DCOMPILER=BORLANDC -DPREPROCESSED
#		make -DCOMPILER=BORLANDC -DPREPROCESSED install
# To link kmmalloc V.2.5.1 (malloc() package of kmatsui) or later do:
#		make [-DPREPROCESSED] -DKMMALLOC
#		make [-DPREPROCESSED] -DKMMALLOC install
# To make mcpp.lib (subroutine-build of mcpp) do:
#       make -DMCPP_LIB mcpplib
#       make -DMCPP_LIB mcpplib_install
# To make testmain.c (sample to use mcpp.lib) against mcpp.lib do
#   (add '-DDLL_IMPORT' to link against the DLL):
#       make -DMCPP_LIB [-DOUT2MEM] testmain
#       make -DMCPP_LIB [-DOUT2MEM] testmain_install
# To compile MCPP with C++, rename *.c other than lib.c to *.cpp and do:
#		make -DCPLUS
#		make -DCPLUS install

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
BINDIR = \PUB\COMPILERS\BCC55\BIN
#	for Borland C V.4.0
#BINDIR = E:\BC4\BIN
!else
BINDIR = \PUB\BIN
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

OBJS = main.obj directive.obj eval.obj expand.obj support.obj system.obj  \
		mbchar.obj lib.obj

$(NAME).exe : $(OBJS)
	$(CC) $(LINKFLAGS) $(OBJS) $(MEMLIB)

!if 	$d( PREPROCESSED)
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
	$(NAME) $(CPPFLAGS) $(CPP_LANG) $(MEM_MACRO) $(preproc) mcpp.H
$(OBJS) : mcpp.H
!else
main.obj directive.obj eval.obj expand.obj support.obj system.obj mbchar.obj: \
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
	-del *.obj *.exe *.bak mcpp.H *.i *.tds *.lib *.dll mcpp$(DLL_VER).* *.so

!if 	$d( MCPP_LIB)
# subroutine-build
CFLAGS = $(CFLAGS) -DMCPP_LIB=1
LIBDIR = \PUB\COMPILERS\BCC55\LIB
#LINKER = tlink32   # BCC40
LINKER = ilink32   # BCC55
ADD_OBJS = +main +directive +eval +expand +support +system +mbchar +lib

mcpplib:	mcpplib_lib mcpplib_dll

mcpplib_lib:	$(OBJS)
	tlib mcpp.lib $(ADD_OBJS)

# DLL
DLL_VER = 0
SOBJS = main.so directive.so eval.so expand.so support.so system.so mbchar.so lib.so
.SUFFIXES: .so
.c.so   :
    $(CC) $(CFLAGS) $(MEM_MACRO) -DDLL_EXPORT=1 -o$*.so $<
mcpplib_dll: $(SOBJS)
	$(LINKER) -Tpd c0d32.obj $(SOBJS), mcpp$(DLL_VER).dll, , \
            cw32.lib import32.lib $(MEMLIB)
	implib mcpp$(DLL_VER).lib mcpp$(DLL_VER).dll

mcpplib_install:
	copy mcpp.lib $(LIBDIR)
	copy mcpp$(DLL_VER).lib $(LIBDIR)
	copy mcpp$(DLL_VER).dll $(BINDIR)

mcpplib_uninstall:
	del $(LIBDIR)\mcpp.lib $(LIBDIR)\mcpp$(DLL_VER).lib \
            $(BINDIR)\mcpp$(DLL_VER).dll

# use mcpp as a subroutine from testmain.c
NAME = testmain
!if	$d( DLL_IMPORT)
CFLAGS = $(CFLAGS) -DDLL_IMPORT=1
LINKLIB = mcpp$(DLL_VER).lib
!else
LINKLIB = mcpp.lib
!endif
!if    $d( OUT2MEM)
# output to memory buffer
CFLAGS = $(CFLAGS) -DOUT2MEM=1
!endif
LINKFLAGS = $(NAME).obj -e$(NAME).exe $(LINKLIB)
$(NAME)	:	$(NAME).obj
	$(CC) $(LINKFLAGS)
$(NAME)_install	:
	copy $(NAME).exe $(BINDIR)
$(NAME)_uninstall	:
	del $(BINDIR)\$(NAME).exe

!endif
