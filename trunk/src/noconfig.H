/*
 * noconfig.H
 * Configurations for MCPP not using config.h.
 *
 * 2003/11  Created.                                        by kmatsui
 *
 * 2004/02  Extended multi-byte character handling.
 *          Added configuration for Plan 9 / pcc.           by kmatsui
 *
 * 2005/03  Changed default setting to FreeBSD 5.* / GNU C 3.4.
 *                                                          by kmatsui
 *
 * WARNING: These are default settings.  To configure for your system, you
 *      must edit this file here and there.
 */

/*
 *  'Target' means the O.S. and the compiler to which cpp is implemented.
 *  'Host' means the O.S. and the compiler with which cpp is compiled.
 */

/* Define target operating-system.  */
#define SYSTEM              SYS_FREEBSD

/* Define target compiler.          */
#define COMPILER            GNUC

/* Define host operating-system.    */
#define HOST_SYSTEM         SYSTEM

/* Define host compiler.            */
#define HOST_COMPILER       COMPILER

/* Version message.                 */
/* "MCPP V.2.* (200y/mm) $(MODE) mode for " precedes VERSION_MSG    */
#define VERSION_MSG     \
    "GNU C compiled by GNU C 3.4"
#if     0
    "DJGPP compiled by DJGPP (GNU C 2.7.1)"
    "LCC-Win32 compiled by LCC-Win32 V.3.2"
    "Visual C compiled by Visual C .net 2003"
    "Borland C compiled by BCC V.5.5"
    "LSI C-86 compiled by LSI C-86 V.3.3"
    "PLAN9 PCC compiled by PCC"
#endif

#define TRUE                1
#define FALSE               0

/*
 *      P A R T   1     Configurations for target-operating-system
 *                      and target-compiler.
 */

/*
 * Names of the SYSTEM (i.e. target operating system).  This is needed so that
 * cpp can use appropriate filename conventions.
 */
#define SYS_UNKNOWN         0
#define SYS_UNIX            0x1000
#define SYS_ULTRIX          0x1100  /* (SYS_ULTRIX & 0xF000) == SYS_UNIX    */
#define SYS_LINUX           0x1800  /* (SYS_LINUX & 0xF000) == SYS_UNIX     */
#define SYS_FREEBSD         0x1A00  /* (SYS_FREEBSD & 0xF000) == SYS_UNIX   */
#define SYS_CYGWIN          0x1C00  /* (SYS_CYGWIN & 0xF000) == SYS_UNIX    */
#define SYS_PLAN9           0x1F00  /* (SYS_PLAN9 & 0xF000) == SYS_UNIX     */
#define SYS_OS9             0x2000
#define SYS_OSK             0x2100  /* (SYS_OSK & 0xF000)   == SYS_OS9      */
#define SYS_OS9000          0x2200  /* (SYS_OS9000 & 0xF000) == SYS_OS9     */
#define SYS_MAC             0x6000
#define SYS_MSDOS           0x7000
#define SYS_DJGPP           0x7400  /* (SYS_DJGPP & 0xF000) == SYS_MSDOS    */
#define SYS_OS2             0x7800  /* (SYS_OS2 & 0xF000)   == SYS_MSDOS    */
#define SYS_WIN32           0x7C00  /* (SYS_WIN32 & 0xF000) == SYS_MSDOS    */

/* COMPILER */
#define COMPILER_UNKNOWN    0
#define PCC                 0x1000  /* Portable CC  */
#define PLAN9_PCC           0x1F00  /* Plan 9 pcc (cpp && 2c, 8c, etc.)     */
#define MWC09               0x2000
#define MWC68K              0x2100
#define ULTRAC              0x2200
#define MWPC                0x6000
#define MSC                 0x7000  /* Microsoft C, Visual C++      */
#define BORLANDC            0x7040  /* Turbo C, Borland C   */
#define LSIC                0x7060
#define DOS_SYMANTECC       0x7070
#define LATTICEC            0x7080
#define LCC                 0x7CC0
#define GNUC                0x00F0

#define SYS_FAMILY          (SYSTEM & 0xF000)
#define COMPILER_FAMILY     (COMPILER & 0xF0)
#define HOST_SYS_FAMILY     (HOST_SYSTEM & 0xF000)

