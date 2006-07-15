dnl configure.ac for MCPP   2003/11, 2006/07  kmatsui
dnl Process this file with autoconf to produce a configure script.

dnl WARNING: This script does not assume cross-compiling.
dnl To cross-compile you must edit here and there of this file.

AC_INIT( mcpp, 2.6, kmatsui@t3.rim.or.jp)
AC_CONFIG_SRCDIR( src/main.c)
AM_CONFIG_HEADER( src/config.h)
AC_CONFIG_AUX_DIR( config)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE( no-dependencies)

dnl Checks for programs.

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_EGREP

dnl WARNING: Configure must check the characteristics of target compiler.
dnl MCPP may modify the behavior of the target compiler-system.
dnl You must uninstall MCPP before re-configuring.

AC_MSG_CHECKING( [whether the cpp is target preprocessor])
if test `echo '' | ${CPP} -v - 2>&1 | grep 'MCPP' > /dev/null; echo $?` = 0;
        then
    AC_MSG_ERROR( Do 'make uninstall' before configuring again.)
fi
AC_MSG_RESULT( yes)

dnl Define host system and target system

case $host_os in
linux*)
    AC_DEFINE( [HOST_SYSTEM], [SYS_LINUX], [Define the host system.])
    ;;
freebsd*)
    AC_DEFINE( [HOST_SYSTEM], [SYS_FREEBSD])
    ;;
cygwin* | mingw*)
    AC_DEFINE( [HOST_SYSTEM], [SYS_CYGWIN])
    ac_c_define_HOST_SYSTEM=[SYS_CYGWIN]
    ;;
*)
    AC_MSG_WARN( [Unsupported host OS, assuming to be an UNIX variant])
    AC_DEFINE( [HOST_SYSTEM], [SYS_UNIX])
    ;;
esac

case $target_os in
linux*)
    AC_DEFINE( [SYSTEM], [SYS_LINUX], [Define the target system.])
    ;;
freebsd*)
    AC_DEFINE( [SYSTEM], [SYS_FREEBSD])
    ;;
cygwin* | mingw*)
    AC_DEFINE( [SYSTEM], [SYS_CYGWIN])
    ;;
*)
    AC_MSG_WARN( Unsupported target OS, assuming to be an UNIX variant)
    AC_DEFINE( [SYSTEM], [SYS_UNIX])
    ;;
esac

dnl Checks for header files.

AC_CHECK_HEADERS( [unistd.h, stdint.h, inttypes.h])

dnl Checks for typedefs, and compiler characteristics.

AC_CHECK_TYPES( [intmax_t, long long])

dnl Check for library functions.

AC_CHECK_FUNCS( [getopt stpcpy])

dnl Checks for some system characteristics.

AC_CACHE_CHECK( if the cases of filename are folded,
    ac_cv_fname_fold,
    [touch hJkL
    [ac_cv_fname_fold=no]
    AC_RUN_IFELSE( [[
#include    <stdio.h>
#include    <errno.h>
    void exit();
int main( void)
{
    if (fopen( "HjKl", "r") == NULL)
        exit( errno);
    else
        exit( 0);
} ]],
        [ac_cv_fname_fold=yes],
        [],
        AC_MSG_WARN( cannot run on cross-compiling so assumes that the cases of filename are not folded.)
    )
    rm hJkL],
)
if test x${ac_cv_fname_fold} = xyes; then
    AC_DEFINE( [FNAME_FOLD], 1,
        [Define if the cases of file name are folded.])
fi

dnl Checks for some host compiler characteristics.

AC_DEFINE_UNQUOTED( [OBJEXT], ["${ac_cv_objext}"],
    [Define the suffix of object file.])

dnl Check printf length modifier of the longest integer.

