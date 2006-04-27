/*-
 * Copyright (c) 1998, 2002-2005 Kiyoshi Matsui <kmatsui@t3.rim.or.jp>
 * All rights reserved.
 *
 * Some parts of this code are derived from the public domain software
 * DECUS cpp (1984,1985) written by Martin Minow.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 *                              M A I N . C
 *                  M C P P   M a i n   P r o g r a m
 */

/*
 * Edit history of DECUS CPP (MM: Martin Minow)
 * 21-May-84    MM      "Field test" release
 * 11-Jul-84    MM      "Official" first release (that's what I thought!)
 * 31-Aug-84    MM      USENET net.sources release.
 *  7-Dec-84    MM      Stuff in Nov 12 Draft Standard
 * 07-Jun-85    KR      Latest revision
 */

/*
 * CPP Version 2.0
 * 1998/08      Re-written according to ISO 9899:1990 and it's Amendment 1,
 *      Corrigendum 1, 2.               by kmatsui (kmatsui@t3.rim.or.jp)
 *
 *      Implemented translation phases precisely.       (support.c)
 *      Revised tokenization according to the Standard and Amendment1.
 *                                                      (support.c & others)
 *      Implemented the pre-defined macros __STDC__, __STDC_VERSION__,
 *          __TIME__, and revised __DATE__.  Made these standard macros can't
 *          be undefined nor redefined.                 (main.c)
 *      Implemented _Pragma() operator.                 (main.c & others)
 *      Revised some non-standard pre-defined macros.   (main.c)
 *      Implemented #error directive.  The error message is output to the
 *          stderr.                                     (control.c)
 *      Implemented #pragma __once directive, -i option and -M* option,
 *          imported from GNU C / cpp.                  (control.c & others)
 *      Implemented #pragma __put_defines, #pragma __debug directives and the
 *          old style directives corresponding to them. (system.c & others)
 *      Made #pragma lines to be output with warning to the stderr for the
 *          compiler which can't recognize the directive.       (system.c)
 *      Made #line argument to be subject to macro expansion.
 *                                                      (control.c, support.c)
 *      Reinforced the test of #define syntax.          (control.c)
 *      Created Standard conforming mode of macro-expansion (including the
 *          processing of #, ## operators).  The Standard mode of expansion is
 *          very different from the pre-Standard modes. (expand.c)
 *      Created "post-Standard" mode of preprocessing, which is a simplified
 *          version of Standard mode.                   (all files)
 *      Simplified CON_FALSE mode corresponding to K&R 1st. specifications.
 *          CON_NOEXPAND, CON_EXPAND modes of the original version are
 *          retained (after revision).  (main.c, control.c eval.c, expand.c)
 *      Revised # operator so as to inserts \ before \ or " in stringized
 *          arguments (except in MBCHAR) in Standard mode.      (expand.c)
 *      Changed the type of #if expression from int to long / unsigned long.
 *          Reinforced expression evaluation.           (eval.c)
 *      Implemented wide character constant, multi-character character
 *          constant, and revised multi-byte character constant in #if
 *          expression.                                 (eval.c)
 *      Revised the handling of MBCHAR in string literal and character
 *          constant.                   (support.c, expand.c, eval.c, main.c)
 *      Supplemented the optional phase for the pre-Standard compiler-proper
 *          to concatenate adjacent string literals, convert '\a' and '\v' to
 *          octals, convert digraphs.                   (main.c)
 *      Implemented the features of C99-1997/11 draft except Unicode-related
 *          features (_Pragma operator, variable arguments of macro, //
 *          comments, long long of #if expression, p+ of pp-number)
 *                                                      (all files)
 *      Supplemented the C++ preprocessor option.       (support.c, system.c)
 *      Refined error checks and diagnostic messages.   (all files)
 *      Implemented -M* option.                         (main.c, system.c)
 *      Updated MS-DOS memory model option.             (system.c)
 *      Revised command line options.                   (system.c)
 *      Made the source files compilable by C++ as well as C.   (all files)
 *      Re-organized and re-written the source files to be portable to many
 *           systems.                                   (all files)
 *
 *      See "cpp_20.man", "cpp_20.doc" and "cpp_test.doc" for details.
 *      (2004/11    Those documents are later renamed as "mcpp-manual.txt"
 *          , "mcpp-porting.txt" and "cpp-test.txt".)
 *
 *      Dependencies among the source files are as follows:
 *          main.c, control.c, eval.c, expand.c, support.c and system.c
 *              depend on system.H and internal.H
 *              system.H should be included prior to internal.H.
 *          lib.c depends on system.H.
 *          lib.c is provided for the library which has not the particular
 *              functions.
 *      You should add to stack size
 *              NMACWORK + (NEXP * 30) + (sizeof (int) * 100)
 *          and for MODE == STANDARD
 *              (sizeof (char *) * 12 * RESCAN_LIMIT)
 *          other than the size needed by the system.
 */

/*
 * CPP Version 2.1
 * 1998/09      kmatsui
 *      Updated C99 features according to 1998/08 draft (including UCN,
 *          optional multi-byte-character in identifier, type of #if
 *          expression in integer of maximum size and concatenation of
 *          wide-character-string-literal and character-string-literal).
 *                                              (main.c, eval.c, support.c)
 */

/*
 * CPP Version 2.2
 * 1998/11      kmatsui
 *      Updated according to C++ Standard (ISO/IEC 14882:1998).
 *                                                      (eval.c, support.c)
 *      Fixed the bug of interaction of predefined non-standard macro with
 *          -D option.              (main.c, control.c, expand.c, system.c)
 */

/*
 * CPP Version 2.3 pre-release 1
 * 2002/08      kmatsui
 *      Updated according to C99 (ISO/IEC 9899:1999).
 *      Added compatibility mode of C++ to C99.         (system.c, expand.c)
 *      Increased the class of warnings from four (OR of 1, 2, 4, 8) to
 *          five (OR of 1, 2, 4, 8, 16).
 *      Changed some errors to warnings.
 *      Changed some options on invoking.               (system.c)
 *      Added the options for compatibility to GNU C / cpp.
 *                                                      (system.c, support.c)
 *      Added implementation for Linux, Cygwin / GNU C, LCC-Win32.
 *                                          (system.H, control.c, system.c)
 *      Fixed a few bugs.                   (control.c, expand.c, support.c)
 *      Renamed functions and some variables using underscore to separate
 *          the two words.                  (internal.H, all the *.c files)
 *
 * CPP Version 2.3 pre-release 2
 * 2002/12      kmatsui
 *      Added implementation for GNU C 3.2.             (system.H, system.c)
 *      Fixed a few bugs.                               (system.c, expand.c)
 *
 * CPP Version 2.3 release
 * 2003/02      kmatsui
 *      Implemented identifier-like operators in C++98.
 *                                          (eval.c, control.c, support.c)
 *      Reinforced checking of __VA_ARGS__.             (control.c)
 *      Enabled interspersed options between filename arguments on
 *          invocation.                                 (system.c)
 *      Renamed #pragma __debug and #pragma __warning to #pragma __debug_cpp
 *          and #pragma __warning_cpp.                  (system.c)
 *      Renamed macros for include directory.           (system.H, system.c)
 *
 * CPP Version 2.3 patch 1
 * 2003/03      kmatsui
 *      Revised the MODEs other than STANDARD.          (system.c)
 */