/* Default MBCHAR (multi-byte character) encoding.  */
#define EUC_JP          0x10    /* Extended UNIX code of JIS X 0208 */
#define GB2312          0x20    /* EUC-like encoding of Chinese GB 2312-80  */
#define KSC5601         0x30    /* EUC-like encoding of Korean KS C 5601    */
#define SJIS            0x80    /* Shift-JIS encoding of JIS X 0208 */
#define BIGFIVE         0x90    /* Encoding of Taiwanese Big Five   */
#define ISO2022_JP      0x100   /* ISO-2022-JP (ISO-2022-JP1) encoding      */
#define UTF8            0x1000  /* UTF-8 encoding           */

/*
 * MBCHAR means multi-byte character encoding.
 * In SYS_MSDOS you must specify one of the above encodings (or 0).
 * In 32-bit or larger system MBCHAR means the default encoding, and you
 *      can change the encoding by #pragma __setlocale, -m <encoding> option
 *      or environment variable LC_ALL, LC_CTYPE, LANG.
 * MBCHAR == 0 means no multi-byte character encoding.
 */

/*
 * In order to predefine target-dependent macros,
 * several macros are defined here:
 * *_OLD define the macro beginning with an alphabetic letter,
 * *_STD, *_STD?, *_EXT, *_EX2 define the macro beginning with an '_'.
 * *_STD1 define the macro beginning with '__' and ending with an alpha-
 *          numeric letter.
 * *_STD2 define the macro beginning with '__' and ending with '__'.
 * These may not be defined, if they are not needed.
 * They should not be #defined to no token or to "".
 *
 * CPU_OLD, CPU_STD1, CPU_STD2, CPU_SP_OLD, CPU_SP_STD
 *      define the target cpu (by name)
 * SYSTEM_OLD, SYSTEM_STD1, SYSTEM_STD2, SYSTEM_EXT, SYSTEM_EX2
 *      define the target operating system (by name)
 * SYSTEM_SP_OLD, SYSTEM_SP_STD     define the target-OS specific macro name
 * COMPILER_OLD, COMPILER_STD1, COMPILER_STD2, COMPILER_EXT, COMPILER_EX2
 *          , COMPILER_SP_OLD, COMPILER_SP_STD
 *      define the target compiler (by name)
 * COMPILER_CPLUS        define the target C++ compiler
 * COMPILER_SP1, COMPILER_SP2, COMPILER_SP3
 *      define the compiler-specific macros
 *
 * <macro>_VAL      specify the value of the <macro>.
 *      If not specified, these values default to "1".
 *      To define the value of no-token, specify as "" rather than no-token.
 * CPU_*, SYSTEM_OLD, SYSTEM_STD?, COMPILER_OLD have the value of "1".
 */

/*
 * target-compiler-dependent definitions:
 *
 * LINE_PREFIX  defines the output line prefix, if not "#line 123".
 *              This should be defined as "# " to represent "# 123" format
 *              ("#line " represents "#line 123" format).
 *
 * HAVE_PRAGMA  should be set TRUE, if COMPILER can handle the #pragma lines.
 *
 * C_INCLUDE_DIR1, C_INCLUDE_DIR2   may be defined if you have a compiler-
 *              specific include directory which is to be searched *before*
 *              the operating-system specific directories (e.g. /usr/include).
 * CPLUS_INCLUDE_DIR1, CPLUS_INCLUDE_DIR2, CPLUS_INCLUDE_DIR3
 *      , CPLUS_INCLUDE_DIR4     are for C++ include directory which exist
 *              other than C include directory.
 * ENV_C_INCLUDE_DIR    may be defined to the name of environment-variable for
 *              C include directory.
 * ENV_CPLUS_INCLUDE_DIR    is name of environment-variable for C++ include
 *              directory which exists other than ENV_C_INCLUDE_DIR.
 * ENV_SEP      is the separator (other than space) of include-paths in an
 *              environment-variable.  e.g. the ':' in
 *              "/usr/abc/include:/usr/xyz/include"
 *
 * EMFILE       should be defined to the macro to represent errno of 'too many
 *              open files' if the macro is different from EMFILE.
 *
 * ONE_PASS     should be set TRUE, if COMPILER is "one pass compiler".
 *
 * FNAME_FOLD   means that target-system folds upper and lower cases of
 *              directory and file-name.
 *
 * SEARCH_INIT  specifies the default value of 'search_rule' (in system.c).
 *              'search_rule' holds searching rule of #include "header.h" to
 *              search first before searching user specified or system-
 *              specific include directories.
 *              CURRENT means to search the directory relative to "current
 *              directory" which is current at cpp invocation.
 *              SOURCE means to search the directory relative to that of the
 *              source file (i.e. "includer").
 *              (CURRENT & SOURCE) means to search current directory first
 *              source directory next.
 *              'search_rule' is initialized to SEARCH_INIT.
 */
