# makefile to compile MCPP version 2.7 and later for LCC-Win32 / LCC make
#       2008/03 kmatsui
# You must first edit BINDIR according to your system.
# To compile MCPP do:
#       make
#       make install
# To re-compile MCPP using compiled MCPP, edit this makefile and do:
#       make
#       make install
# To link malloc() package of kmatsui, edit this makefile and do:
#       make
#       make install
# To make subroutine-build of MCPP, edit this makefile and do:
#       make mcpplib
#       make mcpplib_install
# To make testmain using libmcpp do:
#       make testmain
#       make testmain_install

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
	mbchar.obj

$(NAME) : $(OBJS)
	lcclnk $(LINKFLAGS) *.obj $(MEMLIB)

PREPROCESSED = 0
CMACRO = $(MEM_MACRO)
$(OBJS) : noconfig.H
main.obj directive.obj eval.obj expand.obj support.obj system.obj mbchar.obj: \
		system.H internal.H
# To make MCPP using MCPP itself, comment out the above 5 lines and
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
	-del *.obj *.exe mcpp.H *.lib *.dll *.exp _*.c

LIBDIR = \PUBLIC\COMPILERS\LCC\lib
# For subroutine-build, uncomment the following line.
#CFLAGS = -A -DMCPP_LIB=1 -DDLL_EXPORT $(MEM_MACRO)
DLL_VER = 0

mcpplib: mcpplib_lib mcpplib_dll

mcpplib_lib:	$(OBJS)
	# lcclib does not work if the output file already exists.
	-del mcpp.lib
	lcclib -out:mcpp.lib $(OBJS)

mcpplib_dll:	$(OBJS)
	lcclnk -dll -S $(OBJS) $(MEMLIB) mcpp_lib.def

mcpplib_install:
	copy mcpp.lib $(LIBDIR)
	copy mcpp$(DLL_VER).lib $(LIBDIR)
	copy mcpp$(DLL_VER).dll $(BINDIR)

mcpplib_uninstall:
	del $(LIBDIR)\mcpp.lib $(LIBDIR)\mcpp$(DLL_VER).lib \
            $(BINDIR)\mcpp$(DLL_VER).dll

# To use mcpp as a subroutine from testmain.c, uncomment the following lines.
#NAME = testmain
# To output to memory buffer, uncomment the next line.
#CFLAGS = -A -DMCPP_LIB -DOUT2MEM -DDLL_IMPORT
#LINKFLAGS = -o $(NAME).exe $(NAME).obj mcpp$(DLL_VER).lib $(MEMLIB)
#$(NAME)	:	$(NAME).obj
#	lcclnk $(LINKFLAGS)
#$(NAME)_install	:
#	copy $(NAME).exe $(BINDIR)
#$(NAME)_uninstall	:
#	del $(BINDIR)\$(NAME).exe