/*
 * MCPP Version 2.4 prerelease
 * 2003/11      kmatsui
 *      Renamed CPP as MCPP (This is not the name of executable).
 *      Created configure script to MAKE automatically.  Accordingly,
 *          reorganized system.H, created configed.H and unconfig.H, and
 *          changed some macro names.
 *      Implemented dirname/filename/line information of defined macros,
 *      changed DEFBUF and FILEINFO structure, reorganized some functions and
 *          variables.                  (other than system.H, eval.c lib.c)
 *      Added #pragma __push_macro, #pragma __pop_macro, #pragma __preprocess,
 *          #pragma __preprocessed.                     (system.c)
 *      Added implementation for Visual C++             (system.H, system.c)
 *      Removed settings on VMS, VAX C, DEC C and OS-9/09.
 *                                                      (system.H, system.c)
 *      Removed CON_NOEXPAND and CON_EXPAND modes.  Renamed CON_FALSE mode as
 *          PRE_STANDARD.                               (all the files)
 *
 * MCPP Version 2.4 release
 * 2004/02      kmatsui
 *      Implemented handling of multi-byte character encodings other than
 *          2-byte encodings.  Made various encodings available
 *          simultaneously.  Created mbchar.c.  Added #pragma __setlocale.
 *          Added -m <encoding> option.  Enabled environment variable LC_ALL,
 *          LC_CTYPE and LANG to specify the encoding.  (all the files)
 *      Added implementation for Plan 9 / pcc.          (noconfig.H, system.c)
 *
 * MCPP Version 2.4.1
 * 2004/03      kmatsui
 *      Added -c option (compatible mode to GNU C expansion of recursive
 *          macro).                                     (expand.c, system.c)
 */

/*
 * MCPP Version 2.5
 * 2005/03      kmatsui
 *      Absorbed POST_STANDARD into STANDARD as an execution time mode
 *          (POST_STD mode).                            (all the files)
 *      Absorbed OLD_PREPROCESSOR into PRE_STANDARD as an execution time
 *          mode (OLD_PREP mode).                       (all the files)
 *      Revised STD mode macro expansion routine using GNU C 3.2 testsuite
 *          and Wave 1.0 testcases.                     (expand.c)
 *      Revised OLD_PREP mode to follow "Reiser cpp model".   (control.c)
 *      Removed FOLD_CASE settings.                     (system.c)
 *      Renamed most of #pragma __* as #pragma MCPP *.  (system.c)
 *      Updated to cope with GNU C V.3.3 and 3.4, and changed some options.
 *                                                      (system.c)
 */

/*
 * CPP Version 2.0 / main.c
 * 1998/08      kmatsui
 *      Renamed cpp1.c main.c.
 *      Created do_pragma_op(), de_stringize(), devide_line(), putout(),
 *          putline(), post_preproc(), conv_esc(), conv2oct(), is_last_esc(),
 *          esc_mbchar(), conv_a_digraph().
 *      Removed output().
 *      Moved sharp() from cpp1.c to system.c,
 *          addfile(), openfile(), initdefines(), unpredefine()
 *              from cpp3.c to main.c,
 *      Revised most of the functions and variables.
 */

/*
 * CPP Version 2.1 / main.c
 * 1998/09      kmatsui
 *      Revised several functions.
 */

/*
 * CPP Version 2.2 / main.c
 * 1998/11      kmatsui
 *      Created undef_a_predef().
 *      Revised several functions.
 */

/*
 * CPP Version 2.3 pre-release 1 / main.c
 * 2002/08      kmatsui
 *      Implemented __STDC_HOSTED__ pre-defined macro.
 *      Added "stdc3" variable for the -V option.
 *      Added "lang_asm" variable for the -a (-lang-asm,
 *          -x assembler-with-cpp) option.
 *      Renamed several functions using underscore.
 *
 * CPP Version 2.3 release / main.c
 * 2003/02      kmatsui
 */

/*
 * MCPP Version 2.4 prerelease / main.c
 * 2003/11      kmatsui
 *      Moved open_file() and add_file() from main.c to system.c.
 *      Added the following variables for dir/filename information:
 *          null, inc_dirp, cur_fname, cur_fullname[].
 *      Revised main(), cur_file() and init_defines().
 *      Changed predefined macro __decus_cpp as __MCPP.
 *
 * MCPP Version 2.4 release / main.c
 * 2004/02      kmatsui
 *      Added the following variables for multi-byte character handling.
 *          mbchar, mbstart, mbmask, bsl_in_mbchar, bsl_need_escape.
 *      Changed some macros and updated esc_mbchar().
 */

/*
 * MCPP Version 2.5 / main.c
 * 2005/03      kmatsui
 *      Absorbed POST_STANDARD into STANDARD and OLD_PREPROCESSOR into
 *          PRE_STANDARD.
 */

/*
 * The main routine and it's supplementary routines are placed here.
 * The post-preprocessing routines are also placed here.
 */

#if PREPROCESSED                /* Use "pre-preprocessed" header    */
#include    "mcpp.H"
#else
#include    "system.H"
#include    "internal.H"
#endif

    int     mode = 0;               /* Mode of preprocessing        */
    int     cflag = FALSE;          /* -C option (keep comments)    */
    int     zflag = FALSE;      /* -i option (no output of included file)   */
    int     pflag = FALSE;          /* -P option (no #line output)  */
    int     qflag = FALSE;      /* -Q option (diagnostics to "mcpp.err")    */
#if MODE == STANDARD
#if OK_TRIGRAPHS
    int     tflag = TFLAG_INIT;     /* -3 option (trigraphs)        */
#endif
#if OK_DIGRAPHS
    int     digraphs = DIGRAPHS_INIT;       /* -2 option (digraphs) */
#endif
    long    cplus = 0L;             /* Value of __cplusplus for C++ */
    long    stdc_ver = 0L;          /* Value of __STDC_VERSION__    */
    int     stdc_val = STDC;        /* Value of __STDC__            */
    int     stdc2;                  /* cplus || stdc_ver >= 199901L */
    int     stdc3;              /* cplus >= 199901L || stdc_ver >= 199901L.
        (cplus >= 199901L) specifies compatibility to C99 (extended feature
        of this cpp)    */
#endif
/*
 * lang_asm allows the following non-standard features.
 * 1. #non-directive.
 * 2. <newline> in a string-literal.
 * 3. invalid pp-token generated by ## operator.
 * lang_asm is not available in POST_STD mode.
 */
    int     lang_asm = FALSE;       /* -a option (assembler source) */
    int     std_line_prefix = STD_LINE_PREFIX;
            /* Output line and file information in C source style   */

#if MODE == STANDARD
/*
 * Translation limits specified C90, C99 or C++.
 */
    /* The following three values are temporarily set for do_options()      */
    long    str_len_min = NBUFF;    /* Least maxmum of string len.  */
    size_t  id_len_min = IDMAX;     /* Least maximum of ident len.  */
    int     n_mac_pars_min = NMACPARS;  /* Least maximum of num of params.  */
    int     exp_nest_min;           /* Least maximum of expr nest   */
    int     blk_nest_min;           /* Least maximum of block nest  */
    int     inc_nest_min;           /* Least maximum of include nest*/
    long    n_macro_min;        /* Least maximum of num of macros   */

    long    line_limit;             /* Maximum source line number   */
#endif

/*
 * Commonly used global variables:
 * line         is the current input line number.
 * wrong_line   is set in many places when the actual output line is out of
 *              sync with the numbering, e.g, when expanding a macro with an
 *              embedded newline.
 * identifier   holds the last identifier scanned (which might be a candidate
 *              for macro expansion).
 * errors       is the running mcpp error counter.
 * infile       is the head of a linked list of input files (extended by
 *              #include and macros being expanded).  'infile' always points
 *              to the current file/macro.  'infile->parent' to the includer,
 *              etc.  'infile->fp' is NULL if this input stream is a macro.
 * inc_dirp     Directory of #includer with trailing PATH_DELIM.  This points
 *              to one of incdir[] or to the current directory (represented as
 *              "".  This should not be NULL.
 * flbuf        is used for diagnostics on macro call.
 */
    long        line;               /* Current line number          */
    int         wrong_line;         /* Force #line to compiler      */
    int         errors = 0;         /* Cpp error counter            */
    int         warn_level = -1;    /* Level of warning (have to initialize)*/
    FILEINFO *  infile = NULL;      /* Current input file           */
    int         include_nest;       /* Nesting level of #include    */
    const char *    null = "";      /* "" string for convenience    */
    const char **   inc_dirp;       /* Directory of #includer       */
    const char *    cur_fname;      /* Current source file name     */
                /* cur_fname is not rewritten by #line directive    */
    char        cur_fullname[ FILENAMEMAX + 1];
        /* Full name of current source file (i.e. *inc_dirp/cur_fname)      */
    int         no_source_line;     /* Do not output line in diag.  */
    char        identifier[ IDMAX + IDMAX/8];       /* Current identifier   */
