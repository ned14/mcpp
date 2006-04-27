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
 *                          I N T E R N A L . H
 *      I n t e r n a l   D e f i n i t i o n s   f o r   M C P P
 *
 * In general, definitions in this file should not be changed by implementor.
 *
 * 1984/05 - 1985/06    DECUS CPP / cpp.h
 *              Created and released on USENET.         by Martin Minow
 *
 * 1998/08      CPP Version 2.0 / internal.H
 *              Re-written and renamed from cpp.h.      by kmatsui
 * 1998/09      CPP Version 2.1 / internal.H
 *              Updated according to C99-1998/08 draft. by kmatsui
 * 1998/11      CPP Version 2.2 / internal.H
 *              Slightly modified.                      by kmatsui
 * 2002/08      CPP Version 2.3 pre-release 1 / internal.H
 *              Slightly modified.                      by kmatsui
 * 2002/12      CPP Version 2.3 pre-release 2 / internal.H
 *              Slightly modified.                      by kmatsui
 * 2003/02      CPP Version 2.3 release / internal.H
 *              Slightly modified.                      by kmatsui
 * 2003/11      MCPP Version 2.4 pre-release / internal.H
 *              Changed DEFBUF and FILEINFO.  Reorganized some functions and
 *              variables.                              by kmatsui
 * 2004/02      MCPP Version 2.4 release / internal.H
 *              Extended multi-byte character handling. by kmatsui
 * 2004/03      MCPP Version 2.4.1 / internal.H
 *              Added -c option                         by kmatsui
 * 2005/03      MCPP Version 2.5 / internal.H
 *              Absorbed POST_STANDARD into STANDARD and OLD_PREPROCESSOR
 *                  into PRE_STANDARD.                  by kmatsui
 */

#ifndef SYSTEM_H
    #error "system.H" must be included prior to "internal.H"
#endif

#define EOS             '\0'        /* End of string                */
#define CHAR_EOF        0           /* Returned by get() on eof     */
#define NULLST          ((char *) NULL)     /* Pointer to nowhere   */

#define VA_ARGS         (UCHARMAX + 1)      /* Signal of variable arguments */
#define DEF_PRAGMA      (-1 - VA_ARGS)      /* _Pragma() pseudo-macro       */
#define DEF_NOARGS      (-2 - VA_ARGS)      /* #define foo vs #define foo() */

/*
 * The following may need to change if the host system doesn't use ASCII.
 * These magic characters must be control characters which can't be used
 * in source file.
 */
#if     MODE == STANDARD
#define DEF_MAGIC       0x19        /* Magic for #defines           */
#define IN_SRC          0x1A        /* Magic of name from source    */
#define RT_END          0x1C        /* Magic of macro rescan boundary   */
#define ST_QUOTE        0x1D        /* Magic for stringizing        */
#define CAT             0x1E        /* Token concatenation delim.   */
#define TOK_SEP         0x1F        /* Magic to wrap expanded macro */
#else
#define COM_SEP         0x1F        /* Comment as token separator   */
#endif

#define MAC_PARM        0x7F        /* Macro formals signal         */

/* Special character types  */
#define LET             1           /* Letter (alphabet and _)      */
#define DIG             2           /* Digit                        */
#define DOT             4           /* . might start a number       */
#define PUNC            8           /* Punctuators and operators    */
#define QUO             16          /* Both flavors of quotation ",'*/
#define SPA             32          /* White spaces                 */

/*
 * Codes for operators used in #if expression.
 * The value is stored in 'openum'.
 */