# AC_CHECK_MOD_LL( modifier, additional include header)
# Try the printf length modifier for the longest integer.
AC_DEFUN( [AC_CHECK_MOD_LL], [
    AC_CACHE_CHECK( [printf length modifier for the longest integer],
        [ac_cv_c_printf_modifier],
        AC_RUN_IFELSE( [[
#include    <stdio.h>
#include    <assert.h>
    int     strcmp();
    void    exit();
$3
int main( void)
{   char    buf@<:@ 20@:>@;
    $1  num = 1234567890123456789;
    sprintf( buf, "%$2d", num);
    assert( strcmp( buf, "1234567890123456789") == 0);
    exit( 0);
} ]],
            [ac_cv_c_printf_modifier=$2],
            ,
            AC_MSG_WARN( cannot run on cross-compiling so assumes the length modifier of long long is "%ll")
            AC_DEFINE( [LL_FORM], "ll",
                [Define printf length modifier for the longest integer.])
            [ac_cv_c_printf_modifier=ll]
        )
    )
] )

if test x${ac_cv_type_intmax_t} = xyes; then
    AC_CHECK_MOD_LL( [intmax_t], [j], [#include <stdint.h>])
    if test x$ac_c_define_HOST_SYSTEM = xSYS_CYGWIN; then
        sleep 1         # wait for cleanup
    fi
fi

if test x${ac_cv_type_long_long} = xyes; then
    for modifier in ll q L
    do
        AC_CHECK_MOD_LL( [long long], ${modifier})
        if test x${ac_cv_c_printf_modifier+set} = xset; then
            break
        fi
        if test x$ac_c_define_HOST_SYSTEM = xSYS_CYGWIN; then
            sleep 1         # wait for cleanup
        fi
    done
fi

if test x${ac_cv_c_printf_modifier+set} = xset; then
    AC_DEFINE_UNQUOTED( [LL_FORM], "${ac_cv_c_printf_modifier}",
        [Define printf length modifier for the longest integer.])
fi

dnl Define target specific predefined macros.

AC_MSG_NOTICE( [setting predefined macros for target cpu])

case $target_cpu in
i?86*)
    Target_Cpu=i386
    ;;
*)
    Target_Cpu=$target_cpu
    ;;
esac

if test x$mcpp_ac_c_define_CPU_STD != x__${Target_Cpu}; then
    AC_DEFINE_UNQUOTED( [CPU_STD1], "__${Target_Cpu}",
        [Define the cpu-specific-predefined-macro.])
fi
if test x$mcpp_ac_c_define_CPU_STD != x__${Target_Cpu}__; then
    AC_DEFINE_UNQUOTED( [CPU_STD2], "__${Target_Cpu}__",
        [Define the cpu-specific-predefined-macro.])
fi
if test x$mcpp_ac_c_define_CPU_OLD != x${Target_Cpu}; then
    AC_DEFINE_UNQUOTED( [CPU_OLD], "${Target_Cpu}",
        [Define the cpu-specific-predefined-macro.])
fi

fbsd=`expr ${target_os} : '\(freebsd\)'`
if test x${fbsd} = xfreebsd &&  \
        test x$mcpp_ac_c_define_SYSTEM_SP_STD != x__FreeBSD__; then
    fbsd_ver=`echo '__FreeBSD__' | ${CPP} - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ .*//'`
    AC_DEFINE_UNQUOTED( [SYSTEM_EXT_VAL], "${fbsd_ver}",
        [Define the version of FreeBSD.])
fi