#if DEBUG || DEBUG_EVAL
    int         debug = 0;          /* != 0 if debugging now        */
#endif

/*
 *   in_directive is set TRUE while a control line is scanned by control().
 * It modifies the behavior of squeeze_ws() in expand.c so that newline is
 * not skipped even if getting macro arguments.
 */
    int     in_directive = FALSE;   /* TRUE scanning control line   */
    int     in_define = FALSE;      /* TRUE scanning #define line   */
#if MODE == STANDARD
    int     in_getarg;              /* TRUE collecting macro arguments      */
#endif
#if MODE == PRE_STANDARD
    long    in_asm = 0L;    /* Starting line of #asm - #endasm block*/
#endif

/*
 *   macro_line is set to the line number of start of a macro call while
 * expanding the macro, else set to 0.  Line number is remembered for
 * diagnostics of unterminated macro call.  On unterminated macro call
 * macro_line is set to MACRO_ERROR.
 */
    long    macro_line = 0L;

#if MODE == STANDARD
/*
 *   compat_mode is set to TRUE, if recursive macro call is expanded more
 * than Standard's specification.  This mode is compatible to GNU C and
 * some other implementations.
 */
    int     compat_mode = FALSE;
#endif

/*
 * openum is the return value of scan_op() in support.c.
 */
    int     openum;

/*
 *   mkdep means to output source file dependency line, specified by -M*
 * option.  The OR of the following values is used.
 *      MD_MKDEP    (1) :   Output dependency line.
 *      MD_SYSHEADER(2) :   Print also system headers or headers with
 *          absolute path not only user headers.
 *      MD_FILE     (4) :   Output to the file named *.d instead of fp_out.
 *          Normal output is done to fp_out as usual.
 */
#if OK_MAKE
    int     mkdep = 0;
#endif

/*
 *   If zflag is TRUE, no_output is incremented when a file is #included,
 * and decremented when the file is finished.
 * If no_output is larger than 0, processed files are not output, meanwhile
 * the macros in the files are defined.
 *   If mkdep != 0 && (mkdep & MD_FILE) == 0, no_output is set to 1 initially.
 */
    int     no_output = 0;

/*
 * keep_comments is set TRUE by the -C option.  If TRUE, comments are written
 * directly to the output stream.  This is needed if the output from cpp is
 * to be passed to lint (which uses commands embedded in comments).  cflag
 * contains the permanent state of the -C flag.  keep_comments is always
 * falsified when compilation is supressed by a false #if or when no_output
 * is TRUE.
 */
    int     keep_comments;              /* Write out comments flag  */

/*
 * ifstack[] holds information about nested #if's.  It is always accessed via
 * ifptr->stat.  The information is as follows:
 *      WAS_COMPILING   state of compiling flag at outer level.
 *      ELSE_SEEN       set TRUE when #else seen to prevent 2nd #else.
 *      TRUE_SEEN       set TRUE when #if or #elif succeeds
 * ifstack[0].stat holds the compiling flag.  It is WAS_COMPILING if compila-
 * tion is currently enabled.  Note that this must be initialized to
 * WAS_COMPILING.
 */
    IFINFO      ifstack[ BLK_NEST + 1] = { {WAS_COMPILING, 0L, 0L}, };
                /* Note: '+1' is necessary for the initial state.   */
    IFINFO *    ifptr = ifstack;        /* -> current ifstack[]     */

#if MODE == STANDARD
/* In POST_STD mode, insert_sep is set to INSERT_SEP when :
 *  1. the next get() shall insert a token separator.
 *  2. unget() has been called when insert_sep == INSERTED_SEP.
 * set to INSERTED_SEP when :
 *  get() has been called when insert_sep == INSERT_SEP.
 * set to NO_SEP when :
 *  get() has been called when insert_sep == INSERTED_SEP.
 */
    int     insert_sep = NO_SEP;
#endif

#if MODE == STANDARD
/* has_pragma is set to TRUE so as to execute _Pragma() operator when the
 * psuedo macro _Pragma() is found.
 */
    int     has_pragma = FALSE;
#endif

/* File pointers for input and output.  */
    FILE *  fp_in;                  /* Input stream to preprocess   */
    FILE *  fp_out;                 /* Output stream preprocessed   */
    FILE *  fp_err;                 /* Diagnostics stream           */
    FILE *  fp_debug;               /* Debugging information stream */

/* Variables on multi-byte character encodings. */
    int     mbchar = MBCHAR;        /* Encoding of multi-byte char  */
    int     mbmask;                 /* Char type other than mbchar  */
    int     mbstart;                /* 1st byte of mbchar (or shift)*/
#if BSL_IN_MBCHAR
    int     bsl_in_mbchar;          /* 2nd byte of mbchar has '\\'  */
    int     bsl_need_escape;    /* '\\' in MBCHAR should be escaped */
#endif
/* Function pointer to mb_read_*() functions.   */
    size_t  (*mb_read)( int c1, char ** in_pp, char ** out_pp);

/*
 * work[] and workp are used to store one piece of text in a temporary buffer.
 * To initialize storage, set workp = work.  Note that the work buffer is used
 * by several subroutines -- be sure that your data won't be overwritten.
 * work[] is used for:
 *      1. temporary buffer in macro expansion (exp_special(), expand(),
 *         catenate())
 *      2. temporary buffer in processing control line.
 */
    char        work[ NWORK + IDMAX];   /* Work buffer              */
    char *      workp;                  /* Pointer into work[]      */
    char * const     work_end = & work[ NWORK];
                                        /* End of buffer work[]     */

#define MBCHAR_IS_ESCAPE_FREE   (SJIS_IS_ESCAPE_FREE && \
            BIGFIVE_IS_ESCAPE_FREE && ISO2022_JP_IS_ESCAPE_FREE)

#if PROTO

static void     cur_file( void);
static void     init_defines( void);
static void     mcpp_main( void);
static void     putout( char * out);
static void     put_a_line( const char * out);
#if MODE == STANDARD && OK_PRAGMA_OP
static void     do_pragma_op( void);
static void     put_seq( char * begin, char * seq);
static char *   de_stringize( char * in, char * out);
#endif
#if NWORK < NMACWORK
static void     devide_line( void);
#endif
#if CONCAT_STRINGS  \
        || (MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS) \
        || (BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE) || ! HAVE_C_BACKSLASH_A
static int      post_preproc( char * out);
#endif
#if ! HAVE_C_BACKSLASH_A
static char *   conv_esc( char * cp);
#endif
#if CONCAT_STRINGS
static int      is_last_esc( char * cp);
static char *   conv2oct( char * in, char * out);
#endif
#if MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS
static char *   conv_a_digraph( char * cp);
#endif
#if BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE
static char *   esc_mbchar( char * str, char * str_end);
#endif

#else   /* ! PROTO  */

static void     cur_file();     /* Print source file name           */
static void     init_defines(); /* Predefine macros                 */
static void     mcpp_main();    /* Main loop to process input lines */
static void     putout();       /* May concatenate adjacent string  */
static void     put_a_line();   /* Put out the processed line       */
#if MODE == STANDARD && OK_PRAGMA_OP
static void     do_pragma_op(); /* Exucute the _Pragma() operator   */
static void     put_seq();      /* Put out the failed sequence      */
static char *   de_stringize(); /* "De-stringize" for _Pragma() op. */
#endif
#if NWORK < NMACWORK
static void     devide_line();  /* Devide long line for compiler    */
#endif
#if CONCAT_STRINGS  \
        || (MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS) \
        || (BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE) || ! HAVE_C_BACKSLASH_A
static int      post_preproc(); /* Post-preprocess for older comps  */
#else
#if BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE || ! HAVE_C_BACKSLASH_A
static int      post_preproc(); /* Post-preprocess for older comps  */
#endif
#endif
#if ! HAVE_C_BACKSLASH_A
static char *   conv_esc();     /* Convert '\a' and '\v' to octals  */
#endif
#if CONCAT_STRINGS
static int      is_last_esc();  /* String end with a hex or octal ? */
static char *   conv2oct();     /* Convert hexdigit char to octal   */
#endif
#if MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS
static char *   conv_a_digraph();   /* Convert a digraph in place   */
#endif
#if BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE
static char *   esc_mbchar();   /* Insert \ before 2nd byte of SJIS */
#endif