#define CURRENT             1
#define SOURCE              2

#if     SYS_FAMILY == SYS_UNIX
#define SYSTEM_OLD          "unix"
#define SYSTEM_STD1         "__unix"
#define SYSTEM_STD2         "__unix__"
#endif

#if     SYSTEM == SYS_ULTRIX
#define CPU_OLD             "vax"       /* "mips"   */
#define CPU_STD1            "__VAX"     /* "__mips", "__MIPSEL", "__MIPSEB" */
#define SYSTEM_OLD          "ultrix"
#define SYSTEM_STD1          "__ULTRIX"
#endif

#if     SYSTEM == SYS_FREEBSD || SYSTEM == SYS_LINUX
#define CPU_STD2            "__i386__"
#endif

#if     SYSTEM == SYS_FREEBSD
#define SYSTEM_EXT          "__FreeBSD__"
#define SYSTEM_EXT_VAL      "5"     /* V.4.*: 4, V.5.*: 5   */
#endif

#if     SYSTEM == SYS_LINUX
#define SYSTEM_EXT          "__linux__"
#endif

#if     SYSTEM == SYS_PLAN9
#define SYSTEM_EXT          "_PLAN9"
#define MBCHAR              UTF8
#define HAVE_UNSIGNED_LONG  TRUE
#define HAVE_LONG_LONG      TRUE
#define HAVE_LONG_DOUBLE    TRUE
#define ENV_C_INCLUDE_DIR   "include"
#define ENV_SEP             ' '
#endif

#if     SYSTEM == SYS_CYGWIN
#define CPU_STD2            "__i386__"
#define SYSTEM_EXT          "__CYGWIN__"
#define SYSTEM_EX2          "__CYGWIN32__"
#define MBCHAR              SJIS
#define FNAME_FOLD          TRUE
#endif  /* SYSTEM == SYS_CYGWIN */

#if     SYS_FAMILY == SYS_UNIX
#ifndef MBCHAR
#define MBCHAR              EUC_JP  /* UTF8 if you like */
#endif
#ifndef FNAME_FOLD
#define FNAME_FOLD          FALSE
#endif
#define SEARCH_INIT         SOURCE
#endif

#if     SYSTEM == SYS_MSDOS || SYSTEM == SYS_WIN32
/*
 *      Here we assume that the operating system is MS-DOS or WIN32, the
 * processor is an Intel 8086 (or i386, etc.) and #pragma directive, unsigned
 * long and long double data types are available for the compilers.
 */
#if     SYSTEM == SYS_MSDOS
#define CPU_STD2            "__8086__"
#define SYSTEM_OLD          "MSDOS"
#define SYSTEM_STD2         "__MSDOS__"
#else   /* SYSTEM == SYS_WIN32  */
#define CPU_STD2            "__i386__"
#define SYSTEM_OLD          "WIN32"
#define SYSTEM_STD2         "__WIN32__"
#define SYSTEM_EXT          "__FLAT__"
#endif  /* SYSTEM == SYS_WIN32  */

#ifndef MBCHAR
#define MBCHAR              SJIS    /* Multi-byte char is encoded in SJIS   */
#endif
#define HAVE_UNSIGNED_LONG  TRUE    /* Compiler proper has unsigned long    */
#define HAVE_LONG_DOUBLE    TRUE    /* Compiler proper has long double      */

#if     COMPILER == LCC
#define COMPILER_STD2       "__LCC__"
#define SYSTEM_EX2          "_WIN32"
#define COMPILER_SP1        "__LCCDEBUGLEVEL"
#define COMPILER_SP1_VAL    "0"
#define COMPILER_SP2        "__LCCOPTIMLEVEL"
#define COMPILER_SP2_VAL    "0"
#define ONE_PASS            TRUE
#define HAVE_LONG_LONG      TRUE
#define SJIS_IS_ESCAPE_FREE FALSE 
#else