#define INV             0           /* Invalid, must be zero        */
#define OP_EOE          INV         /* End of expression            */
#define VAL             1           /* Value (operand)              */
#define OP_LPA          2           /* (    */
/* The following are unary.     */
#define FIRST_UNOP      OP_PLU      /* First unary operator         */
#define OP_PLU          3           /* + (ANSI Standard)            */
#define OP_NEG          4           /* -    */
#define OP_COM          5           /* ~    */
#define OP_NOT          6           /* !    */
#define LAST_UNOP       OP_NOT      /* Last unary operator          */
/* The following are binary.    */
#define FIRST_BINOP     OP_MUL      /* First binary operator        */
#define OP_MUL          7           /* *    */
#define OP_DIV          8           /* /    */
#define OP_MOD          9           /* %    */
#define OP_ADD          10          /* +    */
#define OP_SUB          11          /* -    */
#define OP_SL           12          /* <<   */
#define OP_SR           13          /* >>   */
#define OP_LT           14          /* <    */
#define OP_LE           15          /* <=   */
#define OP_GT           16          /* >    */
#define OP_GE           17          /* >=   */
#define OP_EQ           18          /* ==   */
#define OP_NE           19          /* !=   */
#define OP_AND          20          /* &    */
#define OP_XOR          21          /* ^    */
#define OP_OR           22          /* |    */
#define OP_ANA          23          /* &&   */
#define OP_ORO          24          /* ||   */
#define OP_QUE          25          /* ?    */
#define OP_COL          26          /* :    */
#define LAST_BINOP      OP_COL      /* Last binary operator         */
/* Parenthesis  */
#define OP_RPA          27          /* )    */
#define OP_END          28          /* End of expression marker     */
#define OP_FAIL         (OP_END + 1)    /* For error returns        */

/*
 * The following are operators used in macro definition only.
 */
#if     MODE == STANDARD
#define OP_STR          30          /* #    */
#define OP_CAT          31          /* ##   */
#define OP_ELL          32          /* ...  */
#endif

/*
 * The following are C source operators or punctuators,
 * not preprocessing operators.
 * Note: "sizeof", "defined" are read as identifier for convenience.
 */
#define OP_1            33  /* Any other single byte ops or puncs   */
                                    /* =, ., ;, [, ], {, }, ','     */
#define OP_2            34  /* Any other two bytes operators        */
                    /* &=, |=, ++, +=, --, -=, ->, %=, *=, /=, ^=,  */
#define OP_3            35  /* Three bytes operators :  <<=, >>=    */
/*
 * The following are operators spelled in digraphs.
 */
#if     MODE == STANDARD
#define OP_LBRACE_D     0x40        /* <% i.e. {    */
#define OP_RBRACE_D     0x41        /* %> i.e. }    */
#define OP_LBRCK_D      0x42        /* <: i.e. [    */
#define OP_RBRCK_D      0x43        /* :> i.e. ]    */
#define OP_SHARP_D      0x44        /* %: i.e. #    */
#define OP_DSHARP_D     0x45        /* %:%: i.e. ## */
#define OP_DIGRAPH      0x40    /* (OP_*_D & OP_DIGRAPH) == 0x40    */
#endif

/*
 * The following are for lexical scanning only.
 */
/* Token types          */
#define NO_TOKEN        0
#define NAM             65          /* Identifier (name, keyword)   */
#define NUM             66          /* Preprocessing number         */
#define STR             67          /* Character string literal     */
#define CHR             69          /* Integer character constant   */
#if     MODE == STANDARD
#define WSTR            68          /* Wide string literal          */
#define WCHR            70          /* Wide character constant      */
#endif
#define OPE             71          /* Operator or punctuator       */
#define SPE             72          /* Unknown token (@ or others)  */
#define SEP             73          /* Token separator or magics    */

/*
 * The following are values of 'debug' variable which is set by the
 * arguments of #pragma MCPP debug directive.
 */
#define PATH            1
#define TOKEN           2
#define EXPAND          4
#define IF              8
#define EXPRESSION      16
#define GETC            32
#define MEMORY          64

/*
 * USE_MBCHAR means to implement multi-byte character handling routines.
 * BSL_IN_MBCHAR means multi-byte character may coincide with '\\'.
 * MB_ERROR signals wrong multi-byte character sequence.
 */
#define USE_MBCHAR      (SYSTEM != SYS_MSDOS | MBCHAR)
#define BSL_IN_MBCHAR   \
        (SYSTEM != SYS_MSDOS | (MBCHAR & (SJIS | BIGFIVE | ISO2022_JP)))
#define MB_ERROR        0x8000