#endif  /* ! PROTO  */


int
#if PROTO
main( int argc, char ** argv)
#else
main( argc, argv)
    int     argc;
    char ** argv;
#endif
{
    char *  in_file = NULL;
    char *  out_file = NULL;

    fp_in = stdin;
    fp_out = stdout;
    fp_err = stderr;
    fp_debug = stdout;
        /*
         * Debugging information is output to stdout in order to
         *      synchronize with preprocessed output.
         */

    inc_dirp = &null;   /* Initialize to current (null) directory   */
    cur_fname = "(predefined)";     /* For predefined macros        */
    init_defines();                         /* Predefine macros     */
    mb_init();      /* Should be initialized prior to get options   */
    do_options( argc, argv, &in_file, &out_file);   /* Command line options */

    /* Open input file, "-" means stdin.    */
    if (in_file != NULL && ! str_eq( in_file, "-")) {
        if (freopen( in_file, "r", fp_in) == NULL) {
            fprintf( fp_err, "Can't open input file \"%s\".\n", in_file);
            exit( IO_ERROR);
        }
        strcpy( work, in_file);     /* Remember input filename      */
    } else {
        strcpy( work, "<stdin>");
    }
    /* Open output file, "-" means stdout.  */
    if (out_file != NULL && ! str_eq( out_file, "-")) {
        if (freopen( out_file, "w", fp_out) == NULL) {
            fprintf( fp_err, "Can't open output file \"%s\".\n", out_file);
            exit( IO_ERROR);
        }
    }
    if (qflag) {                            /* Redirect diagnostics */
        if (freopen( "mcpp.err", "a", fp_err) == NULL) {
            fprintf( fp_out, "Can't open \"mcpp.err\"\n");
            exit( IO_ERROR);
        }
    }
    add_file( fp_in, work);         /* "open" main input file       */
    infile->dirp = inc_dirp;
    strcpy( cur_fullname, work);
#if OK_MAKE
    if (mkdep && str_eq( infile->filename, "<stdin>") == FALSE)
        put_depend( work);          /* Putout target file name      */
#endif
    at_start();                     /* Do the pre-main commands     */

    mcpp_main();                    /* Process main file            */

#if OK_MAKE
    if (mkdep)
        put_depend( NULLST);    /* Append '\n' to dependency line   */
#endif

    at_end();                       /* Do the final commands        */

    if (errors > 0 && no_source_line == FALSE) {
        fprintf( fp_err, "%d error%s in preprocessor.\n",
                errors, (errors == 1) ? "" : "s");
        exit( IO_ERROR);
    }
    exit( IO_SUCCESS);              /* No errors or -E option set   */
    return  IO_SUCCESS;             /* Never reach here             */
}

void
#if PROTO
sharp( void)
#else
sharp()
#endif
/*
 * Output a line number line.
 */
{
    if (no_output || pflag || infile == NULL)
        goto  sharp_exit;
    if (keep_comments)
        fputc( '\n', fp_out);           /* Ensure to be on line top */
    if (std_line_prefix)
        fprintf( fp_out, "#line %ld", line);
    else
        fprintf( fp_out, "%s%ld", LINE_PREFIX, line);
    cur_file();
    fputc( '\n', fp_out);
sharp_exit:
    wrong_line = FALSE;
}

static void
#if PROTO
cur_file( void)
#else
cur_file()
#endif
/*
 * Output a current source file name.
 */
{
    static char *   sharp_filename = NULL;
    register const char *     name = "";
    char *  cp;

    if (infile->fp != NULL) {
        cp = stpcpy( work, *(infile->dirp));
        strcpy( cp, infile->filename);
        name = work;
        if (sharp_filename == NULL || ! str_eq( name, sharp_filename)) {
            if (sharp_filename != NULL)
                free( sharp_filename);
            sharp_filename = save_string( name);
        }
    }
    if (! no_output)
        fprintf( fp_out, " \"%s\"", name);
}

/*
 * This is the table used to predefine target machine, operating system and
 * compiler designators.  It may need hacking for specific circumstances.
 * The -N option supresses preset definitions.
 */
typedef struct pre_set {
    const char *    name;
    const char *    val;
} PRESET;

static PRESET   preset[] = {

#ifdef  CPU_OLD
        { CPU_OLD, "1"},
#endif
#ifdef  CPU_SP_OLD
        { CPU_SP_OLD, "1"},
#endif
#ifdef  SYSTEM_OLD
        { SYSTEM_OLD, "1"},
#endif
#ifdef  SYSTEM_SP_OLD
        { SYSTEM_SP_OLD, "1"},
#endif
#ifdef  COMPILER_OLD
        { COMPILER_OLD, "1"},
#endif
#ifdef  COMPILER_SP_OLD
        { COMPILER_SP_OLD, "1"},
#endif

        { NULL, NULL},  /* End of macros beginning with alphabet    */

#ifdef  CPU_STD
        { CPU_STD, "1"},
#endif
#ifdef  CPU_STD1
        { CPU_STD1, "1"},
#endif
#ifdef  CPU_STD2
        { CPU_STD2, "1"},
#endif
#ifdef  SYSTEM_STD
        { SYSTEM_STD, "1"},
#endif
#ifdef  SYSTEM_STD1
        { SYSTEM_STD1, "1"},
#endif
#ifdef  SYSTEM_STD2
        { SYSTEM_STD2, "1"},
#endif

#ifdef  SYSTEM_EXT
        { SYSTEM_EXT, SYSTEM_EXT_VAL},
#endif
#ifdef  SYSTEM_EX2
        { SYSTEM_EX2, SYSTEM_EX2_VAL},
#endif
#ifdef  SYSTEM_SP_STD
        { SYSTEM_SP_STD, SYSTEM_SP_STD_VAL},
#endif
#ifdef  COMPILER_STD
        { COMPILER_STD, COMPILER_STD_VAL},
#endif
#ifdef  COMPILER_STD1
        { COMPILER_STD1, COMPILER_STD1_VAL},
#endif
#ifdef  COMPILER_STD2
        { COMPILER_STD2, COMPILER_STD2_VAL},
#endif
#ifdef  COMPILER_EXT
        { COMPILER_EXT, COMPILER_EXT_VAL},
#endif
#ifdef  COMPILER_EX2
        { COMPILER_EX2, COMPILER_EX2_VAL},
#endif
#ifdef  COMPILER_SP_STD
        { COMPILER_SP_STD, COMPILER_SP_STD_VAL},
#endif
#ifdef  COMPILER_SP1
        { COMPILER_SP1, COMPILER_SP1_VAL},
#endif
#ifdef  COMPILER_SP2
        { COMPILER_SP2, COMPILER_SP2_VAL},
#endif
#ifdef  COMPILER_SP3
        { COMPILER_SP3, COMPILER_SP3_VAL},
#endif
#if     MODE == STANDARD
#ifdef  COMPILER_CPLUS
        { COMPILER_CPLUS, COMPILER_CPLUS_VAL},
#endif
#endif
        { NULL, NULL},  /* End of macros with value of any integer  */
};

