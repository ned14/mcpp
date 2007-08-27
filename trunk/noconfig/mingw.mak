# makefile to compile MCPP version 2.7 and later for MinGW / GCC / GNU make
#   2007/08   kmatsui
#
# First, you must edit GCCDIR, BINDIR, INCDIR, gcc_maj_ver and gcc_min_ver.
# To make compiler-independent-build of MCPP do:
#       make 
#       make install
# To make GCC-specific-build of MCPP:
#       make COMPILER=GNUC
#       make COMPILER=GNUC install
# To re-compile MCPP using GCC-specific-build of MCPP do:
#       make COMPILER=GNUC PREPROCESSED=1
#       make COMPILER=GNUC PREPROCESSED=1 install
# To link malloc() package of kmatsui do:
#       make [COMPILER=GNUC] [PREPROCESSED=1] MALLOC=KMMALLOC
#       make [COMPILER=GNUC] [PREPROCESSED=1] MALLOC=KMMALLOC install
# To make libmcpp (subroutine build of mcpp):
#       make MCPP_LIB=1 mcpplib
#       make MCPP_LIB=1 mcpplib_install
# To make testmain using libmcpp (add 'DLL_IMPORT=1' to link against DLL):
#       make MCPP_LIB=1 [OUT2MEM=1] testmain
#       make MCPP_LIB=1 [OUT2MEM=1] testmain_install

# COMPILER:
#   Specify whether make a compiler-independent-build or GCC-specific-build
# compiler-independent-build:   empty
# compiler-specific-build:      GNUC

# NAME: name of mcpp executable
NAME = mcpp

# CC:   name of gcc executable
#       e.g. gcc, mingw32-gcc
CC = gcc
GPP = g++
CFLAGS = -c -O2 -Wall   #-v 
CPPFLAGS =

LINKFLAGS = -o $(NAME)

ifeq    ($(COMPILER), )
# compiler-independent-build
CPPOPTS =
# BINDIR:   /usr/bin or /usr/local/bin
BINDIR = /usr/local/bin
# INCDIR:   empty
INCDIR =

else
# compiler-specific-build:  Adjust for your system

ifeq    ($(COMPILER), GNUC)
# The directory where 'gcc' (cc) command is located
GCCDIR = /mingw/bin
# set GCC version
gcc_maj_ver = 3
gcc_min_ver = 4
# INCDIR:   version specific include directory
INCDIR = /mingw/lib/gcc/mingw32/3.4.5/include
CPPOPTS = -DCOMPILER=$(COMPILER)

# BINDIR:   the directory where cc1 resides
BINDIR = /mingw/libexec/gcc/mingw32/3.4.5
cpp_call = $(BINDIR)/cc1.exe
endif
endif

LIBDIR = /usr/local/lib

ifneq   ($(MALLOC), )
ifeq    ($(MALLOC), KMMALLOC)
        LINKFLAGS += -lkmmalloc_debug
        MEM_MACRO = -D_MEM_DEBUG -DXMALLOC
endif
        MEM_MACRO += -D$(MALLOC)
else
        MEM_MACRO =
endif

OBJS = main.o directive.o eval.o expand.o support.o system.o mbchar.o lib.o

$(NAME): $(OBJS)
	$(CC) $(OBJS) $(LINKFLAGS)
ifeq    ($(COMPILER), GNUC)
	$(CC) cc1.c -o cc1.exe
endif

PREPROCESSED = 0

ifeq	($(PREPROCESSED), 1)
CMACRO = -DPREPROCESSED
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
ifeq    ($(COMPILER), GNUC)
	$(CC) -v -E -Wp,-b $(CPPFLAGS) $(CPPOPTS) $(MEM_MACRO) -o mcpp.H preproc.c
else
	@echo "Do 'sudo make COMPILER=GNUC install' prior to recompile."
	@echo "Then, do 'make COMPILER=GNUC PREPROCESSED=1'."
	@exit
endif
$(OBJS) : mcpp.H
else
CMACRO = $(CPPOPTS) $(MEM_MACRO)
$(OBJS) : noconfig.H
main.o directive.o eval.o expand.o support.o system.o mbchar.o:   \
        system.H internal.H
endif

.c.o	:
	$(CC) $(CFLAGS) $(CMACRO) $(CPPFLAGS) $<

install :
	install -s -b $(NAME).exe $(BINDIR)/$(NAME).exe
ifeq    ($(COMPILER), GNUC)
    @./set_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'    \
            '$(cpp_call)' '$(CC)' '$(GPP)' 'x' 'ln -s' '$(INCDIR)' SYS_MINGW
endif

clean	:
	-rm *.o *.so *.exe cc1.exe mcpp.H mcpp.err libmcpp.*

uninstall:
	rm -f $(BINDIR)/$(NAME).exe
ifeq    ($(COMPILER), GNUC)
	@./unset_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'   \
            '$(cpp_call)' '$(CC)' '$(GPP)' 'x.exe' 'ln -s' '$(INCDIR)' SYS_MINGW
endif

ifeq    ($(COMPILER), )
ifeq    ($(MCPP_LIB), 1)
# compiler-independent-build and MCPP_LIB is specified:
CFLAGS += -DMCPP_LIB

mcpplib :   mcpplib_a mcpplib_dll

mcpplib_a:  $(OBJS)
	ar -rv libmcpp.a $(OBJS)

# DLL
DLL_VER = 0
SOBJS = main.so directive.so eval.so expand.so support.so system.so mbchar.so lib.so
.SUFFIXES: .so
.c.so   :
	$(CC) $(CFLAGS) $(MEM_MACRO) -DDLL_EXPORT -c -o$*.so $*.c
        # -fPIC is not necessary for MinGW
mcpplib_dll: $(SOBJS)
	$(CC) -shared $(SOBJS) -olibmcpp-$(DLL_VER).dll -Wl,--enable-auto-image-base,--out-implib,libmcpp.dll.a

mcpplib_install:
	cp libmcpp.a libmcpp.dll.a $(LIBDIR)
	cp libmcpp-$(DLL_VER).dll $(BINDIR)
	ranlib $(LIBDIR)/libmcpp.a
mcpplib_uninstall:
	rm -f $(LIBDIR)/libmcpp.a $(LIBDIR)/libmcpp.dll.a $(BINDIR)/libmcpp-$(DLL_VER).dll

# use mcpp as a subroutine from testmain.c
NAME = testmain
ifeq    ($(OUT2MEM), 1)
# output to memory buffer
CFLAGS += -DOUT2MEM
endif
LINKFLAGS = $(NAME).o -o$(NAME).exe
ifeq    ($(DLL_IMPORT), 1)
LINKFLAGS += $(LIBDIR)/libmcpp.dll.a
CFLAGS += -DDLL_IMPORT
else
LINKFLAGS += $(LIBDIR)/libmcpp.a
endif
ifeq    ($(MALLOC), KMMALLOC)
    LINKFLAGS += -lkmmalloc_debug
endif
$(NAME) :   $(NAME).o
	$(CC) $(LINKFLAGS)
$(NAME)_install :
	install -s $(NAME).exe $(BINDIR)/$(NAME).exe
$(NAME)_uninstall   :
	rm -f $(BINDIR)/$(NAME).exe

endif
endif