/*
 * The DEFBUF structure stores information about #defined macros.
 * Note that DEFBUF->parmnames is parameter names catenated with commas,
 * which is saved for the check of redefinition for Standard mode.
 * 'parmnames' and 'repl' are allocated to the area succeding to name.
 */
typedef struct defbuf {
        struct defbuf * link;       /* Pointer to next def in chain */
        short           nargs;      /* Number of parameters         */
#if     MODE == STANDARD
        char *          parmnames;  /* -> Parameter names catenated by ','  */
#endif
        char *          repl;       /* Pointer to replacement text  */
#if     DEBUG
        const char *    dir;        /* The macro is defined in      */
        const char *    fname;      /*      dir/fname source file   */
        long            mline;      /*          at the line.        */
#endif
#if     MODE == STANDARD
        char            push;       /* Push level indicator         */
#endif
        char            name[1];    /* Macro name                   */
} DEFBUF;

/*
 * The FILEINFO structure stores information about open files and macros
 * being expanded.
 */
typedef struct fileinfo {
        char *          bptr;       /* Current pointer into buffer  */
        long            line;       /* Current line number of file  */
        FILE *          fp;         /* Source file if non-null      */
        long            pos;        /* Position next to #include    */
        struct fileinfo *   parent; /* Link to includer             */
        struct ifinfo *     initif; /* Initial ifstack (return there on EOF)*/
        const char **   dirp;       /* Include directory the file resides   */
        const char *    filename;   /* File/macro name              */
        char *          buffer;     /* Buffer of current input line */
} FILEINFO;

/*
 * IFINFO stores information of conditional compilation.
 */
typedef struct ifinfo {
        int             stat;       /* State of compilation         */
        long            ifline;     /* Line #if started             */
        long            elseline;   /* Line #else started           */
} IFINFO;

/*
 * These bits are set in IFINFO.stat
 */
#define WAS_COMPILING   1           /* TRUE if compile set at entry */
#define ELSE_SEEN       2           /* TRUE when #else processed    */
#define TRUE_SEEN       4           /* TRUE when #if TRUE processed */

#define compiling       ifstack[0].stat

/* VAL_SIGN structure stores information about evaluated number.    */
typedef struct val_sign {
        expr_t          val;        /* Value                        */
        int             sign;       /* Signed, unsigned, error      */
} VAL_SIGN;

/* Values of VAL_SIGN.sign. */
#define SIGNED          1
#define UNSIGNED        0
#define VAL_ERROR       (-1)

/* Value of macro_line on macro call error.         */
#define MACRO_ERROR     (-1L)

/* Values of mkdep. */
#define MD_MKDEP        1   /* Output source file dependency line   */
#define MD_SYSHEADER    2   /* Print also system-header names       */
#define MD_FILE         4   /* Output to the file named *.d         */
#define MD_PHONY        8   /* Print also phony targets for each header */
#define MD_QUOTE        16  /* 'Quote' $ and space in target name   */

#if     MODE == STANDARD
/*
 * Values of insert_sep (flag of insertion of token separator).
 * Used only in POST_STD mode.
 */
#define NO_SEP          0   /* No separator is inserted             */
#define INSERT_SEP      1   /* Next get() shall insert a separator  */
#define INSERTED_SEP    2   /* Last get() has Inserted a separator  */
#endif

#if     OK_SIZE
/*
 * Define bits for the basic types and their adjectives.
 */
#define T_CHAR          1
#define T_INT           2
#define T_FLOAT         4
#define T_DOUBLE        8
#define T_LONGDOUBLE    16
#define T_SHORT         32
#define T_LONG          64
#define T_LONGLONG      128
#define T_SIGNED        256
#define T_UNSIGNED      512
#define T_PTR           1024        /* Pointer to data objects      */
#define T_FPTR          2048        /* Pointer to functions         */
/*
 * The SIZES structure is used to store the values for #if sizeof.
 */
typedef struct sizes {
        int             bits;       /* If this bit is set,          */
        int             size;       /* this is the datum size value */
        int             psize;      /* this is the pointer size     */
} SIZES;
#endif

