# makefile to compile MCPP version 2.6.3 for Linux / GCC / GNU make
#       2007/03 kmatsui
#
# First, you must edit GCCDIR, BINDIR, INCDIR, gcc_maj_ver and gcc_min_ver.
# To make compiler-independent-build of MCPP:
#       make
#	    sudo make install
# To make GCC-specific-build of MCPP:
#       make COMPILER=GNUC
#       sudo make COMPILER=GNUC install
# To re-compile MCPP using GCC-specific-build of MCPP do:
#       make COMPILER=GNUC PREPROCESSED=1
#       sudo make COMPILER=GNUC PREPROCESSED=1 install
# To link malloc() package of kmatsui do:
#       make [COMPILER=GNUC] [PREPROCESSED=1] MALLOC=KMMALLOC
#       sudo make [COMPILER=GNUC] [PREPROCESSED=1] MALLOC=KMMALLOC install
# To make libmcpp (subroutine build of mcpp):
#       make MCPP_LIB=1 mcpplib
#    	sudo make MCPP_LIB=1 mcpplib_install
# To make testmain using libmcpp:
#		make MCPP_LIB=1 [OUT2MEM=1] testmain
#		sudo make MCPP_LIB=1 [OUT2MEM=1] testmain_install
# To compile cpp with C++, rename *.c other than lib.c to *.cc,
#   and do:
#       make CPLUS=1
#       sudo make CPLUS=1 install

# COMPILER:
#   Specify whether make a compiler-independent-build or GCC-specific-build
# compiler-independent-build:   empty
# compiler-specific-build:      GNUC

# NAME: name of mcpp executable
NAME = mcpp

# CC:   name of gcc executable
#       e.g. cc, gcc, gcc-2.95.3, i686-pc-linux-gnu-gcc-4.1.1
CC = gcc
GPP = g++
CFLAGS = -c -O2 -Wall   # -v
#CFLAGS += -fstack-protector-all     # for gcc 4.1 or later
CPPFLAGS =

LINKFLAGS = -o $(NAME)
#LINKFLAGS += -fstack-protector-all  # for gcc 4.1 or later

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
CPPOPTS = -DCOMPILER=$(COMPILER)
# BINDIR:   the directory where cpp0 or cc1 resides
#BINDIR = /usr/lib/gcc-lib/i386-redhat-linux/2.95.3
#BINDIR = /usr/local/gcc-3.2/lib/gcc-lib/i686-pc-linux-gnu/3.2
#BINDIR = /usr/lib/gcc-lib/i386-vine-linux/3.3.6
BINDIR = /usr/local/libexec/gcc/i686-pc-linux-gnu/4.1.1
# INCDIR:   version specific include directory
#INCDIR = /usr/lib/gcc-lib/i386-redhat-linux/2.95.3/include
#INCDIR = /usr/local/gcc-3.2/lib/gcc-lib/i686-pc-linux-gnu/3.2/include
#INCDIR = /usr/lib/gcc-lib/i386-vine-linux/3.3.6/include
INCDIR = /usr/local/lib/gcc/i686-pc-linux-gnu/4.1.1/include
# set GCC version
gcc_maj_ver = 4
gcc_min_ver = 1
ifeq ($(gcc_maj_ver), 2)
cpp_call = $(BINDIR)/cpp0
else
cpp_call = $(BINDIR)/cc1
endif
endif
endif

# The directory where 'gcc' (cc) command is located
#GCCDIR = /usr/bin
GCCDIR = /usr/local/bin

CPLUS =
ifeq	($(CPLUS), 1)
	GCC = $(GPP)
	preproc = preproc.cc
else
	GCC = $(CC)
	preproc = preproc.c
endif

ifneq	($(MALLOC), )
ifeq	($(MALLOC), KMMALLOC)
	LINKFLAGS += -lkmmalloc_debug
	MEM_MACRO = -D_MEM_DEBUG -DXMALLOC
endif
	MEM_MACRO += -D$(MALLOC)
else
	MEM_MACRO =
endif

OBJS = main.o directive.o eval.o expand.o support.o system.o mbchar.o lib.o

all	:	$(NAME)
$(NAME): $(OBJS)
	$(GCC) $(OBJS) $(LINKFLAGS)

PREPROCESSED = 0