static void
#if PROTO
init_defines( void)
#else
init_defines()
#endif
/*
 * Initialize the built-in #define's.  There are two flavors:
 *      #define __MCPP     2           (static definitions)
 *      #define __FILE__        ??          (dynamic, evaluated by magic)
 * Called only on cpp startup.
 *
 * Note: the built-in static definitions are supressed by the -N option,
 * definitions beginning with alphabet are supressed by the -S1 option,
 * __LINE__, __FILE__, __DATE__, __TIME__, __STDC__ , __STDC_HOSTED__ and
 * __MCPP are always present (if defined).
 * __STDC_VERSION__ and __cplusplus are defined by do_options() after
 * command-line scanning.
 */
{
#if MODE == STANDARD
    char    tmp[ 16];
    char    timestr[ 14];
    time_t  tvec;
    char *  tstring;
#endif
    int     n = sizeof preset / sizeof (PRESET);
    register PRESET *   pp;

    /* Predefine the built-in symbols.  */
    for (pp = preset; pp < preset + n; pp++) {
        if (pp->name && *(pp->name))
            look_and_install( pp->name, DEF_NOARGS - 1, null, pp->val);
    }

#if MODE == STANDARD
/*
 * The magic pre-defines (Standard predefined macros) are initialized with
 * negative argument counts.  expand() notices this and calls the appropriate
 * routine.  DEF_NOARGS is one greater than the first "magic" definition.
 * 'DEF_NOARGS - n' is reserved for the pre-defined macros.
 */
    look_and_install( "__LINE__", DEF_NOARGS - 3, null, "-1234567890");
    /* Room for 11 chars (10 for long and 1 for '-' in case of wrap round.  */
    look_and_install( "__FILE__", DEF_NOARGS - 4, null, null);
                                            /* Should be stuffed    */

    /*
     * Define __DATE__, __TIME__ as present date and time.
     */
    time( &tvec);
    tstring = ctime( &tvec);
    sprintf( timestr, "\"%.3s %c%c %.4s\"",
        tstring + 4,
        *(tstring + 8) == '0' ? ' ' : *(tstring + 8),
        *(tstring + 9),
        tstring + 20);
    look_and_install( "__DATE__", DEF_NOARGS - 2, null, timestr);
    sprintf( timestr, "\"%.8s\"", tstring + 11);
    look_and_install( "__TIME__", DEF_NOARGS - 2, null, timestr);

/* Define __STDC__ as 1 or such for Standard conforming compiler.   */
    sprintf( tmp, "%d", STDC);
    look_and_install( "__STDC__", DEF_NOARGS - 2, null, tmp);
    sprintf( tmp, "%d", STDC_HOSTED);
    look_and_install( "__STDC_HOSTED__", DEF_NOARGS - 1, null, tmp);
    /*
     * Some compilers, e.g. GNU C older than 3.3, define this macro by
     * -D option.
     */
#endif  /* MODE == STANDARD */
/* Not define __STDC__ for pre-Standard compiler.   */

    look_and_install( "__MCPP", DEF_NOARGS - 1, null, "2");
    /* MCPP V.2.x   */
    /* This macro is predefined yet can be undefined by -U or #undef.   */
}

void
#if PROTO
un_predefine( int clearall)
#else
un_predefine( clearall)
    int     clearall;                       /* TRUE for -N option   */
#endif
/*
 * Remove predefined symbols from the symbol table.
 */
{
    register PRESET *   pp;
    DEFBUF *    defp;
    int     n = sizeof preset / sizeof (PRESET);

    for (pp = preset; pp < preset + n; pp++) {
        if (pp->name) {
            if (*(pp->name) && (defp = look_id( pp->name)) != NULL
                    && defp->nargs == DEF_NOARGS - 1)
                undefine( pp->name);
        } else if (clearall == FALSE) {     /* -S<n> option         */
            break;
        }
    }
}

void
#if PROTO
undef_a_predef( const char * name)
#else
undef_a_predef( name)
    char *  name;
#endif
/*
 * Remove a predefined name from the preset[] table so that the name can be
 * redefined by -D option.
 * The strange ordering (insert, command-line-scan, remove)
 * is needed to avoid interaction with -D arguments.
 */
{
    register PRESET *   pp;
    int     n = sizeof preset / sizeof (PRESET);

    for (pp = preset; pp < preset + n; pp++) {
        if (pp->name && *(pp->name) && str_eq( pp->name, name)) {
            pp->name = "";
            break;
        }
    }
}

/*
 * These definitions are used by the routines of string concatenation.
 */
#if CONCAT_STRINGS
#define DONE            0
#define SUSPEND         1
#define FLUSH           (-1)
#endif

/*
 * output[] and out_ptr are used for:
 *      buffer to store preprocessed line (this line is put out or handed to
 *      post_preproc() via putout() for string concatenation)
 */
static char     output[ NMACWORK];  /* Buffer for preprocessed line */
static     char * const out_end = & output[ NWORK - 2];
                                    /* Limit of output line         */
static     char * const out_wend = & output[ NMACWORK - 2];
                                    /* Buffer end of output line    */
static     char *       out_ptr;    /* Current pointer into output[]*/
#if CONCAT_STRINGS
static char     catbuf[ NWORK + IDMAX + 4]; /* Buffer for string catenation */
static     char *       catbufp = catbuf;   /* Pointer into catbuf[]*/
static     char * const catbuf_end = & catbuf[ NWORK + IDMAX];
static     int          catflag;    /* Flag returned from post_preproc()    */
#endif

static void
#if PROTO
mcpp_main( void)
#else
mcpp_main()
#endif
/*
 * Main process for mcpp -- copies tokens from the current input stream
 * (main file or included file) to the output file.
 */
{
    register int    c;              /* Current character            */
    int     newlines;               /* Blank lines and control lines*/
    char *  wp;                     /* Temporary pointer            */
    DEFBUF *    defp;               /* Macro definition             */

    if (! no_output) {  /* Explicitly output a #line at the start of cpp    */
        line++;
        sharp();
        line--;
    }
    keep_comments = cflag && !no_output;

    /*
     * This loop is started "from the top" at the beginning of each line.
     * 'wrong_line' is set TRUE in many places if it is necessary to write
     * a #line record.  (But we don't write them when expanding macros.)
     *
     * 'newlines' variable counts the number of blank lines that have been
     * skipped over.  These are then either output via #line records or
     * by outputting explicit blank lines.
     */
    while (1) {                             /* For the whole file   */
        newlines = 0;                       /* Count empty lines    */

        while (1) {                         /* For each line, ...   */
            c = get();                      /* First of the line    */
            out_ptr = output;               /* Top of the line buf  */
            if (c == ' ') {         /* Dosen't occur in POST_STD    */
                *out_ptr++ = ' ';           /* Retain a space       */
                c = get();          /* First of token (else '\n')   */
            }
#if MODE == PRE_STANDARD
            if (mode == OLD_PREP && c == COM_SEP)
                 c = get();                 /* Skip 0-length comment*/
#endif
            if (c == '#') {                 /* Is 1st non-space '#' */
                newlines = control( newlines);      /* Do a #command*/
#if MODE == STANDARD && OK_DIGRAPHS
            } else if (mode == STD && digraphs && c == '%') {
                    /* In POST_STD digraphs are already converted   */
                if (get() == ':') {         /* '%:' i.e. '#'        */
                    newlines = control( newlines);  /* Do a #command*/
                } else {
                    unget();
                    if (! compiling) {
                        skip_nl();
                        newlines++;
                    } else {
                        break;
                    }
                }
#endif
            } else if (c == CHAR_EOF) {     /* End of input         */
                break;
            } else if (! compiling) {       /* #ifdef false?        */
                skip_nl();                  /* Skip to newline      */
                newlines++;                 /* Count it, too.       */
#if MODE == PRE_STANDARD
            } else if (in_asm && ! no_output) { /* In #asm block    */
                put_asm();                  /* Put out as it is     */
#endif
            } else if (c == '\n') {         /* Blank line           */
                if (keep_comments)
                    fputc( '\n', fp_out);   /* May flush comments   */
                else
                    newlines++;             /* Wait for a token     */
            } else {
                break;                      /* Actual token         */
            }
        }

        if (c == CHAR_EOF)                  /* Exit process at      */
            break;                          /*   end of input       */

        /*
         * If the loop didn't terminate because of end of file, we
         * know there is a token to compile.  First, clean up after
         * absorbing newlines.  newlines has the number we skipped.
         */
#if CONCAT_STRINGS
        if (catflag != SUSPEND)
#endif
        {
            if (no_output) {
                wrong_line = FALSE;
            } else {
                if (wrong_line || newlines > 10) {
                    sharp();                /* Output # line number */
                } else {                    /* If just a few, stuff */
                    while (newlines-- > 0)  /* them out ourselves   */
                        fputc('\n', fp_out);
                }
            }
        }

        /*
         * Process each token on this line.
         */
        while (c != '\n' && c != CHAR_EOF) {    /* For the whole line   */
            if (scan_token( c, (wp = out_ptr, &wp), out_wend) == NAM
                    && (defp = is_macro( &wp)) != NULL) {   /* A macro  */
                wp = expand( defp, out_ptr, out_wend);
                                            /* Expand it completely */
#if MODE == STANDARD && OK_PRAGMA_OP
                if (has_pragma) {           /* Found _Pramga()      */
                    do_pragma_op();         /* Do _Pragma() operator*/
                    has_pragma = FALSE;     /* Reset signal         */
                    out_ptr = output;       /* Do the rest of line  */
                    wrong_line = TRUE;      /* Line-num out of sync */
                } else
#endif
                {
                    out_ptr = wp;
                }
            } else {                        /* Not a macro call     */
                out_ptr = wp;               /* Advance the place    */
                if (wrong_line)             /* is_macro() swallowed */
                    break;                  /*      the newline     */
            }
#if NWORK < NMACWORK
            if (out_end <= out_ptr)         /* Too long line        */
                devide_line();              /* Devide long line     */
#endif
            if ((c = get()) == ' ') {       /* Token separator      */
                *out_ptr++ = ' ';
                c = get();                  /* First of token       */
            }
#if PRE_STANDARD
            if (mode == OLD_PREP && c == COM_SEP)
                c = get();                  /* Skip 0-length comment*/
#endif
        }                                   /* Line for loop        */

        putout( output);                    /* Output the line      */
    }                                       /* Continue until EOF   */

#if CONCAT_STRINGS
    if (catflag == SUSPEND)                 /* Input end with string*/
        put_a_line( catbuf);
#endif
}