#define str_eq(s1, s2)  (strcmp(s1, s2) == 0)

#ifndef IO_SUCCESS
#define IO_SUCCESS      0
#endif
#ifndef IO_ERROR
#define IO_ERROR        (errno ? errno : 1)
#endif

/* For conv_case()  */
#define UPPER           1           /* To upper */
#define LOWER           0           /* To lower */

/*
 * Externs
 */
extern int      mode;               /* Mode of preprocessing        */
extern int      cflag;              /* -C option (keep comments)    */
extern int      zflag;  /* -Z option (no-output of included file)   */
extern int      pflag;              /* -P option (no #line output)  */
extern int      qflag;              /* -Q option (diag to mcpp.err) */
#if     MODE == STANDARD
#if     OK_TRIGRAPHS
extern int      tflag;              /* -3 option (toggle trigraphs) */
#endif
#if     OK_DIGRAPHS
extern int      digraphs;           /* -2 option (toggle digraphs)  */
#endif
extern int      stdc_val;           /* Value of __STDC__            */
extern long     stdc_ver;           /* Value of __STDC_VERSION__    */
extern long     cplus;              /* Value of __cplusplus for C++ */
extern int      stdc2;              /* cplus || stdc_ver >= 199901L */
extern int      stdc3;      /* (stdc_ver or cplus) >= 199901L       */
extern long     str_len_min;        /* Least maxmum of string len.  */
extern size_t   id_len_min;         /* Least maximum of ident len.  */
extern int      n_mac_pars_min;     /* Least maximum of num of pars.*/
extern int      exp_nest_min;       /* Least maximum of expr nest   */
extern int      blk_nest_min;       /* Least maximum of block nest  */
extern int      inc_nest_min;       /* Least maximum of include nest*/
extern long     n_macro_min;        /* Least maximum of num of macro*/
extern long     line_limit;         /* Maximum source line number   */
extern int      has_pragma;         /* '_Pragma()' has been expanded*/
#endif  /* MODE == STANDARD */
extern int      lang_asm;           /* -a option (assembler source) */
extern int      std_line_prefix;    /* #line in C source style      */
extern int      warn_level;         /* Level of warning             */
extern int      errors;             /* Error counter                */
extern long     line;               /* Current source line number   */
extern int      wrong_line;         /* Force #line to compiler      */
extern int      keep_comments;      /* Don't remove comments        */
extern int      include_nest;       /* Nesting level of #include    */
extern const char *     null;       /* "" string for convenience    */
extern const char **    inc_dirp;   /* Directory of #includer       */
extern const char *     cur_fname;  /* Current source file name     */
extern int      no_output;          /* Don't output included file   */
extern int      in_directive;       /* In process of #directive     */
extern int      in_define;          /* In #define line              */
#if     MODE == STANDARD
extern int      in_getarg;          /* Collecting arguments of macro*/
extern int      compat_mode;        /* Compatible macro recursion   */
#endif
extern long     macro_line;         /* Line number of macro call    */
extern int      openum;             /* Number of operator or punct. */
extern IFINFO *     ifptr;          /* -> current ifstack item      */
extern FILEINFO *   infile;         /* Current input file or macro  */
extern int      no_source_line;     /* Do not output line in diag.  */
extern FILE *   fp_in;              /* Input stream to preprocess   */
extern FILE *   fp_out;             /* Output stream preprocessed   */
extern FILE *   fp_err;             /* Diagnostics stream           */
extern FILE *   fp_debug;           /* Debugging information stream */
#if     MODE == STANDARD
extern int      insert_sep;         /* Inserted token separator flag*/
#endif
#if     OK_MAKE
extern int      mkdep;              /* Output source file dependency*/
#endif
extern int      mbchar;             /* Encoding of multi-byte char  */
extern int      mbmask;             /* Char type other than mbchar  */
extern int      mbstart;            /* 1st byte of mbchar (or shift)*/
#if     BSL_IN_MBCHAR
extern int      bsl_in_mbchar;      /* 2nd byte of mbchar has '\\'  */
extern int      bsl_need_escape;/* '\\' in mbchar should be escaped */
#endif
extern char *   workp;              /* Free space in work[]         */
extern char * const     work_end;   /* End of work[] buffer         */
extern char     identifier[];       /* Lastly scanned name          */
#if     SYSTEM == SYS_MSDOS
extern char     type[];             /* Character classifier         */
#else
extern short    *type;              /* Character classifier         */
#endif
extern IFINFO   ifstack[];          /* Information of #if nesting   */
extern char     cur_fullname[];     /* Full name of current source  */
extern char     work[];
        /* Temporary buffer for directive line and macro expansion  */