#if     COMPILER == MSC
#define SYSTEM_EX2          "_WIN32"
#if     _MSC_VER >= 1200    /* Visual C++ V.6.0 or later    */
    /* When HOST == TARGET  */
#define ONE_PASS            TRUE
#endif
#define COMPILER_EXT        "_MSC_VER"
#define COMPILER_EXT_VAL    "1310"  /* MSC 6.0: "600", MSC 7.0: "700"
                , VC 4.0: "1000", VC 5.0: "1100", VC 6.0: "1200"
                , VC .NET 2002: "1300", VC .NET 2003: "1310"        */
#define COMPILER_SP1        "_MSC_EXTENSIONS"
#define COMPILER_SP1_VAL    "1"
#define COMPILER_SP2        "_M_IX86"
#define COMPILER_SP2_VAL    "600"       /* Pentium Pro and later    */
#define HAVE_LONG_LONG      TRUE
    /* __int64 instead of long long (VC 2.0 and later)      */
#define EXPAND_PRAGMA       TRUE
#define HAVE_DIGRAPHS       TRUE
#define BIGFIVE_IS_ESCAPE_FREE      TRUE 
#define SEARCH_INIT         SOURCE
#else

#if     COMPILER == BORLANDC
#define ONE_PASS            TRUE
#define COMPILER_STD2       "__TURBOC__"
#if     __BORLANDC__        /* Borland C    */
#define COMPILER_STD2_VAL   "0x0550"
    /* BC 2.0:"0x0297", BC 3.0:"0x0400", BC 4.0:"0x0452", BC 5.5:"0x0550"   */
#define COMPILER_EXT        "__BORLANDC__"
#define COMPILER_EXT_VAL    "0x0550"
    /* BC 2.0:"0x0297", BC 3.0:"0x0400", BC 4.0:"0x0452", BC 5.5:"0x0550"   */
#define COMPILER_CPLUS      "__BCPLUSPLUS__"
#define COMPILER_CPLUS_VAL  "0x0550"            /* BC++ 2.0: "0x0200,
        BC++ 3.0: "0x0300", BC++ 4.0: "0x0320", BC++ 5.5: "0x0550   */
#define C_INCLUDE_DIR1      "/BCC55/INCLUDE"
#define SEARCH_INIT         (CURRENT & SOURCE)
    /* BC++ 5.*: (CURRENT & SOURCE), BC++ 4.* or former: CURRENT    */
#if     __BORLANDC__ >= 0x550
#define HAVE_LONG_LONG      TRUE    /* __int64 instead of long long */
#endif
#else
#if     __TCPLUSPLUS__      /* Turbo C++    */
#define COMPILER_STD2_VAL   "0x0296"        /* TC++ 1.0: "0x0296"   */
#define COMPILER_CPLUS      "__TCPLUSPLUS__"
#define COMPILER_CPLUS_VAL  "0x0100"        /* TC++ 1.0: "0x0100"   */
#define C_INCLUDE_DIR1      "/TCPP1/INCLUDE"
#else                       /* Turbo C      */
#define COMPILER_STD2_VAL   "0x0200"        /* TC 2.0: "0x0200"     */
#define C_INCLUDE_DIR1      "/TC2/INCLUDE"
#endif
#endif
#else

#if     COMPILER == LSIC
#define COMPILER_STD2       "__LSIC__"
#define COMPILER_OLD        "LSI_C"
#define LINE_PREFIX         "# "
#define C_INCLUDE_DIR1      "/LSIC86/INCLUDE"
#define SEARCH_INIT         SOURCE
#define STDC                0
#endif
#endif
#endif
#endif

#ifndef SJIS_IS_ESCAPE_FREE
#define SJIS_IS_ESCAPE_FREE TRUE    /* or FALSE following your compiler     */
#endif

#endif  /* SYSTEM == SYS_MSDOS || SYSTEM == SYS_WIN32   */

#if     SYSTEM == SYS_DJGPP
#define CPU_STD2             "__i386__"
#define SYSTEM_STD2          "__GO32__"
#ifndef MBCHAR
#define MBCHAR              SJIS
#endif
#define SJIS_IS_ESCAPE_FREE TRUE    /* Compiler can handle '\\' in SJIS     */
#define COMPILER_STD2       "__DJGPP__"
#define COMPILER_STD2_VAL   "1"     /* "1" for DJGPP V.1.*, "2" for V.2.*   */
#define HAVE_LONG_LONG      FALSE
#endif  /* SYSTEM == SYS_DJGPP  */