if test x${ac_cv_c_compiler_gnu} = xyes; then

    AC_DEFINE( [HOST_COMPILER], [GNUC], [Define the host compiler.])

    AC_DEFINE( [HOST_CMP_NAME], "GCC", [Define the host compiler name.])

    AC_MSG_CHECKING( for value of __GNUC__)
    gcc_maj_ver=`echo '__GNUC__' | ${CPP} - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ *//'`
    AC_MSG_RESULT( ${gcc_maj_ver})
    AC_DEFINE_UNQUOTED( [GCC_MAJOR_VERSION], "${gcc_maj_ver}",
        [Define gcc major version.])
    AC_MSG_CHECKING( for value of __GNUC_MINOR__)
    gcc_min_ver=`echo __GNUC_MINOR__ | ${CPP} - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ *//'`
    AC_MSG_RESULT( ${gcc_min_ver})
    AC_DEFINE_UNQUOTED( [GCC_MINOR_VERSION], "${gcc_min_ver}",
        [Define gcc minor version.])

    AC_CACHE_CHECK( if the library is glibc,
        gcc_cv_glibc,
        [AC_PREPROC_IFELSE( [
#include <features.h>
#if ! (defined __GLIBC__ || defined __GNU_LIBRARY)
#error Not glibc
#endif],
            [gcc_cv_glibc=yes],
            [gcc_cv_glibc=no]
        )]
    )
    if test x${gcc_cv_glibc} = xyes; then
        AC_DEFINE( [LIB_IS_GLIBC], 1, [Define if the library is glibc.])
    fi

else    # not gcc

# mcpp_AC_C_DEFINE( SYSTEM | CPU | COMPILER, STD | OLD, ${enableval})
# Specify the target-specific predefined macro.
AC_DEFUN( [mcpp_AC_C_DEFINE], [
    name=`echo $3 | sed 's/=.*//'`
    mcpp_macro=$1_SP_$2
    stripped_mcpp_macro=`echo $mcpp_macro | sed 's/STD//'`
    if test x${stripped_mcpp_macro} != x${mcpp_macro}; then
        stripped_name=`echo ${name} | sed 's/^_//'`
        if test x${stripped_name} = x${name}; then
            AC_MSG_ERROR( the macro name '${name}' should begin with '_')
        fi
    else
        stripped_name=`echo ${name} | sed 's/^@<:@A-Za-z@:>@//'`
        if test x${stripped_name} = x${name}; then
            AC_MSG_ERROR( the macro name '${name}' should begin with an alphabetic letter)
        fi
    fi
    if test x${name} = x$3; then
        [replace=1]
    else
        [replace=`echo $3 | sed 's/^.*=//'`]
    fi
    AC_DEFINE_UNQUOTED( [$mcpp_macro], "${name}")
    AC_DEFINE_UNQUOTED( [${mcpp_macro}_VAL], "${replace}")
    mcpp_ac_c_define_$1_$2=${name}
])

# mcpp_AC_C_PREDEF( SYSTEM | CPU | COMPILER, STD | OLD, additional description)
# Specify the target-specific predefined macro via mcpp_AC_C_DEFINE.
AC_DEFUN( [mcpp_AC_C_PREDEF], [
    AC_ARG_ENABLE( $1-$2-macro,
        AC_HELP_STRING( --enable-$1-$2-macro=name@<:@=replace@:>@,
            [specify the $1-specific $2-style predefined macro.  $3]),
        if test x${enableval} = xyes || test x${enableval} = xno; then
            AC_MSG_ERROR( This option requires an argument as 'name@<:@=replace@:>@')
        fi
        mcpp_AC_C_DEFINE( $1, $2, ${enableval})
        AH_TEMPLATE( [$1_SP_$2],
            [Define the name of $1-specific $2-style predefined macro.])
        AH_TEMPLATE( [$1_SP_$2_VAL],
            [Define the value of $1-specific $2-style predefined macro.])
    )
])

mcpp_AC_C_PREDEF( [SYSTEM], [STD],
    [In all this and the following 5 options,
    'STD'-style name begins with an '_',
    'OLD'-style name begins with an alphabetic and
    if '=replace' is missing the macro is defined as 1.]
)
mcpp_AC_C_PREDEF( [SYSTEM], [OLD])
mcpp_AC_C_PREDEF( [CPU], [STD])
mcpp_AC_C_PREDEF( [CPU], [OLD])
mcpp_AC_C_PREDEF( [COMPILER], [STD])
mcpp_AC_C_PREDEF( [COMPILER], [OLD])

fi

## --enable-replace-cpp option
AC_ARG_ENABLE( replace-cpp,
    AC_HELP_STRING( [--enable-replace-cpp],
        [Replace the resident preprocessor with mcpp]),
    ,       ## the compiler name will be defined later
    AC_DEFINE( [COMPILER], [STAND_ALONE], [Specify stand-alone build.])
            ## else define COMPILER as STAND_ALONE
)

if test x${enable_replace_cpp} = xyes; then         ## replace-cpp mode

dnl Check the characteristics of target compiler.
dnl Note: The following 3 hundreds and more lines are for replace-cpp-mode.
dnl Target-compiler-specific configuration need these lines.
dnl These lines are not used in stand-alone configuration.

dnl Check which header file CPP includes,
dnl relative to CURRENT or SOURCE directory.

# AC_C_INCLUDE_PREF( CURRENT | SOURCE | NEXT)
# Check the include preference ('NEXT' means CURRENT & SOURCE)
AC_DEFUN( [AC_C_INCLUDE_PREF], [
    AC_EGREP_CPP( $1, [
#include "../next/header.h"
],
        [ac_cv_c_include_pref=$1],
    )
])

AC_CACHE_CHECK( include preference - relative to current or source directory,
    ac_cv_c_include_pref,
    mkdir dir ../next ../next/dir
    echo '#include "dir/what.h"'    > ../next/header.h
    echo '#include "next.h"'        >> ../next/header.h
    echo 'CURRENT'  > dir/what.h
    echo 'SOURCE'   > ../next/dir/what.h
    echo 'NEXT'     > ../next/next.h
    AC_C_INCLUDE_PREF( [SOURCE])
    if test x${ac_cv_c_include_pref} != xSOURCE; then
        AC_C_INCLUDE_PREF( [CURRENT])
        if test x${ac_cv_c_include_pref} = xCURRENT; then
            AC_C_INCLUDE_PREF( [NEXT])
            if test x${ac_cv_c_include_pref} = xNEXT; then
                [ac_cv_c_include_pref="(CURRENT & SOURCE)"]
            fi
        else
            AC_MSG_ERROR( Failed to include header file.)
        fi
    fi
    rm -rf dir ../next
    if test x$ac_c_define_HOST_SYSTEM = xSYS_CYGWIN; then
        sleep 1         # wait for cleanup
    fi
)
AC_DEFINE_UNQUOTED( [SEARCH_INIT], ${ac_cv_c_include_pref},
    [Define include preference.])
 
AC_CACHE_CHECK( if the argument of pragma is macro expanded,
    ac_cv_c_pragma_is_expanded,
    AC_COMPILE_IFELSE( [[
#define once
#pragma once]],
        [ac_cv_c_pragma_is_expanded=no],
        [ac_cv_c_pragma_is_expanded=yes]
    )
)
if test x${ac_cv_c_pragma_is_expanded} = xyes; then
    AC_DEFINE( [EXPAND_PRAGMA], 1,
        [Define if the argument of pragma is macro expanded.])
fi

AC_CACHE_CHECK( if digraphs can be used,
    ac_cv_c_digraph,
    [AC_PREPROC_IFELSE( [
%:if 1
%:endif],
        [ac_cv_c_digraph=yes],
        [ac_cv_c_digraph=no]
    )]
)
if test x${ac_cv_c_digraph} = xyes; then
    AC_DEFINE( [HAVE_DIGRAPHS], 1, [Define if digraphs are available.])
fi

AC_CACHE_CHECK( preprocessor output format of line directive,
    ac_cv_c_line_format,
    [ac_cv_c_line_format=\"`echo '#line 4321' | ${CPP} - | grep 432 | sed 's/432.*$//'`\"]
## Note: the output line number may be 4320 on GCC 2.* or other preprocessors.
)
AC_DEFINE_UNQUOTED( [LINE_PREFIX], ${ac_cv_c_line_format},
    [Define output format of line directive.])
if test "x${ac_cv_c_line_format}" = "x\"#line \""; then
    ac_cv_c_std_line_prefix=1
else
    ac_cv_c_std_line_prefix=0
fi
AC_DEFINE_UNQUOTED( [STD_LINE_PREFIX], ${ac_cv_c_std_line_prefix},
    [Define whether output format of line directive is C source style.])

AC_CACHE_CHECK( default value of __STDC__,
    ac_cv_c_stdc,
    [stdc=`echo '__STDC__' | ${CPP} - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ .*//'`]
    if test x"${stdc}" = x"__STDC__"; then
        [ac_cv_c_stdc=0]
    else
        [ac_cv_c_stdc=${stdc}]
    fi
)
AC_DEFINE_UNQUOTED( [STDC], ${ac_cv_c_stdc},
    [Define the default value of __STDC__.])

AC_CACHE_CHECK( default value of __STDC_VERSION__,
    ac_cv_c_stdc_ver,
    [stdc_ver=`echo '__STDC_VERSION__' | ${CPP} - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ .*//'`]
    if test x"${stdc_ver}" = x"__STDC_VERSION__"; then
        [ac_cv_c_stdc_ver=0]
    else
        [ac_cv_c_stdc_ver=${stdc_ver}]
    fi
)
AC_DEFINE_UNQUOTED( [STDC_VERSION], ${ac_cv_c_stdc_ver},
    [Define the default value of __STDC_VERSION__.])

if test x${ac_cv_c_compiler_gnu} != xyes; then

# Specify the compiler's name
AC_ARG_WITH( compiler-name,
    AC_HELP_STRING( [--with-compiler-name=NAME],
        [specify the compiler name which is used as the definition of COMPILER macro]),
    AC_DEFINE_UNQUOTED( [COMPILER], ${compiler_name}, [specify the compiler name]),
    AC_MSG_ERROR( --with-compiler-name=NAME option should be specified to define [COMPILER] macro)
)

# Specify the C++ header file directory
AC_ARG_WITH( cxx-include-dir,
    AC_HELP_STRING( [--with-cxx-include-dir=directory],
        [specify the C++ include directory by absolute path [[none]]]),
    [case "${withval}" in
    [\\/]* | [A-Za-z]:[\\/]*)
        cxx_include_dir=`echo ${withval} | sed 's,/$,,'; 's,\\$,,'`
        AC_DEFINE_UNQUOTED( [CPLUS_INCLUDE_DIR1], ${cxx_include_dir},
            [Define the C++ include directory.])
        ;;
    *)  AC_MSG_ERROR( ${withval} should be an absolute path) ;;
    esac]
)

fi  # not gcc

dnl Define compiler specific elements.

AC_MSG_NOTICE( checking for some compiler specific elements)

# Define GCC specific elements.
if test x${ac_cv_c_compiler_gnu} = xyes; then

    AC_DEFINE( [COMPILER], [GNUC], [Define the target compiler.])

    cpp_call=`echo '' | ${CPP} -v - 2>&1 | $EGREP '/cpp|/cc1' | sed 's/^ *//; s/ .*//'` 
    AC_MSG_CHECKING( for the directory to install mcpp)
    cpp_name=`expr ${cpp_call} : '.*/\(.*\)'`
    bindir=`expr ${cpp_call} : "\(.*\)/${cpp_name}"`
    AC_MSG_RESULT( ${bindir})

    lang_save=${LANG}
    export LANG=C

    AC_MSG_CHECKING( for the compiler-specific C include directory)
    c_include_dirs=`echo '' | ${CPP} -xc -v - 2>&1 | sed '1,/> search starts here/d' | sed '/End of search list/q' | sed 's/^ *//' | $EGREP -v '^/usr(/local|)/include$|End of search list'`
    n=1
    for dir in ${c_include_dirs}
    do
        case $n in
        1)  c_include_dir1=${dir};;
        2)  c_include_dir2=${dir};;
        esac
        let n=n+1
    done
    if test x${c_include_dir1+set} = xset; then
        AC_DEFINE_UNQUOTED( [C_INCLUDE_DIR1], "${c_include_dir1}",
            [Define compiler-specific C include directory 1.])
        AC_MSG_RESULT( ${c_include_dir1})
    else
        AC_MSG_RESULT( none)
    fi
    if test x${c_include_dir2+set} = xset; then
        AC_DEFINE_UNQUOTED( [C_INCLUDE_DIR2], "${c_include_dir2}",
            [Define compiler-specific C include directory 2.])
        AC_MSG_RESULT( ${c_include_dir2})
    fi

    AC_PROG_CXX
    AC_PROG_CXXCPP
    AC_LANG_PUSH( C++)
    AC_MSG_CHECKING( for the compiler-specific C++ include directory)
    cxx_include_dirs=`echo '' | ${CXXCPP} -xc++ -v - 2>&1 | sed '1,/> search starts here/d' | sed '/End of search list/q' | sed '/End of search list/d' | sed 's/^ *//' | $EGREP -v "^/usr(/local|)/include\$|^${c_include_dir1}\$|^${c_include_dir2}\$"`
    n=1
    for dir in ${cxx_include_dirs}
    do
        case $n in
        1)  cxx_include_dir1=${dir};;
        2)  cxx_include_dir2=${dir};;
        3)  cxx_include_dir3=${dir};;
        4)  cxx_include_dir4=${dir};;
        esac
        let n=n+1
    done
    if test x${cxx_include_dir1+set} = xset; then
        AC_DEFINE_UNQUOTED( [CPLUS_INCLUDE_DIR1], "${cxx_include_dir1}",
            [Define compiler-specific C include directory 1.])
        AC_MSG_RESULT( ${cxx_include_dir1})
    fi
    if test x${cxx_include_dir2+set} = xset; then
        AC_DEFINE_UNQUOTED( [CPLUS_INCLUDE_DIR2], "${cxx_include_dir2}",
            [Define compiler-specific C include directory 2.])
        AC_MSG_RESULT( ${cxx_include_dir2})
    fi
    if test x${cxx_include_dir3+set} = xset; then
        AC_DEFINE_UNQUOTED( [CPLUS_INCLUDE_DIR3], "${cxx_include_dir3}",
            [Define compiler-specific C include directory 3.])
        AC_MSG_RESULT( ${cxx_include_dir3})
    fi
    if test x${cxx_include_dir4+set} = xset; then
        AC_DEFINE_UNQUOTED( [CPLUS_INCLUDE_DIR4], "${cxx_include_dir4}",
            [Define compiler-specific C include directory 4.])
        AC_MSG_RESULT( ${cxx_include_dir4})
    fi
    AC_LANG_POP( C++)

    export LANG=${lang_save}

    if test x${c_include_dir1+set} = xset; then
        inc_dir=${c_include_dir1}
    else
        inc_dir=/usr/local/include
    fi

    AC_SUBST( [inc_dir])
    AC_SUBST( [cpp_call])
    AC_SUBST( [gcc_maj_ver])
    AC_SUBST( [gcc_min_ver])
    AC_PATH_PROG( [gcc_path], [${CC}], , [${PATH}])

    AC_PROG_LN_S

    if test x${gcc_maj_ver} = x2; then

        AC_CACHE_CHECK( for value of predefined macro '__SIZE_TYPE__',
            size_type,
            [size_type=`echo '__SIZE_TYPE__' | ${CPP} - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ *//'`]
        )
        AC_DEFINE_UNQUOTED( [COMPILER_SP1_VAL], "${size_type}",
            [Define the type of size_t.])

        AC_CACHE_CHECK( for value of predefined macro '__PTRDIFF_TYPE__',
            ptrdiff_type,
            [ptrdiff_type=`echo '__PTRDIFF_TYPE__' | ${CPP} - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ *$//'`]
        )
        AC_DEFINE_UNQUOTED( [COMPILER_SP2_VAL], "${ptrdiff_type}",
            [Define the type of ptrdiff_t.])

        AC_CACHE_CHECK( for value of predefined macro '__WCHAR_TYPE__',
            wchar_type,
            [wchar_type=`echo '__WCHAR_TYPE__' | ${CPP} - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ *$//'`]
        )
        AC_DEFINE_UNQUOTED( [COMPILER_SP3_VAL], "${wchar_type}",
        [Define the type of wchar_t.])

    fi
