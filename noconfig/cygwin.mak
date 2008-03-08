# makefile to compile MCPP version 2.7 and later for CygWIN / GCC / GNU make
# 2008/03   kmatsui
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
#       e.g. cc, gcc, gcc-2.95.3, i686-pc-linux-gnu-gcc-3.4.3
CC = gcc
CXX = g++
CFLAGS = -c -O2 -Wall   #-v
CPPFLAGS =
#CPPFLAGS = -Wp,-vQW3
    # for MCPP to output a bit verbose diagnosis to "mcpp.err"

LINKFLAGS = -o $(NAME)

ifeq    ($(COMPILER), )
# compiler-independent-build
CPPOPTS =
# BINDIR:   directory to install mcpp: /usr/bin or /usr/local/bin
BINDIR = /usr/local/bin
# INCDIR:   empty
INCDIR =

else
# compiler-specific-build:  Adjust for your system

ifeq    ($(COMPILER), GNUC)
# The directory where 'gcc' (cc) command is located
GCCDIR = /usr/bin
#GCCDIR = /usr/local/bin
# set GCC version
gcc_maj_ver = 3
gcc_min_ver = 4
# INCDIR:   version specific include directory
#INCDIR = /usr/lib/gcc-lib/i686-pc-cygwin/2.95.3-5/include
INCDIR = /usr/lib/gcc/i686-pc-cygwin/3.4.4/include
CPPOPTS = -DCOMPILER=$(COMPILER)

# BINDIR:   the directory where cpp0 or cc1 resides
#BINDIR = /usr/lib/gcc-lib/i686-pc-cygwin/2.95.3-5
BINDIR = /usr/lib/gcc/i686-pc-cygwin/3.4.4
target = i686-pc-cygwin
ifeq ($(gcc_maj_ver), 2)
cpp_call = $(BINDIR)/cpp0.exe
else
cpp_call = $(BINDIR)/cc1.exe
endif
endif
endif

OBJS = main.o directive.o eval.o expand.o support.o system.o mbchar.o

$(NAME): $(OBJS)
	$(CC) $(OBJS) $(LINKFLAGS)

PREPROCESSED = 0

ifeq	($(PREPROCESSED), 1)
CMACRO = -DPREPROCESSED
#CPPFLAGS += -std=c99
    # '-std=c99' option is sometimes nessecary to work around 
    # the confusion of system header files
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
ifeq    ($(COMPILER), GNUC)
	$(CC) -v -E -Wp,-b $(CPPFLAGS) $(CPPOPTS) -o mcpp.H preproc.c
else
	@echo "Do 'sudo make COMPILER=GNUC install' prior to recompile."
	@echo "Then, do 'make COMPILER=GNUC PREPROCESSED=1'."
	@exit
endif
$(OBJS) : mcpp.H
else
CMACRO = $(CPPOPTS)
$(OBJS) : noconfig.H
main.o directive.o eval.o expand.o support.o system.o mbchar.o:   \
        system.H internal.H
endif

.c.o	:
	$(CC) $(CFLAGS) $(CMACRO) $(CPPFLAGS) $<

install :
	install -s -b $(NAME).exe $(BINDIR)/$(NAME).exe
ifeq    ($(COMPILER), GNUC)
    @./set_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'        \
            '$(cpp_call)' '$(CC)' '$(CXX)' 'x$(CPPFLAGS)' 'x' 'ln -s'   \
            '$(INCDIR)' SYS_CYGWIN
endif

clean	:
	-rm *.o *.so *.exe mcpp.H mcpp.err libmcpp.* cygmcpp.*

uninstall:
	rm -f $(BINDIR)/$(NAME).exe
ifeq    ($(COMPILER), GNUC)
	./unset_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'   \
        '$(cpp_call)' '$(CC)' '$(CXX)' 'x.exe' 'ln -s' '$(INCDIR)' SYS_CYGWIN
endif

ifeq    ($(COMPILER), )
ifeq    ($(MCPP_LIB), 1)
# compiler-independent-build and MCPP_LIB=1
CFLAGS += -DMCPP_LIB
LIBDIR=/usr/local/lib

mcpplib :   mcpplib_a mcpplib_dll

mcpplib_a:  $(OBJS)
	ar -rv libmcpp.a $(OBJS)

# DLL
DLL_VER = 0
SOBJS = main.so directive.so eval.so expand.so support.so system.so mbchar.so
.SUFFIXES: .so
.c.so   :
	$(CC) $(CFLAGS) $(MEM_MACRO) -c -DPIC -o $*.so $*.c
mcpplib_dll: $(SOBJS)
	$(CC) -shared -o cygmcpp-$(DLL_VER).dll $(SOBJS) -Wl,--enable-auto-image-base,--out-implib,libmcpp.dll.a

mcpplib_install:
	cp libmcpp.a libmcpp.dll.a $(LIBDIR)
	cp cygmcpp-$(DLL_VER).dll $(BINDIR)
	ranlib $(LIBDIR)/libmcpp.a
mcpplib_uninstall:
	rm -f $(LIBDIR)/libmcpp.a $(LIBDIR)/libmcpp.dll.a $(BINDIR)/cygmcpp-$(DLL_VER).dll

# use mcpp as a subroutine from testmain.c
NAME = testmain
ifeq    ($(OUT2MEM), 1)
# output to memory buffer
CFLAGS += -DOUT2MEM
endif
LINKFLAGS = $(NAME).o -o $(NAME).exe
ifeq    ($(DLL_IMPORT), 1)
LINKFLAGS += $(LIBDIR)/libmcpp.dll.a
CFLAGS += -DDLL_IMPORT
else
LINKFLAGS += $(LIBDIR)/libmcpp.a
endif
$(NAME) :   $(NAME).o
	$(CC) $(LINKFLAGS)
$(NAME)_install :
	install -s $(NAME).exe $(BINDIR)/$(NAME).exe
$(NAME)_uninstall   :
	rm -f $(BINDIR)/$(NAME).exe

endif
endif