#if     COMPILER_FAMILY == GNUC
#define COMPILER_EXT        "__GNUC__"
#define COMPILER_EXT_VAL    "3"     /* "2" for V.2.*, "3" for V.3.* */
#define COMPILER_EX2        "__GNUC_MINOR__"
#define COMPILER_EX2_VAL    "4"     /* "95" for V.2.95, "3" for V.3.3   */
#define COMPILER_CPLUS      "__GNUG__"
#define COMPILER_CPLUS_VAL  "3"     /* "2" for V.2.*, "3" for V.3.* */
#define LINE_PREFIX         "# "
#ifndef HAVE_DIGRAPHS
#define HAVE_DIGRAPHS       TRUE
#endif
#define HAVE_UNSIGNED_LONG  TRUE
#ifndef HAVE_LONG_LONG
#define HAVE_LONG_LONG      TRUE
#endif
#define HAVE_INTMAX_T       TRUE    /* FALSE for V.2, TRUE for V.3  */
#if     HAVE_INTMAX_T
#define HAVE_INTTYPES_H     TRUE
#endif
#define HAVE_LONG_DOUBLE    TRUE
#define STDC_VERSION        0L      /* 199409L for V.2.9, 0L for V.3.*      */
#define CPLUS_INCLUDE_DIR1  "/usr/include/c++"  /* "/usr/include/c++/3.3.2" */
#define CPLUS_INCLUDE_DIR2  "/usr/include/c++/backward"     /* "/usr/include/c++/3.3.2/i386-vine-linux" */
/* #define CPLUS_INCLUDE_DIR3  "/usr/include/c++/3.3.2/backward"    */
/* #define C_INCLUDE_DIR1  "/usr/lib/gcc-lib/i386-vine-linux/3.3.2/include" */
#ifndef ENV_C_INCLUDE_DIR
#define ENV_C_INCLUDE_DIR   "C_INCLUDE_PATH"
#define ENV_CPLUS_INCLUDE_DIR   "CPLUS_INCLUDE_PATH"
#define ENV_VERSION         "GCC_VERSION"
            /* The name of environment variable to specify GNU C VERSION    */
#endif
/*
 * __SIZE_TYPE__, __PTRDIFF_TYPE__ and __WCHAR_TYPE__ are predefines of
 * GNU C / cpp.
 */
#define COMPILER_SP1        "__SIZE_TYPE__"
#define COMPILER_SP2        "__PTRDIFF_TYPE__"
#define COMPILER_SP3        "__WCHAR_TYPE__"
#define COMPILER_SP1_VAL    "unsigned int"
    /* SYS_DJGPP: "long unsigned int", others: "unsigned int"       */
#define COMPILER_SP2_VAL    "int"
    /* SYS_DJGPP: "long int", others: "int"     */
#define COMPILER_SP3_VAL    "int"
    /* SYS_FREEBSD,SYS_DJGPP: "int" , SYS_CYGWIN: "short unsigned int"
        , others: "long int"    */
#endif  /* COMPILER_FAMILY == GNUC   */

#if     SYS_FAMILY == SYS_OS9
#define SYSTEM_OLD          "OS9"
#ifndef MBCHAR
#define MBCHAR              SJIS
#endif
#define SJIS_IS_ESCAPE_FREE TRUE    /* Compiler can handle '\\' in SJIS     */
#define C_INCLUDE_DIR1      "/D0/INCLUDE"
#if     COMPILER == MWC68K
#define CPU_STD2            "__m68k__"
#define SYSTEM_STD2         "__OSK__"
#define COMPILER_STD2       "__MWC68K__"
#else
#if     COMPILER == ULTRA_C
#define CPU_STD2            "__m68k__"      /* "__i386__", ...      */
#define SYSTEM_STD2         "__OSK__"       /* "__OS9000__"         */
#define COMPILER_STD2       "__ULTRAC__"
#define HAVE_UNSIGNED_LONG  TRUE
#define HAVE_LONG_DOUBLE    TRUE
#endif
#endif
#endif  /* SYSTEM == SYS_OS9    */

/*
 * defaults
 */