#if     OK_SIZE
extern SIZES    size_table[];       /* For #if sizeof sizes         */
#endif
#if     DEBUG || DEBUG_EVAL
extern int      debug;              /* Debug class                  */
#endif
#if     MODE == PRE_STANDARD
extern long     in_asm;             /* In #asm - #endasm block      */
#endif

#if     PROTO   /* prototype declarations   */

/* main.c   */
extern void     sharp( void);
extern void     un_predefine( int clearall);
extern void     undef_a_predef( const char * name);
/* control.c    */
extern int      control( int newlines);
extern DEFBUF * do_define( int ignore_redef);
extern DEFBUF * look_id( const char * name);
extern DEFBUF **    look_prev( const char * name, int * cmp);
extern DEFBUF * look_and_install( const char * name, int numargs
        , const char * parmnames, const char * repl);
extern DEFBUF * install( const char * name, int numargs, const char * parmnames
        , const char * repl, DEFBUF ** prevp, int cmp);
extern int      undefine( const char * name);
extern void     dump_a_def( const char * why, const DEFBUF * dp, int newdef
        , int dDflag, int comment, FILE * fp);
extern void     dump_def( int dDflag, int comment);
/* eval.c   */
extern expr_t   eval( void);
extern VAL_SIGN *   eval_num( const char * nump);
/* expand.c */
extern DEFBUF * is_macro( char ** cp);
extern char *   expand( DEFBUF * defp, char * out, char * out_end);
/* support.c    */
extern int      get_unexpandable( int c, int diag);
extern void     skip_nl( void);
extern int      skip_ws( void);
extern int      scan_token( int c, char ** out_pp, char * out_end);
extern char *   scan_quote( int delim, char * out, char * out_end, int diag);
#if     MODE == STANDARD
extern int      id_operator( const char * name);
#endif
extern int      get( void);
#if     MODE == STANDARD && OK_TRIGRAPHS
extern int      cnv_trigraph( char * in);
#endif
#if     MODE == STANDARD && OK_DIGRAPHS
extern int      cnv_digraph( char * in);
#endif
extern void     unget( void);
extern FILEINFO *   unget_string( const char * text, const char * name);
extern char *   save_string( const char * text);
extern FILEINFO *   get_file( const char * name, size_t bufsize);
extern char *   (xmalloc)( size_t size);
extern char *   (xrealloc)( char * ptr, size_t size);
extern void     cfatal( const char * format, const char * arg1, long arg2
        , const char * arg3);
extern void     cerror( const char * format, const char * arg1, long arg2
        , const char * arg3);
extern void     cwarn( const char * format, const char * arg1, long arg2
        , const char * arg3);
/* system.c */
extern void     do_options( int argc, char ** argv, char ** in_pp
        , char ** out_pp);
extern void     at_start( void);
#if     OK_MAKE
extern void     put_depend( const char * filename);
#endif
extern int      do_include( int next);
extern void     add_file( FILE * fp, const char * filename);
extern void     conv_case( char * name, char * lim, int upper);
#if     MODE == STANDARD
extern void     do_pragma( void);
#else
extern void     put_asm( void);
#endif
extern void     do_old( void);
extern void     at_end( void);
/* mbchar.c     */
#if     SYSTEM != SYS_MSDOS
extern const char *     set_encoding( char * name, char * env, int pragma);
#endif
extern void     mb_init( void);
extern size_t   (*mb_read)( int c1, char ** in_pp, char ** out_pp);
extern uexpr_t  mb_eval( char ** seq_pp);
#if     DEBUG
/* support.c    */
extern void     dump_string( const char * why, const char * text);
extern void     dump_unget( const char * why);
/* system.c     */
extern void     print_heap( void);
#endif