#if MODE == STANDARD && OK_PRAGMA_OP

static void
#if PROTO
do_pragma_op( void)
#else
do_pragma_op()
#endif
/*
 * Execute the _Pragma() operator contained in an expanded macro.
 * Note: _Pragma() operator is also implemented as a special macro.  Therefore
 *      it is always searched as a macro.
 * There might be more than one _Pragma() in a expanded macro and those may be
 *      surrounded by other token sequences.
 * Since all the macros have been expanded completely, any name identical to
 *      macro should not be re-expanded.
 */
{
    FILEINFO *  file;
    DEFBUF *    defp;
    int     prev = output < out_ptr;        /* There is a previous sequence */
    int     token_type;
    char *  cp1, * cp2;
    register int    c;

    file = unget_string( out_ptr, NULLST);
    while (c = get(), file == infile) {
        if (c == ' ') {
            *out_ptr++ = ' ';
            continue;
        }
        if (scan_token( c, (cp1 = out_ptr, &cp1), out_wend)
                    == NAM && (defp = is_macro( &cp1)) != NULL
                && defp->nargs == DEF_PRAGMA) {     /* _Pragma() operator   */
            if (prev) {
                putout( output);    /* Putout the previous sequence */
                sharp();
                cp1 = stpcpy( output, "pragma ");   /* From top of buffer   */
            }
            *cp1++ = get();                                 /* '('  */
            while ((c = get()) == ' ')
                *cp1++ = ' ';
            if (((token_type = scan_token( c, (cp2 = cp1, &cp1), out_wend))
                    != STR && token_type != WSTR)) {
                /* Not a string literal */
                put_seq( output, cp1);
                return;
            }
            workp = de_stringize( cp2, work);
            while ((c = get()) == ' ')
                *cp1++ = ' ';
            if (c != ')') {         /* More than a string literal   */
                unget();
                put_seq( output, cp1);
                return;
            }
            strcpy( workp, "\n");       /* Terminate with <newline> */
            unget_string( work, NULLST);
            do_pragma();                /* Do the #pragma "line"    */
            infile->bptr += strlen( infile->bptr);      /* Clear sequence   */
            sharp();
            cp1 = out_ptr = output;     /* From the top of buffer   */
            prev = FALSE;
        } else {                        /* Not pragma sequence      */
            out_ptr = cp1;
            prev = TRUE;
        }
    }
    unget();
    if (prev)
        putout( output);
    sharp();
}

static void
#if PROTO
put_seq( char * begin, char * seq)
#else
put_seq( begin, seq)
    char *  begin;                  /* Sequence already in buffer   */
    char *  seq;                    /* Sequence to be read          */
#endif
/*
 * Put out the failed sequence as it is.
 */
{
    FILEINFO *  file = infile;
    register int    c;

    cerror( "Operand of _Pragma() is not a string literal"  /* _E_  */
            , NULLST, 0L, NULLST);
    while (c = get(), file == infile)
        *seq++ = c;
    unget();
    out_ptr = seq;
    putout( begin);
}

static char *
#if PROTO
de_stringize( char * in, char * out)
#else
de_stringize( in, out)
    char *  in;                 /* Null terminated string literal   */
    char *  out;                            /* Output buffer        */
#endif
/*
 * Make token sequence from a string literal for _Pragma() operator.
 */
{
    char *  in_p;
    int     c1;
    register int   c;

    in_p = in;
    if (*in_p == 'L')
        in_p++;                             /* Skip 'L' prefix      */
    while ((c = *++in_p) != EOS) {
        if (c == '\\' && ((c1 = *(in_p + 1), c1 == '\\') || c1 == '"'))
            c = *++in_p;            /* "De-escape" escape sequence  */
        *out++ = c;
    }
    *--out = EOS;                   /* Remove the closing '"'       */
    return  out;
}

#endif  /* MODE == STANDARD && OK_PRAGMA_OP */

#if NWORK < NMACWORK
static void
#if PROTO
devide_line( void)
#else
devide_line()
#endif
/*
 * Devide a too long line into output lines shorter than NWORK.
 * This routine is called prior to post_preproc() to save size of catbuf[]
 * for 8-bits systems.
 */
{
    FILEINFO *  file;
    char *  save;
    char *  wp;
    int     token_type;
    register int    c;

    file = unget_string( output, NULLST);   /* To re-read the line  */
    wp = out_ptr = output;
    if (mode != POST_STD) {
        skip_ws();
        unget();
    }

    while ((c = get()), file == infile) {
        if (c == ' ') {
            *out_ptr++ = ' ';
            wp++;
            continue;
        }
        token_type = scan_token( c, &wp, out_wend); /* Read a token */
        if (NWORK-1 <= wp - out_ptr) {          /* Too long a token */
            cfatal( "Too long token %s", out_ptr, 0L, NULLST);      /* _F_  */
        } else if (out_end <= wp || token_type == STR
#if MODE == STANDARD
                    || token_type == WSTR
#endif
                ) {
            /* Too long line or string literal (maybe concatenated later)   */
            save = save_string( out_ptr);       /* Save the token   */
            while (*(out_ptr - 1) == ' ')
                out_ptr--;              /* Remove trailing spaces   */
            putout( output);            /* Putout the former tokens */
#if CONCAT_STRINGS
            if (catflag != SUSPEND)
                sharp();
            else
                wrong_line = TRUE;
#else
            sharp();                        /* Correct line number  */
#endif
            wp = out_ptr = stpcpy( output, save);   /* Restore the token    */
            free( save);
        } else {                            /* Still in size        */
            out_ptr = wp;                   /* Advance the pointer  */
        }
    }

    unget();
}
#endif

static void
#if PROTO
putout( char * out)
#else
putout( out)
    char *  out;    /* Output line (line-end is always 'out_ptr')   */
#endif
/*
 * Put out a line with or without "post-preprocessing".
 */
{
    *out_ptr++ = '\n';                      /* Put out a newline    */
    *out_ptr = EOS;

#if CONCAT_STRINGS
    do {                                    /* String concatenation */
        catflag = post_preproc( out);
        switch (catflag) {
        case DONE   :               /* The line has been done       */
        case FLUSH  :               /* The line has been suspended  */
            put_a_line( catbuf);            /* Put out the line     */
            catbufp = catbuf;               /* This line has done   */
            break;
        case SUSPEND:
            catbufp = NULL;                 /* Suspend the line     */
            break;
        }
    } while (catflag == FLUSH);     /* Process the current line     */
#else
#if (BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE) || ! HAVE_C_BACKSLASH_A
    post_preproc( out);
#else
#if MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS
    if (mode == STD && digraphs)
        post_preproc( out);
#endif
#endif
    put_a_line( out);                       /* No post-preprocess   */
#endif
}