#ifdef  SYSTEM_EXT
#ifndef SYSTEM_EXT_VAL
#define SYSTEM_EXT_VAL      "1"
#endif
#endif
#ifdef  SYSTEM_EX2
#ifndef SYSTEM_EX2_VAL
#define SYSTEM_EX2_VAL      "1"
#endif
#endif
#ifdef  COMPILER_STD1
#ifndef COMPILER_STD1_VAL
#define COMPILER_STD1_VAL   "1"
#endif
#endif
#ifdef  COMPILER_STD2
#ifndef COMPILER_STD2_VAL
#define COMPILER_STD2_VAL   "1"
#endif
#endif
#ifdef  COMPILER_EXT
#ifndef COMPILER_EXT_VAL
#define COMPILER_EXT_VAL    "1"
#endif
#endif
#ifdef  COMPILER_EX2
#ifndef COMPILER_EX2_VAL
#define COMPILER_EX2_VAL    "1"
#endif
#endif
#ifdef  COMPILER_CPLUS
#ifndef COMPILER_CPLUS_VAL
#define COMPILER_CPLUS_VAL  "1"
#endif
#endif

#ifndef LINE_PREFIX
#define LINE_PREFIX         "#line "
#define STD_LINE_PREFIX     TRUE    /* C source format              */
#else
#define STD_LINE_PREFIX     FALSE   /* Compiler-dependent format    */
#endif

#ifndef HAVE_PRAGMA
#define HAVE_PRAGMA         TRUE
#endif

#ifndef HAVE_DIGRAPHS
#define HAVE_DIGRAPHS       FALSE
#endif

#ifndef ENV_C_INCLUDE_DIR
#define ENV_C_INCLUDE_DIR   "INCLUDE"
#endif
#ifndef ENV_CPLUS_INCLUDE_DIR
#define ENV_CPLUS_INCLUDE_DIR   "CPLUS_INCLUDE"
#endif

#ifndef ENV_SEP
#if     SYS_FAMILY == SYS_MSDOS
#define ENV_SEP             ';'
#else
#define ENV_SEP             ':'
#endif
#endif

#ifndef ONE_PASS
#define ONE_PASS            FALSE
#endif

#ifndef FNAME_FOLD
#define FNAME_FOLD          TRUE
#endif

#ifndef SEARCH_INIT
#define SEARCH_INIT         CURRENT
#endif

/*
 * These three definitions should be set TRUE, if *both* of the target
 * and the host compilers have the specified data type.
 * The latter two are used only by #if sizeof evaluation routine.
 * These definitions are written in PART 1 for convenience.
 */
/* The target and host compilers both have the type unsigned long ? */
#ifndef HAVE_UNSIGNED_LONG
#define HAVE_UNSIGNED_LONG  FALSE
#endif
/* The target and host compilers both have the type long long int ? */
#ifndef HAVE_LONG_LONG
#define HAVE_LONG_LONG      FALSE
#endif
/* The target and host compilers both have the type long double ?   */
#ifndef HAVE_LONG_DOUBLE
#define HAVE_LONG_DOUBLE    FALSE
#endif

#ifndef HAVE_INTMAX_T
#define HAVE_INTMAX_T       FALSE
#endif

/* Define the format specifier of intmax_t or long long for fprintf( fp_debug,).   */
#if     HAVE_LONG_LONG
#if     COMPILER == MSC || COMPILER == BORLANDC
#define LL_FORM             "I64"   /* Format specifier for __int64 */
#endif
#ifndef LL_FORM
#define LL_FORM             "ll"    /* C99: for long long, "j" for intmax_t */
#endif
#endif

/*
 * CHARBIT, UCHARMAX are respectively CHAR_BIT, UCHAR_MAX of target compiler.
 * CHARBIT should be defined to the number of bits per character.
 * It is needed only for processing of multi-byte character constants.
 * UCHARMAX should be defined to the maximum value of type unsigned char
 * or maximum value of unsigned int which is converted from type (signed)
 * char.
 *
 * LONGMAX      should be defined to the LONG_MAX in <limits.h>.
 * ULONGMAX     should be defined to the ULONG_MAX in <limits.h> or LONG_MAX
 *              for the compiler which lacks of unsigned long.
 */

