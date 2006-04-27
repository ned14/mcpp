/*
 * configed.H
 * Configurations for MCPP using config.h genarated by configure script.
 *
 * 2003/11  Created.                                    by kmatsui
 *
 * 2004/02  Extended multi-byte character handling.     by kmatsui
 *
 * WARNING: These settings assume HOST == TARGET.  In case of HOST
 *      differs from TARGET, you must edit this file here and there.
 */

#define     DATE            "2005/03"               /* Date of mcpp     */

/*
 *  'Target' means the O.S. and the compiler to which cpp is implemented.
 *  'Host' means the O.S. and the compiler with which cpp is compiled.
 */

#include    "config.h"

/* Define host compiler.            */
#ifndef HOST_COMPILER
#define HOST_COMPILER       COMPILER
#endif

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
 *      can change the encoding by -m <encoding> option.
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

#if     SYSTEM == SYS_FREEBSD || SYSTEM == SYS_LINUX
#if     SYSTEM == SYS_FREEBSD
#define SYSTEM_EXT          "__FreeBSD__"
#else   /* SYSTEM == SYS_LINUX  */
#define SYSTEM_EXT          "__linux__"
#endif  /* SYSTEM == SYS_LINUX  */
#endif  /* SYSTEM == SYS_FREEBSD || SYSTEM == SYS_LINUX */

#if     SYSTEM == SYS_CYGWIN
#define SYSTEM_EXT          "__CYGWIN__"
#define SYSTEM_EX2          "__CYGWIN32__"
#ifndef MBCHAR
#define MBCHAR              SJIS
#endif
#endif  /* SYSTEM == SYS_CYGWIN */

#if     SYS_FAMILY == SYS_UNIX
#ifndef MBCHAR
#define MBCHAR              EUC_JP
#endif
#endif

#if     COMPILER == GNUC
#define COMPILER_EXT        "__GNUC__"
#define COMPILER_EXT_VAL    GCC_MAJOR_VERSION
#define COMPILER_EX2        "__GNUC_MINOR__"
#define COMPILER_EX2_VAL    GCC_MINOR_VERSION
#define COMPILER_CPLUS      "__GNUG__"
#define COMPILER_CPLUS_VAL  GCC_MAJOR_VERSION
#ifndef ENV_C_INCLUDE_DIR
#define ENV_C_INCLUDE_DIR   "C_INCLUDE_PATH"
#define ENV_CPLUS_INCLUDE_DIR   "CPLUS_INCLUDE_PATH"
#define ENV_VERSION         "GCC_VERSION"
            /* The name of environment variable to specify GNU C VERSION    */
#endif
/*
 * __SIZE_TYPE__, __PTRDIFF_TYPE__ and __WCHAR_TYPE__ are the predefines of
 * GNU C / cpp and undocumented in GNU C 2..
 */
#if     __GNUC__ == 2
#define COMPILER_SP1        "__SIZE_TYPE__"
#define COMPILER_SP2        "__PTRDIFF_TYPE__"
#define COMPILER_SP3        "__WCHAR_TYPE__"
#endif

#define CMP_NAME            "GNU C"
#endif  /* COMPILER == GNUC */

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

#if     HAVE_LIMITS_H
#include    "limits.h"
#define CHARBIT             CHAR_BIT
#define UCHARMAX            UCHAR_MAX
#define USHRTMAX            USHRT_MAX
#define LONGMAX             LONG_MAX
#if     HAVE_UNSIGNED_LONG
#define ULONGMAX            ULONG_MAX
#endif
#else   /* ! HAVE_LIMITS_H  */
#ifndef CHARBIT
#define CHARBIT             8
#endif
#ifndef UCHARMAX
#define UCHARMAX            255
#endif
#ifndef LONGMAX
#define LONGMAX             2147483647L
#endif
#ifndef ULONGMAX
#if     HAVE_UNSIGNED_LONG
#define ULONGMAX            4294967295UL
#else
#define ULONGMAX            2147483647L
#endif
#endif
#endif  /* HAVE_LIMITS_H    */

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

#define HOST_PROTOTYPES         PROTOTYPES
#define HOST_HAVE_C_BACKSLASH_A HAVE_C_BACKSLASH_A
#define HOST_HAVE_STRING_H      HAVE_STRING_H
#define HOST_HAVE_STDLIB_H      HAVE_STDLIB_H
#define HOST_HAVE_TIME_H        HAVE_TIME_H
#define HOST_HAVE_STRINGS_H     HAVE_STRINGS_H
#define HOST_HAVE_MEMORY_H      HAVE_MEMORY_H
#define HOST_HAVE_MEMMOVE       HAVE_MEMMOVE
#define HOST_HAVE_MEMCPY        HAVE_MEMCPY
#define HOST_HAVE_GETENV        HAVE_GETENV
#define HOST_HAVE_MEMCMP        HAVE_MEMCMP
#define HOST_HAVE_STRSTR        HAVE_STRSTR
#define HOST_HAVE_STRCSPN       HAVE_STRCSPN
#define HOST_HAVE_STPCPY        HAVE_STPCPY
#define HOST_HAVE_SANE_FGETS    HAVE_SANE_FGETS
#define HOST_HAVE_GETOPT        HAVE_GETOPT
#define HOST_LIB_IS_GLIBC       LIB_IS_GLIBC
#define HOST_CMP_NAME           CMP_NAME

#if     HOST_HAVE_C_BACKSLASH_A
        /* 1: for the host compiler-proper which can't recognize \a, \v */
#define ALERT               '\007'          /* '\a' is "Bell"       */
#define VT                  '\013'          /* Vertical Tab CTRL/K  */
#endif

/*
 * PROTO    means that host-compiler permits prototype declarations.
 */
#if     HOST_PROTOTYPES
#define PROTO               TRUE
#else
#ifndef PROTO
#define PROTO               FALSE
#endif
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
#include    "errno.h"

#if     HOST_HAVE_STRING_H
#include    "string.h"
#else
#if     HOST_HAVE_STRINGS_H
#include    "strings.h"
#define strchr( s, c)       index( (s), (c))
#define strrchr( s, c)      rindex( (s), (c))
#define memcpy( d, s, n)    bcopy( (s), (d), (n))
#define memcmp( s1, s2, n)  bcmp( (s1), (s2), (n))
#endif
#ifdef  __cplusplus
extern "C" char *   memmove( char * dest, const char * src, size_t size);
extern "C" char *   strstr( const char *src, const char * pat);
extern "C" size_t   strcspn( const char * src, const char * cset);
#else
extern char *   memmove();
extern char *   strstr();
extern size_t   strcspn();
#endif
#endif

#if     HOST_HAVE_STDLIB_H
#include    "stdlib.h"
#else
#if     HOST_HAVE_MEMORY_H
#include    "memory.h"
#endif
#if     HOST_HAVE_GETENV
extern char *   getenv();
#endif
extern void     exit();
#endif

#if     HOST_HAVE_STDDEF_H
#include    "stddef.h"
#endif

#if     HOST_HAVE_TIME_H
#include    "time.h"        /* For MODE == STANDARD */
#else
#if     ! HOST_HAVE_TIME_T
typedef long    time_t;
#endif
extern time_t   time();
extern char *   ctime();
#endif

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