static void
#if PROTO
put_a_line( const char * out)
#else
put_a_line( out)
    char *  out;
#endif
/*
 * Finally put out the preprocessed line.
 */
{
    if (no_output)
        return;
    if (NWORK <= strlen( out))
        cfatal( "Too long output line %s", out, 0L, NULLST);        /* _F_  */
    if (fputs( out, fp_out) == EOF)
        cfatal( "File write error", NULLST, 0L, NULLST);    /* _F_  */
}


/*
 *      Routines to  P O S T - P R E P R O C E S S
 *
 * 1998/08      created     kmatsui     (revised 1998/09, 2004/02)
 *    Supplementary phase for the older compiler-propers.
 *      1. Convert digraphs to usual tokens.
 *      2. Convert '\a' and '\v' to octal escape sequences.
 *      3. Concatenate string literals.
 *      4. Double '\\' of the second byte of multi-byte characters.
 *    These conversions are done selectively according to the macros defined
 *  in system.H.
 *      1. Digraphs are converted if MODE == STANDARD && ! HAVE_DIGRAPHS
 *  and digraph recoginition is enabled by DIGRAPHS_INIT and/or -2 option on
 *  execution.
 *      2. Convert '\a', '\v' in string literal or character constant to the
 *  values defined by ALERT_STR, VT_STR (provided HAVE_C_BACKSLASH_A == FALSE).
 *      3. String concatenation is done if CONCAT_STRINGS == TRUE, through the
 *  following phases in this order.
 *      3.1. When a character string literal is succeded by another character
 *  string literal or a wide string literal is succeded by another wide string
 *  literal, and the former literal ends with a octal or hexadecimal escape
 *  sequence and the latter literal starts with a hexa-decimal-digit
 *  character, convert the latter character to 3-digits octal escape sequence.
 *      3.2. Concatenate adjacent character string literals, concatenate
 *  adjacent wide string literals.  (Adjacency of character string literal
 *  and wide string literal is an error.)
 *      4. '\\' of the second byte of SJIS (BIGFIVE or ISO2022_JP) is doubled
 *  if bsl_need_escape == TRUE.
 */

/*
 * 1998/09      kmatsui
 *      Updated post_preproc() to concatenate wide-character-string and
 *          multibyte-character-string generating a wide-character-string.
 */

/*
 * 2004/02      kmatsui
 *      Changed some macros and updated esc_mbchar() according to the
 *          extension of multi-byte character handling.
 */

#if ! CONCAT_STRINGS

#if (MODE == PRE_STANDARD || !OK_DIGRAPHS || HAVE_DIGRAPHS) \
        && ! (BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE) && HAVE_C_BACKSLASH_A
    /* No post_preproc()    */
#else

static int
#if PROTO
post_preproc( char * out)
#else
post_preproc( out)
    char *  out;
#endif
/*
 * Convert digraphs and double '\\' of the second byte of SJIS (BIGFIVE or
 * ISO2022_JP).
 */
{
#if MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS
    int     di_count = 0;
#endif
    int     token_type;
    register int    c;
    char *  str;
    char *  cp = out;

    unget_string( out, NULLST);
    while ((c = get()) != '\n') {   /* Not to read over to next line    */
        if (c == ' ') {
            *cp++ = ' ';
            continue;
        }
        str = cp;
        token_type = scan_token( c, &cp, out_end);
        switch (token_type) {
#if (BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE) || ! HAVE_C_BACKSLASH_A
#if MODE == STANDARD
        case WSTR   :
        case WCHR   :
            str++;                          /* Skip prefix 'L'      */
            /* Fall through */
#endif
        case STR    :
        case CHR    :
#if ! HAVE_C_BACKSLASH_A
            cp = conv_esc( str);
#endif
            if (bsl_need_escape)
                cp = esc_mbchar( str, cp);
            break;
#endif  /* (BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE)
            || ! HAVE_C_BACKSLASH_A */
#if MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS
        case OPE    :
            if (mode == STD && (openum & OP_DIGRAPH)) {
                cp = conv_a_digraph( cp);   /* Convert a digraph    */
                di_count++;
            }
            break;
#endif
        }
    }
    *cp++ = '\n';
    *cp = EOS;
#if MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS
    if (mode == STD && di_count && (warn_level & 16))
        cwarn( "%.0s%ld digraph(s) converted"           /* _W16_    */
                , NULLST, (long) di_count, NULLST);
#endif
    return  0;
}

#endif  /* (MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS)
    || (BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE) || ! HAVE_C_BACKSLASH_A   */

#else   /* CONCAT_STRINGS   */

static int
#if PROTO
post_preproc( char * out)
#else
post_preproc( out)
    char *  out;
#endif
/*
 * Concatenate adjacent character string literals, concatenate adjacent wide
 * string literals.
 */
{
    static     char *   catenated
                = "String literals %s are concatenated";    /* _W8_ */
    static     char *   catp;           /* Current output pointer   */
    static     char *   prev_token;     /* Previous token pointer   */
    static     int      prev_type = 0;  /* Type of previous token   */
#if MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS
    int     di_count = 0;               /* Count of digraphs        */
#endif
    int     c;                          /* Value of the character   */
    int     token_type;                 /* Type of current token    */
    char *  token;                      /* Top of current token     */
    register char *     cp;

    if (catbufp)                        /* Output newer line        */
        catp = catbufp;
    if (catflag != FLUSH) {
#if DEBUG
        if (debug & TOKEN)
            dump_string( "catstr enter", out);
#endif
        unget_string( out, NULLST);     /* To re-read               */
    }

    while ((c = get()) != '\n') {
        if (c == ' ') {
            *catp++ = ' ';
            continue;
        }
        token = cp = catp;
        token_type = scan_token( c, &catp, catbuf_end);

        switch (token_type) {
#if MODE == STANDARD
        case WCHR   :   cp++;           /* Wide-character constant  */
            /* Fall through */
#endif
        case CHR    :                   /* Character constant       */
#if ! HAVE_C_BACKSLASH_A
            catp = conv_esc( cp);
#endif
#if BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE
            if (bsl_need_escape)
                catp = esc_mbchar( cp, catp);
#endif
            break;
#if MODE == STANDARD
        case WSTR   :   cp++;           /* Wide string literal      */
            /* Fall through */
#endif
        case STR    :                   /* Character string literal */
            if (prev_type == token_type) {      /* Diagnose prior   */
                if (warn_level & 8)             /*   to conversion. */
                    cwarn( catenated, prev_token, 0L, NULLST);
            }
#if MODE == STANDARD
            else if (prev_type == WSTR) {
                if (warn_level & 8)
                    cwarn( catenated, prev_token, 0L, NULLST);
                token_type = WSTR;
            }
#endif
#if ! HAVE_C_BACKSLASH_A
            catp = conv_esc( cp);
#endif
            if (prev_type == token_type) {      /* Adjacent strings */
                char    *catp1, *catp2;

                catp1 = token - 1;      /* Just before the token    */
                catp2 = cp + 1;         /* To the content of token  */
                while (type[ *catp1 & UCHARMAX] & SPA)
                    --catp1;            /* Back to closing quote    */
                if (is_last_esc( prev_token)) {
                    catp = conv2oct( catp2, catp1);
                } else {
                    memmove( catp1, catp2, (size_t)(catp - cp));
                    catp -= (size_t)(catp2 - catp1);    /* End of string    */
                }
#if MODE == STANDARD
                if (token_type == WSTR && *prev_token != 'L') {
                    memmove( prev_token + 1, prev_token
                            , (size_t)(catp - prev_token));
                    *prev_token = 'L';
                    catp++;
                }
#endif
                token = prev_token;     /* Preserve prev_token      */
            }
            break;
#if MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS
        case OPE    :
            if (mode == STD && (openum & OP_DIGRAPH)) { /* Digraph  */
                catp = conv_a_digraph( catp);
                di_count++;
            }
            break;
#endif
        default :
            break;
        }

#if BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE
        if ((prev_type == STR
#if MODE == STANDARD
                    || prev_type == WSTR
#endif
                ) && prev_type != token_type && bsl_need_escape)
            catp = esc_mbchar( prev_token, catp);
#endif
        if (catflag == SUSPEND      /* The line has been suspended  */
                && token_type != prev_type) {   /* Next non-string  */
            catflag = FLUSH;        /* To output the pending line   */
            unget_string( token, NULLST);       /* To re-read later */
            prev_type = 0;          /* To re-read the same token    */
            catp = token;
            while (type[ *(catp - 1) & UCHARMAX] & SPA)
                catp--;
            goto  ret;              /* Output the pending line      */
        }
        prev_type = token_type;     /* Remember the token           */
        prev_token = token;
    }

    if (token_type == STR
#if MODE == STANDARD
            || token_type == WSTR
#endif
            )
        catflag = SUSPEND;          /* Suspend the line             */
    else
        catflag = DONE;             /* The line has been processed  */
ret:
    *catp++ = '\n';
    *catp = EOS;
#if MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS
    if (mode == STD && di_count && (warn_level & 16))
        cwarn( "%.0s%ld digraph(s) converted"           /* _W16_    */
                , NULLST, (long) di_count, NULLST);
#endif
#if DEBUG
    if (catflag != FLUSH && (debug & TOKEN))
        dump_string( "catstr exit", NULLST);
#endif
    return  catflag;
}