#ifndef CHARBIT
#define CHARBIT             8
#endif
#ifndef UCHARMAX
#define UCHARMAX            0xFF
#endif
#ifndef LONGMAX
#define LONGMAX             0x7FFFFFFFL
#endif
#ifndef ULONGMAX
#define ULONGMAX            0xFFFFFFFFUL
                    /* or 2147483647L in case of !HAVE_UNSIGNED_LONG        */
#endif

/*
 * CAN_CONCAT_SGRINGS   should be set TRUE if the compiler concatenate
 *              adjacent string literals.
 */
#ifndef CAN_CONCAT_STRINGS
#define CAN_CONCAT_STRINGS  TRUE
#endif

/*
 * Define MBCHAR (multi-byte character encoding) to SJIS, EUC_JP or other.
 */
#ifndef MBCHAR
#define MBCHAR              0
#endif

/*
 * SJIS_IS_ESCAPE_FREE means the compiler does not escape '0x5c' ('\\') in
 * shift-JIS encoded multi-byte character.  SJIS_IS_ESCAPE_FREE == FALSE 
 * enables cpp to insert * '\\' before '\\' of the 2nd byte of SJIS code in
 * literal.  This insertion is for the compiler-proper which can't recognize
 * SJIS literal.
 * BIGFIVE_IS_ESCAPE_FREE means similar case on BIGFIVE encoding.
 * ISO2022_JP_IS_ESCAPE_FREE means similar case on ISO2022_JP encoding.
 *
 * GNU C can handle these encodings if it has been configured so.
 */
#ifndef SJIS_IS_ESCAPE_FREE
#define SJIS_IS_ESCAPE_FREE     FALSE   /* or TRUE following your compiler  */
#endif
#ifndef BIGFIVE_IS_ESCAPE_FREE
#define BIGFIVE_IS_ESCAPE_FREE  FALSE   /* or TRUE following your compiler  */
#endif
#ifndef ISO2022_JP_IS_ESCAPE_FREE
#define ISO2022_JP_IS_ESCAPE_FREE   FALSE   /* or TRUE following your compiler  */
#endif

#ifndef HAVE_C_BACKSLASH_A
#define HAVE_C_BACKSLASH_A  TRUE
#endif

/*
 * These are used to implement the Standard C escape sequences '\a' and '\v'.
 */
#if     ! HAVE_C_BACKSLASH_A
        /* 1: for the target compiler-proper which can't recognize \a, \v   */
#define ALERT_STR           "\\007"
#define VT_STR              "\\013"
#endif

/*
 *      P A R T   2     Configurations for host-compiler.
 *
 *      WARNING: In case of HOST_COMPILER differs from COMPILER, you must
 *          edit here and there of this part.
 */
#if     HOST_COMPILER == GNUC
#define HOST_HAVE_GETOPT            TRUE
#if     HOST_SYSTEM == SYS_LINUX
#define HOST_HAVE_STPCPY            TRUE
#define HOST_LIB_IS_GLIBC           TRUE
#endif

#else
#if     HOST_COMPILER == BORLANDC
#define HOST_HAVE_STPCPY            TRUE
#else
#if     HOST_COMPILER == LSIC
#define HOST_HAVE_STPCPY            TRUE
#define HOST_HAVE_SANE_FGETS        FALSE
#endif
#endif
#endif

#ifndef HOST_HAVE_C_BACKSLASH_A
#define HOST_HAVE_C_BACKSLASH_A     TRUE
#endif
#ifndef HOST_HAVE_GETENV
#define HOST_HAVE_GETENV            TRUE
#endif
#ifndef HOST_HAVE_MEMMOVE
#define HOST_HAVE_MEMMOVE           TRUE
#endif
#ifndef HOST_HAVE_MEMCPY
#define HOST_HAVE_MEMCPY            TRUE
#endif
#ifndef HOST_HAVE_MEMCMP
#define HOST_HAVE_MEMCMP            TRUE
#endif
#ifndef HOST_HAVE_STRSTR
#define HOST_HAVE_STRSTR            TRUE
#endif
#ifndef HOST_HAVE_STRCSPN
#define HOST_HAVE_STRCSPN           TRUE
#endif
#ifndef HOST_HAVE_STPCPY
#define HOST_HAVE_STPCPY            FALSE
#endif
#ifndef HOST_HAVE_GETOPT
#define HOST_HAVE_GETOPT            FALSE
#endif
#ifndef HOST_HAVE_SANE_FGETS
#define HOST_HAVE_SANE_FGETS        TRUE
#endif
#ifndef HOST_LIB_IS_GLIBC
#define HOST_LIB_IS_GLIBC           FALSE
#endif