ifeq	($(PREPROCESSED), 1)
CMACRO = -DPREPROCESSED
#CPPFLAGS = -Wp,-v,-Q,-W3
    # for MCPP to output a bit verbose diagnosis to "mcpp.err"
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
ifeq    ($(COMPILER), GNUC)
	$(GCC) -E -Wp,-b  $(CPPFLAGS) $(CPPOPTS) $(MEM_MACRO) -o mcpp.H $(preproc)
else
	@echo "Do 'sudo make COMPILER=GNUC install' prior to recompile."
	@echo "Then, do 'make COMPILER=GNUC PREPROCESSED=1'."
	@exit
endif
$(OBJS) : mcpp.H
else
CMACRO = $(MEM_MACRO) $(CPPOPTS)
$(OBJS) : noconfig.H
main.o directive.o eval.o expand.o support.o system.o mbchar.o:   \
        system.H internal.H
endif

ifeq	($(CPLUS), 1)
.cc.o	:
	$(GPP) $(CFLAGS) $(CMACRO) $(CPPFLAGS) $<
.c.o	:
	$(CC) $(CFLAGS) $(CMACRO) $(CPPFLAGS) $<
else
.c.o	:
	$(CC) $(CFLAGS) $(CMACRO) $(CPPFLAGS) $<
endif

install :
	install -s $(NAME) $(BINDIR)/$(NAME)
ifeq    ($(COMPILER), GNUC)
	@./set_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)' \
            '$(cpp_call)' '$(CC)' '$(GPP)' 'x' 'ln -s' '$(INCDIR)'
endif

clean	:
	-rm *.o *.so $(NAME) mcpp.H mcpp.err libmcpp.*

uninstall:
	rm -f $(BINDIR)/$(NAME)
ifeq    ($(COMPILER), GNUC)
	@./unset_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'   \
            '$(cpp_call)' '$(CC)' '$(GPP)' 'x' 'ln -s' '$(INCDIR)'
endif

ifeq    ($(COMPILER), )
ifeq    ($(MCPP_LIB), 1)
# compiler-independent-build and MCPP_LIB=1
CFLAGS += -DMCPP_LIB
LIBDIR = /usr/local/lib

mcpplib :   mcpplib_a mcpplib_so

mcpplib_a:	$(OBJS)
	ar -rv libmcpp.a $(OBJS)

# shared library
CUR = 0
REV = 0
AGE = 0
SHLIB_VER = $(CUR).$(REV).$(AGE)
SOBJS = main.so directive.so eval.so expand.so support.so system.so mbchar.so lib.so
.SUFFIXES: .so
.c.so	:
	$(GCC) $(CFLAGS) $(MEM_MACRO) -c -fpic -o$*.so $*.c
mcpplib_so: $(SOBJS)
	ld -shared -olibmcpp.so.$(SHLIB_VER) $(SOBJS)
	chmod a+x libmcpp.so.$(SHLIB_VER)

mcpplib_install:
	cp libmcpp.a libmcpp.so.$(SHLIB_VER) $(LIBDIR)
	ranlib $(LIBDIR)/libmcpp.a
	ln -sf libmcpp.so.$(SHLIB_VER) $(LIBDIR)/libmcpp.so
	ln -sf libmcpp.so.$(SHLIB_VER) $(LIBDIR)/libmcpp.so.$(CUR)
    # You should do 'ldconfig' as a root after install.
mcpplib_uninstall:
	rm -f $(LIBDIR)/libmcpp.a $(LIBDIR)/libmcpp.so.$(SHLIB_VER) $(LIBDIR)/libmcpp.so.$(CUR) $(LIBDIR)/libmcpp.so 

# use mcpp as a subroutine from testmain.c
NAME = testmain
ifeq    ($(OUT2MEM), 1)
# output to memory buffer
CFLAGS += -DOUT2MEM
endif
LINKFLAGS = $(NAME).o -o$(NAME) -lmcpp
ifeq	($(MALLOC), KMMALLOC)
	LINKFLAGS += -lkmmalloc_debug
endif
#LINKFLAGS += -fstack-protector-all
$(NAME)	:	$(NAME).o
	$(GCC) $(LINKFLAGS)
$(NAME)_install	:
	install -s $(NAME) $(BINDIR)/$(NAME)
$(NAME)_uninstall	:
	rm -f $(BINDIR)/$(NAME)

endif
endif