fi

if test x${ac_cv_c_compiler_gnu} = xyes; then

    AC_DEFUN( [AC_CHECK_MBCHAR_IS_ESCAPE_FREE], [
        AC_CACHE_CHECK(
            [if '0x5c' in $1 multi-byte character is handled differently from backslash],
            [ac_cv_c_$2_escape_free],
            AC_RUN_IFELSE( [[
    #include    <assert.h>
        int     strcmp();
        void    exit();
    #define str( a)     # a
    int main( void)
    {
        assert( strcmp( str( "$3"), "\"$3\"") == 0);    /* Multi-byte chars */
        exit (0);
    }]],
                [ac_cv_c_$2_escape_free=yes],
                [ac_cv_c_$2_escape_free=no],
                AC_MSG_WARN( [cannot run on cross-compiling so assumes that '0x5c' in multi-byte character is not rightly handled.])
                [ac_cv_c_$2_escape_free=no]
            )
        )
    ])
    
    ## The following 3 mb_* files are separated from this file, since
    ## the editors do not allow multiple encodings of multi-byte characters.
    
    if test x${ac_cv_c_compiler_gnu} = xyes; then
        lang_save=${LANG}
    fi
    
    ## Set the environment variable, though this is seldom enabled in GCC.
    if test x${ac_cv_c_compiler_gnu} = xyes; then
        export LANG=C-SJIS
    fi
    m4_include(./config/mb_sjis)
    if test x${ac_cv_c_sjis_escape_free} = xyes; then
        AC_DEFINE( [SJIS_IS_ESCAPE_FREE], 1,
            [Define if '0x5c' in SJIS multi-byte character is safe.])
    fi
    
    if test x${ac_cv_c_compiler_gnu} = xyes; then
        export LANG=C-JIS
    fi
    m4_include(./config/mb_jis)
    if test x${ac_cv_c_iso2022_jp_escape_free} = xyes; then
        AC_DEFINE( [ISO2022_JP_IS_ESCAPE_FREE], 1,
            [Define if '0x5c' in ISO2022-JP multi-byte character is safe.])
    fi
    
    if test x${ac_cv_c_compiler_gnu} = xyes; then
        export LANG=${lang_save}
    fi
    
    m4_include(./config/mb_big5)
    if test x${ac_cv_c_bigfive_escape_free} = xyes; then
        AC_DEFINE( [BIGFIVE_IS_ESCAPE_FREE], 1,
            [Define if '0x5c' in BIG5 multi-byte character is safe.])
    fi
    