#if     ! HOST_HAVE_C_BACKSLASH_A
        /* 1: for the host compiler-proper which can't recognize \a, \v     */
#define ALERT               '\007'          /* '\a' is "Bell"       */
#define VT                  '\013'          /* Vertical Tab CTRL/K  */
#endif

/*
 * PROTO    means that host-compiler permits prototype declarations.
 */
#ifndef PROTO
#define PROTO               TRUE
#endif

#if     PROTO
#define register
#else
#define const
#endif

#if     HOST_COMPILER == LSIC
#define const               /* To work around some bugs of LSIC     */
#endif

/*
 * Declaration of standard library functions and macros.
 */

/* stdin, stdout, stderr, FILE, NULL, fgets(), fputs() and other functions. */
#include    "stdio.h"

/* FILENAMEMAX should be defined to FILENAME_MAX of host system.    */
#ifdef  FILENAME_MAX
#define FILENAMEMAX     FILENAME_MAX
#else
#define FILENAMEMAX     BUFSIZ
#endif

/* islower(), isupper(), toupper(), isdigit(), isxdigit(), iscntrl()        */
#include    "ctype.h"

/* errno    */
#if     COMPILER == PLAN9_PCC
/* PLAN9_PCC fails to expand EMFILE defined in "errno.h"    */
extern int  errno;
#define EMFILE  15
#else
#include    "errno.h"
#endif

#if     0
extern FILE *   fopen();
extern FILE *   freopen();
extern int      fclose();
extern int      fseek();
extern long     ftell();
extern int      fprintf();
extern int      sprintf();
extern int      fputs();
extern char *   fgets();
extern int      ferror();
#endif

#if     1

#include    "string.h"
#include    "stdlib.h"
#include    "time.h"        /* For MODE == STANDARD */

#if     0
#ifdef  __cplusplus
extern "C" char *   memmove( char * dest, const char * src, size_t size);
extern "C" char *   strstr( const char *src, const char * pat);
extern "C" size_t   strcspn( const char * src, const char * cset);
#else
extern char *   memmove( char * dest, const char * src, size_t size);
extern char *   strstr( const char *src, const char * pat);
extern size_t   strcspn( const char * src, const char * cset);
#endif
#endif

#else   /* for the COMPILER which has not the standard header files */

#if     0
typedef unsigned    size_t;
typedef long        time_t;
#endif

#if     0
#define strchr( s, c)       index( (s), (c))
#define strrchr( s, c)      rindex( (s), (c))
#define memcpy( d, s, n)    bcopy( (s), (d), (n))
#define memcmp( s1, s2, n)  bcmp( (s1), (s2), (n))
extern char *   index();
extern char *   rindex();
extern char *   bcopy();
extern int      bcmp();
#else
extern char *   strchr();
extern char *   strrchr();
extern char *   memcpy();
extern int      memcmp();
#endif

extern void     exit();
extern char *   malloc();
extern void     free();
extern char *   realloc();
extern int      strcmp();
extern size_t   strlen();
extern char *   strcpy();
extern char *   strcat();
extern size_t   strcspn();
extern char *   strstr();
extern char *   memmove();
extern time_t   time();
extern char *   ctime();

#if     HOST_HAVE_GETENV
extern char *   getenv();
#endif

#endif  /* declaration of standard library functions        */

/* Functions other than standard.   */
#if     PROTO
#ifdef  __cplusplus
extern "C" {
    int     getopt( int argc, char * const * argv, const char * opts);
    extern int      optind;
    extern char *   optarg;
    char *  stpcpy( char * dest, const char * src);
}
#else   /* #ifndef __cplusplus  */
extern int      getopt( int argc, char * const * argv, const char * opts);
extern int      optind;
extern char *   optarg;
extern char *   stpcpy( char * dest, const char * src);
#endif
#else   /* ! PROTO  */
extern int      getopt();
extern int      optind;
extern char *   optarg;
extern char *   stpcpy();
#endif

/* For debugging malloc systems by kmatsui  */
#if     KMMALLOC && _MEM_DEBUG
#if     __cplusplus
extern "C" {
#endif
#include        "xalloc.h"
#if     __cplusplus
}
#endif
#endif