#if HOST_COMPILER == BORLANDC
#define MAXOCT  8   /* To work around a bug of tcc, bcc */
#else
#define MAXOCT  3
#endif

static int
#if PROTO
is_last_esc( char * cp)
#else
is_last_esc( cp)
    char *  cp;
#endif
/*
 * Is the last character of the string a hex-or-octal escape sequence ?
 */
{
    int     i;
    register int    c;

#if MODE == STANDARD
    if (*cp++ == 'L')
#endif
        cp++;                           /* The content of string    */
    while ((c = *cp++ & UCHARMAX) != '"') {
#if BSL_IN_MBCHAR
        if (bsl_in_mbchar) {
            if (type[ c] & mbstart) {   /* Not an escape sequence   */
                mb_read( c, &cp, (workp = work, &workp));
                continue;
            }
        }
#endif
        if (c != '\\')
            continue;

        switch (c = *cp++) {            /* An escape sequence       */
        case 'x' :                  /* Hexadecimal-escape-sequence  */
            while (isxdigit( *cp & UCHARMAX))
                cp++;
            goto  is_last;
        case '0': case '1': case '2': case '3':     /* Octal-escape */
        case '4': case '5': case '6': case '7':     /*  -sequence   */
            for (i = 1; i < MAXOCT && isdigit( *cp & UCHARMAX) && *cp < '8';
                    i++)    /* MAXOCT is bug-to-bug implementation  */
                cp++;
is_last:    if (*cp == '"')             /* String ends with hex or  */
                return  TRUE;           /*   octal escape sequence. */
            break;
        default  :
#if BSL_IN_MBCHAR
            if (bsl_in_mbchar && (type[ c & UCHARMAX] & mbstart))
                cp--;
#endif
            break;
        }
    }
    return  FALSE;
}

static char *
#if PROTO
conv2oct( char * cp, char * out)
#else
conv2oct( cp, out)
    char *  cp;
    char *  out;
#endif
/*
 * If the former string ends with hex or oct escape sequence and the current
 * string starts with a hex char, convert the char to octal sequence.
 */
{
    char *  save;
    char *  in;

    in = save = save_string( cp);       /* Content of the quote     */
    if (isxdigit( *in & UCHARMAX)) {
        sprintf( out, "\\%03o", *in++);
        out += 4;
#if MAXOCT > 3
        while (isdigit( *in & UCHARMAX) && *in != 8 && *in != 9) {
            sprintf( out, "\\%03o", *in++);
            out += 4;
        }
#endif
    }
    out = stpcpy( out, in);
    free( save);
    return  out;
}

#endif      /* CONCAT_STRINGS   */

#if ! HAVE_C_BACKSLASH_A

#ifdef  ALERT_STR
static const char * const   alert = ALERT_STR;
#endif
#ifdef  VT_STR
static const char * const   vt = VT_STR;
#endif

static char *
#if PROTO
conv_esc( char * cp)
#else
conv_esc( cp)
    char *  cp;
#endif
/*
 * Convert \a, \v to 3 digits octal escape sequence.
 */
{
    char *  in;
    char *  save;
    register int    c;

    in = save = save_string( ++cp);     /* Content of the quote     */

    while ((c = *in++ & UCHARMAX) != EOS) {
        if (bsl_in_mbchar && (type[ c] & mbstart)) {
            mb_read( c, &in, (*cp++ = c, &cp));
                /* Multi-byte character is not an escape sequence   */
            continue;
        }
        *cp++ = c;
        if (c != '\\')
            continue;

        switch (c = *in++) {            /* An escape sequence       */
#ifdef  ALERT_STR
        case 'a' :
            cp = stpcpy( cp, alert + 1);
            break;
#endif
#ifdef  VT_STR
        case 'v' :
            cp = stpcpy( cp, vt + 1);
            break;
#endif
        default  :
            if (bsl_in_mbchar && (type[ c & UCHARMAX] & mbstart))
                in--;
            else
                *cp++ = c;
            break;
        }

    }

    *cp = EOS;
    free( save);
    return  cp;
}
#endif  /* ! HAVE_C_BACKSLASH_A */

#if MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS
static char *
#if PROTO
conv_a_digraph( char * cp)
#else
conv_a_digraph( cp)
    char *  cp;                     /* The end of the digraph token */
#endif
/*
 * Convert a digraph to usual token in place.
 * This routine is never called in POST_STD mode.
 */
{
    cp -= 2;
    switch (openum) {
    case OP_LBRACE_D    :
        *cp++ = '{';
        break;
    case OP_RBRACE_D    :
        *cp++ = '}';
        break;
    case OP_LBRCK_D     :
        *cp++ = '[';
        break;
    case OP_RBRCK_D     :
        *cp++ = ']';
        break;
    case OP_SHARP_D     :                       /* Error of source  */
        *cp++ = '#';
        break;
    case OP_DSHARP_D    :                       /* Error of source  */
        cp -= 2;
        *cp++ = '#';
        *cp++ = '#';
        break;
    }
    return  cp;
}
#endif  /* MODE == STANDARD && OK_DIGRAPHS && ! HAVE_DIGRAPHS   */

#if BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE
static char *
#if PROTO
esc_mbchar( char * str,  char * str_end)
#else
esc_mbchar( str, str_end)
    char *  str;        /* String literal or character constant without 'L' */
    char *  str_end;    /* The end of the token */
#endif
/*
 * Insert \ before the byte of 0x5c('\\') of the SJIS, BIGFIVE or ISO2022_JP
 * multi-byte character code in string literal or character constant.
 * Insert \ also before the byte of 0x22('"') and 0x27('\'') of ISO2022_JP.
 * esc_mbchar() does in-place insertion.
 */
{
    char *  cp;
    int     delim;
    register int    c;

    if (! bsl_need_escape)
        return  str_end;
#if MODE == STANDARD
    if ((delim = *str++) == 'L')
#endif
        delim = *str++;                         /* The quote character  */
    while ((c = *str++ & UCHARMAX) != delim) {
        if (type[ c] & mbstart) {               /* MBCHAR   */
            cp = str;
            mb_read( c, &str, (workp = work, &workp));
            while (cp++ < str) {
                c = *(cp - 1);
                if (c == '\\' || c == '"' || c == '\'') {
                                    /* Insert \ before 0x5c, 0x22, 0x27 */
                    memmove( cp, cp - 1, (size_t) (str_end - cp) + 2);
                    *(cp++ - 1) = '\\';
                    str++;
                    str_end++;
                }
            }
        } else if (c == '\\' && ! (type[ *str & UCHARMAX] & mbstart)) {
            str++;                              /* Escape sequence      */
        }
    }
    return  str_end;
}
#endif  /* BSL_IN_MBCHAR && ! MBCHAR_IS_ESCAPE_FREE */