#else           /* old style function declarations  */

/* main.c   */
extern void     sharp();            /* Output # line number         */
extern void     un_predefine();     /* Undefine predefined macros   */
extern void     undef_a_predef();   /* Remove a non-standard predef */
/* control.c    */
extern int      control();          /* Process #directive lines     */
extern DEFBUF * do_define();        /* Do #define directive         */
extern DEFBUF * look_id();          /* Look for a #define'd thing   */
extern DEFBUF **    look_prev();    /* Look for place to insert def.*/
extern DEFBUF * look_and_install(); /* Look and insert macro def.   */
extern DEFBUF * install();          /* Install to symbol table      */
extern int      undefine();         /* Delete from symbol table     */
extern void     dump_a_def();       /* Dump a specific macro def    */
extern void     dump_def();         /* Dump current macro defs      */
/* eval.c   */
extern expr_t   eval();             /* Evaluate #if expression      */
extern VAL_SIGN *   eval_num();     /* Evaluate preprocessing number*/
/* expand.c */
extern DEFBUF * is_macro();         /* The sequence is a macro call?*/
extern char *   expand();           /* Expand a macro completely    */
/* support.c    */
extern int      get_unexpandable(); /* Get next unexpandable token  */
extern void     skip_nl();          /* Skip to the end of the line  */
extern int      skip_ws();          /* Skip over white-spaces       */
extern int      scan_token();       /* Get the next token           */
extern char *   scan_quote();       /* Scan a quoted literal        */
#if     MODE == STANDARD
extern int      id_operator();      /* Check identifier-like ops    */
#endif
extern int      get();              /* Get the next char from input */
#if     MODE == STANDARD && OK_TRIGRAPHS
extern int      cnv_trigraph();     /* Do trigraph replacement      */
#endif
#if     MODE == STANDARD && OK_DIGRAPHS
extern int      cnv_digraph();      /* Convert digraphs to usual tok*/
#endif
extern void     unget();            /* Push back the char to input  */
extern FILEINFO *   unget_string(); /* Push back the string to input*/
extern char *   save_string();      /* Stuff string in malloc mem.  */
extern FILEINFO *   get_file();     /* New FILEINFO initialization  */
extern char *   (xmalloc)();        /* Get memory or die            */
extern char *   (xrealloc)();       /* Reallocate memory or die     */
extern void     cfatal();           /* Print a fatal error and exit */
extern void     cerror();           /* Print an error message       */
extern void     cwarn();            /* Print a warning message      */
/* system.c */
extern void     do_options();       /* Process command line args    */
extern void     at_start();         /* Commands prior to main input */
#if     OK_MAKE
extern void     put_depend();       /* Output source dependency line*/
#endif
extern int      do_include();       /* Process #include directive   */
extern void     add_file();         /* Chain the included file      */
extern void     conv_case();        /* Convert to upper/lower case  */
#if     MODE == STANDARD
extern void     do_pragma();        /* Process #pragma directive    */
#else
extern void     put_asm();          /* Putout an asm code line      */
#endif
extern void     do_old();           /* Process older directives     */
extern void     at_end();           /* Do the final commands        */
/* mbchar.c     */
#if     SYSTEM != SYS_MSDOS
extern char *   set_encoding();     /* Multi-byte char encoding     */
#endif
extern void     mb_init();          /* Initialize mbchar variables  */
extern size_t   (*mb_read)();       /* Read mbchar sequence         */
extern uexpr_t  mb_eval();          /* Evaluate mbchar in #if       */

#if     DEBUG
/* support.c    */
extern void     dump_string();      /* Dump text readably           */
extern void     dump_unget();       /* Dump all ungotten junk       */
/* system.c     */
extern void     print_heap();       /* Print blocks of heap memory  */
#endif

#endif          /* ! PROTO  */