fi ## gcc

fi ## replace-cpp mode

AC_ARG_WITH( [gcc-testsuite-dir],
    AC_HELP_STRING( [--with-gcc-testsuite-dir=directory],
        [specify the gcc testsuite directory to do make check.
        e.g. /usr/src/rpm/SOURCES/gcc-2.95.4/gcc/testsuite,
        /usr/local/src/gcc-3.2/gcc/testsuite, etc.
        The testsuite of mcpp uses gcc testsuite, hence needs gcc source
        or at least gcc testsuite.
        This option needs --enable-replace-cpp option.]),
    if test x"${withval+set}" = x"set"; then
        if test x"${enable_replace_cpp}" = x"yes"; then
            [gcc_testsuite_dir=`echo ${withval} | sed 's,/$,,'`]
        else
            AC_MSG_ERROR( [--with-gcc-testsuite-dir option requires --enable-replace-cpp option])
        fi
    else
        AC_MSG_ERROR( [--with-gcc-testsuite-dir option requires an argument])
    fi
)
AC_SUBST( [gcc_testsuite_dir])
AM_CONDITIONAL( [GNUC], test x${ac_cv_c_compiler_gnu} = xyes)
AM_CONDITIONAL( [REPLACE_CPP], test x${enable_replace_cpp} = xyes)


dnl Final output.

AC_OUTPUT( Makefile src/Makefile tests/Makefile)

