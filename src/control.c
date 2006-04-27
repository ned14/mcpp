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
 *                          C O N T R O L . C
 *              P r o c e s s   C o n t r o l   L i n e s
 *
 * Edit history of DECUS CPP / cpp2.c
 * 13-Nov-84    MM      Split from cpp1.c
 * 06-Jun-85    KR      Latest revision
 */

/*
 * CPP Version 2.0 / control.c
 * 1998/08      kmatsui
 *      Renamed cpp2.c control.c.
 *      Moved dodefine(), is_formal(), mtokensave(), stparmscan(), doundef()
 *          from cpp4.c to control.c.
 *      Moved lookid(), defendel(), dump_a_def() from cpp6.c to control.c.
 *      Moved doinclude(), openinclude(), vmsparse()
 *          from cpp2.c to system.c.
 *      Split doline() from control().
 *      Split getparm(), getrepl(), def_stringization() from dodefine().
 *      Split dumprepl() from dump_a_def().
 *      Devided defendel() to install(), undefine(), lookprev().
 *      Removed textput(), charput(), checkparm() from cpp4.c.
 *      Expanded specification of stparmscan().
 *      Revised most of the functions.
 */

/*
 * CPP Version 2.2 / control.c
 * 1998/11      kmatsui
 *      Revised several functions.
 */

/*
 * CPP Version 2.3 pre-release 1 / control.c
 * 2002/08      kmatsui
 *      Revised several functions.
 *      Renamed several functions using underscore.
 *
 * CPP Version 2.3 release / control.c
 * 2003/02      kmatsui
 *      Reinforced checking __VA_ARGS__ identifier in #define line.
 */

/*
 * MCPP Version 2.4 prerelease
 * 2003/11      kmatsui
 *      Changed DEFBUF and FILEINFO structure, and implemented dir/filename
 *          /line information of defined macros.
 *      Created look_and_install().
 *      Revised do_define(), look_id, look_prev(), install() and dump_a_def().
 */

/*
 * MCPP Version 2.5
 * 2005/03      kmatsui
 *      Revised STRING_FORMAL and COMMENT_INVISIBLE so as to follow so-called
 *          "Reiser model" cpp.
 *      Absorbed POST_STANDARD into STANDARD and OLD_PREPROCESSOR into
 *          PRE_STANDARD.
 */

/*
 * The routines to handle #directives other than #include and #pragma
 * are placed here.
 */

#if PREPROCESSED
#include    "mcpp.H"
#else
#include    "system.H"
#include    "internal.H"
#endif

#if PROTO

static int      do_if( int hash);
static long     do_line( void);
static int      get_parm( void);
static int      get_repl( const char * macroname);
static char *   is_formal( const char * name, int conv);
#if MODE == STANDARD
static char *   def_stringization( char * repl_cur);
static char *   mgtoken_save( const char * macroname);
#else
static char *   str_parm_scan( char * string_end);
#endif
static void     do_undef( void);
static void     dump_repl( const DEFBUF * dp, FILE * fp);

#else   /* ! PROTO  */

static int      do_if();        /* #if, #elif, #ifdef, #ifndef      */
static long     do_line();      /* Process #line directive          */
static int      get_parm();     /* Get parm., its nargs, names, lens*/
static int      get_repl();     /* Get replacement embedded parm-no.*/
static char *   is_formal();    /* If formal param., save the num.  */
#if MODE == STANDARD
static char *   def_stringization();    /* Define stringization     */
static char *   mgtoken_save(); /* Prefix DEF_MAGIC to macro name   */
#else
static char *   str_parm_scan();    /* Scan the param. in string    */
#endif
static void     do_undef();     /* Process #undef directive         */
static void     dump_repl();    /* Dump replacement text            */

#endif  /* ! PROTO  */

/*
 * Generate (by hand-inspection) a set of unique values for each directive.
 * MCPP won't compile if there are hash conflicts.
 */

#define L_if            ('i' ^ (EOS << 1))
#define L_ifdef         ('i' ^ ('e' << 1))
#define L_ifndef        ('i' ^ ('d' << 1))
#if MODE == STANDARD
#define L_elif          ('e' ^ ('f' << 1))
#endif
#define L_else          ('e' ^ ('e' << 1))
#define L_endif         ('e' ^ ('i' << 1))
#define L_define        ('d' ^ ('i' << 1))
#define L_undef         ('u' ^ ('e' << 1))
#define L_line          ('l' ^ ('e' << 1))
#define L_include       ('i' ^ ('l' << 1))
#if COMPILER == GNUC
#define L_include_next  ('i' ^ ('l' << 1) ^ ('_' << 1))
#endif
#if MODE == STANDARD
#define L_error         ('e' ^ ('o' << 1))
#define L_pragma        ('p' ^ ('g' << 1))
#endif

static const char * const   not_ident
            = "Not an identifier \"%s\"";               /* _E_      */
static const char * const   no_arg = "No argument";     /* _E_      */
#if ! OK_IF_JUNK
static const char * const   excess
            = "Excessive token sequence \"%s\"";        /* _E_ _W1_ */
#endif

int
#if PROTO
control( int newlines)
#else
control( newlines)
    int         newlines;                       /* Pending newlines */
#endif
/*
 * Process #directive lines.  Each directive have their own subroutines.
 */
{
    const char * const  many_nesting =
"More than %.0s%ld nesting of #if (#ifdef) sections%s"; /* _F_ _W4_ _W8_    */
    const char * const  not_in_section
    = "Not in a #if (#ifdef) section in a source file"; /* _E_ _W1_ */
    const char * const  illeg_dir
            = "Illegal #directive \"%s%.0ld%s\"";       /* _E_ _W8_ */
    const char * const  in_skipped = " (in skipped block)"; /* _W8_ */
#if MODE == PRE_STANDARD
    int     token_type;
#endif
    register int    c;
    register int    hash;
    char *  tp;

    in_directive = TRUE;
    if (keep_comments) {
        fputc( '\n', fp_out);       /* Possibly flush out comments  */
        newlines--;
    }
    c = skip_ws();
    if (c == '\n')                              /* 'null' directive */
        goto  ret;
#if MODE == PRE_STANDARD
    token_type = scan_token( c, (workp = work, &workp), work_end);
    if (in_asm && (token_type != NAM            /* In #asm block    */
            || (! str_eq( identifier, "asm")    /* Ignore #anything */
                && ! str_eq( identifier, "endasm"))))   /*   other  */
        goto  skip_line;                /*    than #asm or #endasm  */
    if (token_type != NAM) {
        if (mode == OLD_PREP && token_type == NUM) {    /* # 123 [fname]    */
            strcpy( identifier, "line");
        } else {
            if (compiling)
                cerror( illeg_dir, work, 0L, NULLST);
            else if (warn_level & 8)
                cwarn( illeg_dir, work, 0L, in_skipped);
            goto  skip_line;
        }
    }
#else   /* MODE == STANDARD */
    if (scan_token( c, (workp = work, &workp), work_end) != NAM) {
        if (compiling)
            cerror( illeg_dir, work, 0L, NULLST);
        else if (warn_level & 8)
            cwarn( illeg_dir, work, 0L, in_skipped);
        goto  skip_line;
    }
#endif
    if (identifier[ 2] == EOS)
        identifier[ 3] = EOS;                   /* Diddle           */
    hash = (identifier[ 1] == EOS) ? identifier[ 0]
            : (identifier[ 0] ^ (identifier[ 3] << 1));
    if (strlen( identifier) > 7)
        hash ^= (identifier[ 7] << 1);

    switch (hash) {
    case L_if:      tp = "if";      break;
    case L_ifdef:   tp = "ifdef";   break;
    case L_ifndef:  tp = "ifndef";  break;
#if MODE == STANDARD
    case L_elif:    tp = "elif";    break;
#endif
    case L_else:    tp = "else";    break;
    case L_endif:   tp = "endif";   break;
    case L_define:  tp = "define";  break;
    case L_undef:   tp = "undef";   break;
    case L_line:    tp = "line";    break;
    case L_include: tp = "include"; break;
#if COMPILER == GNUC
    case L_include_next:    tp = "include_next";    break;
#endif
#if MODE == STANDARD
    case L_error:   tp = "error";   break;
    case L_pragma:  tp = "pragma";  break;
#endif
    default:        tp = NULL;      break;
    }

    if (tp != NULL && ! str_eq( identifier, tp)) {  /* Hash conflict*/
        hash = 0;                       /* Unknown directive, will  */
        tp = NULL;                      /*   be handled by do_old() */
    }

    /*
     * hash is set to a unique value corresponding to the control directive.
     */
    if (! compiling) {                      /* Not compiling now    */
        switch (hash) {
        case L_else :                       /* Test the #if's nest, */
#if MODE == STANDARD
        case L_elif :                       /*   if 0, compile.     */
#endif
        case L_endif:                       /* Un-nest #if          */
            break;
        case L_if   :                       /* These can't turn     */
        case L_ifdef:                       /*  compilation on, but */
        case L_ifndef:                      /*   we must nest #if's.*/
            if (&ifstack[ BLK_NEST] < ++ifptr)
                goto  if_nest_err;
#if MODE == STANDARD && BLK_NEST > BLK_NEST90MIN
            if ((warn_level & 8) && &ifstack[ blk_nest_min + 1] == ifptr)
                cwarn( many_nesting, NULLST, (long) blk_nest_min, in_skipped);
#endif
            ifptr->stat = 0;                /* !WAS_COMPILING       */
            ifptr->ifline = line;           /* Line at section start*/
            goto  skip_line;
        default     :                       /* Other directives     */
            if (tp == NULL && (warn_level & 8))
                do_old();                   /* Unknown directive ?  */
            goto  skip_line;                /* Skip the line        */
        }
    }
    macro_line = 0;                         /* Reset error flag     */

    switch (hash) {

    case L_if:
    case L_ifdef:
    case L_ifndef:
        if (&ifstack[ BLK_NEST] < ++ifptr)
            goto  if_nest_err;
#if MODE == STANDARD && BLK_NEST > BLK_NEST90MIN
        if ((warn_level & 4) &&
                &ifstack[ blk_nest_min + 1] == ifptr)
            cwarn( many_nesting, NULLST , (long) blk_nest_min, NULLST);
#endif
        ifptr->stat = WAS_COMPILING;
        ifptr->ifline = line;
#if MODE == STANDARD
        goto  ifdo;

    case L_elif:
        if (ifptr == &ifstack[0])
            goto  nest_err;
        if (ifptr == infile->initif) {
            goto  in_file_nest_err;
        }
        if (ifptr->stat & ELSE_SEEN)
            goto  else_seen_err;
        if ((ifptr->stat & (WAS_COMPILING | TRUE_SEEN)) != WAS_COMPILING) {
            compiling = FALSE;              /* Done compiling stuff */
            goto  skip_line;                /* Skip this group      */
        }
        hash = L_if;
ifdo:
#endif  /* MODE == STANDARD    */
        c = do_if( hash);
#if DEBUG
        if (debug & IF) {
            fprintf( fp_debug
                    , "#if (#elif, #ifdef, #ifndef) evaluate to %s.\n"
                    , compiling ? "TRUE" : "FALSE");
            fprintf( fp_debug, "line %ld: %s", line, infile->buffer);
        }
#endif
        if (c == FALSE) {                   /* Error                */
            compiling = FALSE;              /* Skip this group      */
            goto  skip_line;    /* Prevent an extra error message   */
        }
        break;

    case L_else:
        if (ifptr == &ifstack[0])
            goto  nest_err;
        if (ifptr == infile->initif) {
#if MODE == STANDARD
            goto  in_file_nest_err;
#else
            if (warn_level & 1)
                cwarn( not_in_section, NULLST, 0L, NULLST);
#endif
        }
        if (ifptr->stat & ELSE_SEEN)
            goto  else_seen_err;
        ifptr->stat |= ELSE_SEEN;
        ifptr->elseline = line;
        if (ifptr->stat & WAS_COMPILING) {
            if (compiling || (ifptr->stat & TRUE_SEEN) != 0)
                compiling = FALSE;
            else
                compiling = TRUE;
        }
        break;

    case L_endif:
        if (ifptr == &ifstack[0])
            goto  nest_err;
        if (ifptr <= infile->initif) {
#if MODE == STANDARD
            goto  in_file_nest_err;
#else
            if (warn_level & 1)
                cwarn( not_in_section, NULLST, 0L, NULLST);
#endif
        }
        if (! compiling && (ifptr->stat & WAS_COMPILING))
            wrong_line = TRUE;
        compiling = (ifptr->stat & WAS_COMPILING);
        --ifptr;
        break;

    case L_define:
        do_define( FALSE);
        break;

    case L_undef:
        do_undef();
        break;

    case L_line:
        if ((c = do_line()) > 0) {
            line = c;
            sharp();    /* Putout the new line number and file name */
            infile->line = --line;  /* Next line number is 'line'   */
            newlines = -1;
        } else {            /* Error already diagnosed by do_line()  */
            skip_nl();
        }
        break;

    case L_include:
        if (do_include( FALSE) == TRUE)
            newlines = -1;                  /* To clear line number */
        break;

#if MODE == STANDARD
    case L_error:
        cerror( infile->buffer                              /* _E_  */
                , NULLST, 0L, NULLST);
        break;

    case L_pragma:
        do_pragma();
        break;
#endif

    default:                /* Non-Standard or unknown directives   */
        do_old();
        break;
    }

    switch (hash) {
    case L_if       :
    case L_define   :
    case L_line     :
#if MODE == STANDARD
    case L_error    :
#endif
        goto  skip_line;    /* To prevent duplicate error message   */
#if COMPILER == GNUC
    case L_include_next  :
        newlines = -1;
#endif
    case L_include  :
#if MODE == STANDARD
    case L_pragma   :
#endif
        break;              /* Already read over the line           */
    default :               /* L_else, L_endif, L_undef, etc.       */
#if MODE == PRE_STANDARD
        /*
         * Ignore the rest of the #control line so you can write
         *          #if     foo
         *          #endif  foo
         */
        if (mode == OLD_PREP) {
            skip_nl();
            break;
        }
#endif
        if (skip_ws() != '\n') {
#if MODE == STANDARD
            cerror( excess, infile->bptr-1, 0L, NULLST);
#else
            if (warn_level & 1)
                cwarn( excess, infile->bptr-1, 0L, NULLST);
#endif
            skip_nl();
        }
    }
    goto  ret;

#if MODE == STANDARD
in_file_nest_err:
    cerror( not_in_section, NULLST, 0L, NULLST);
    goto  skip_line;
#endif
nest_err:
    cerror( "Not in a #if (#ifdef) section", NULLST, 0L, NULLST);   /* _E_  */
    goto  skip_line;
else_seen_err:
    cerror( "Already seen #else at line %.0s%ld"            /* _E_  */
            , NULLST, ifptr->elseline, NULLST);
skip_line:
    skip_nl();                              /* Ignore rest of line  */
    goto  ret;

if_nest_err:
    cfatal( many_nesting, NULLST, (long) BLK_NEST, NULLST);

ret:
    in_directive = FALSE;
    keep_comments = cflag && compiling && !no_output;
    return  (wrong_line ? 0 : ++newlines);
}

static int
#if PROTO
do_if( int hash)
#else
do_if( hash)
    int     hash;
#endif
/*
 * Process an #if (#elif), #ifdef or #ifndef.  The latter two are straight-
 * forward, while #if needs a subroutine of its own to evaluate the
 * expression.
 * do_if() is called only if compiling is TRUE.  If false, compilation is
 * always supressed, so we don't need to evaluate anything.  This supresses
 * unnecessary warnings.
 */
{
    register int    c;
    register int    found;

    if ((c = skip_ws()) == '\n') {
        unget();
        cerror( no_arg, NULLST, 0L, NULLST);
        return  FALSE;
    }
    if (hash == L_if) {                 /* #if or #elif             */
        unget();
        found = (eval() != 0L);         /* Evaluate expression      */
        hash = L_ifdef;                 /* #if is now like #ifdef   */
    } else {                            /* #ifdef or #ifndef        */
        if (scan_token( c, (workp = work, &workp), work_end) != NAM) {
            cerror( not_ident, work, 0L, NULLST);
            return  FALSE;      /* Next token is not an identifier  */
        }
        found = (look_id( identifier) != NULL); /* Look in table    */
    }
    if (found == (hash == L_ifdef)) {
        compiling = TRUE;
        ifptr->stat |= TRUE_SEEN;
    } else {
        compiling = FALSE;
    }
    return  TRUE;
}

static long
#if PROTO
do_line( void)
#else
do_line()
#endif
/*
 * Parse the line to update the line number and "filename" field for the next
 * input line.
 * Values returned are as follows:
 *  -1:     syntax error or out-of-range error (diagnosed by do_line(),
 *          eval_num()).
 *  [1,32767]:  legal line number for C90, [1,2147483647] for C99.
 * Line number [32768,2147483647] in C90 mode is only warned (not an error).
 * do_line() always absorbs the line (except the <newline>).
 */
{
    const char * const  not_digits
        = "Line number \"%s\" isn't a decimal digits sequence"; /* _E_ _W1_ */
#if MODE == STANDARD
    const char * const  out_of_range
        = "Line number \"%s\" is out of range of [1,%ld]";      /* _E_ _W1_ */
    register int    token_type;
#endif
    VAL_SIGN *      valp;
    char *  save;
    int     c;

    if ((c = skip_ws()) == '\n') {
        cerror( no_arg, NULLST, 0L, NULLST);
        unget();                            /* Push back <newline>  */
        return  -1L;                /* Line number is not changed   */
    }

#if MODE == STANDARD
    token_type = get_unexpandable( c, FALSE);
    if (macro_line == MACRO_ERROR)          /* Unterminated macro   */
        return  -1L;                        /*   already diagnosed. */
    if (token_type == NO_TOKEN)     /* Macro expanded to 0 token    */
        goto  no_num;
    if (token_type != NUM)
#else   /* MODE == PRE_STANDARD */
    if (scan_token( c, (workp = work, &workp), work_end) != NUM)
#endif  /* MODE == PRE_STANDARD */
        goto  illeg_num;
    for (workp = work; *workp != EOS; workp++) {
        if (! isdigit( *workp & UCHARMAX)) {
#if MODE == STANDARD
            cerror( not_digits, work, 0L, NULLST);
            return  -1L;
#else
            if (warn_level & 1)
                cwarn( not_digits, work, 0L, NULLST);
#endif
        }
    }
    valp = eval_num( work);                 /* Evaluate number      */
    if (valp->sign == VAL_ERROR) {  /* Error diagnosed by eval_num()*/
        return  -1;
#if MODE == STANDARD
    } else if (line_limit < valp->val || valp->val <= 0L) {
        if (valp->val < LINE99LIMIT && valp->val > 0L) {
            if (warn_level & 1)
                cwarn( out_of_range, work, line_limit, NULLST);
        } else {
            cerror( out_of_range, work, line_limit, NULLST);
            return  -1L;
        }
#endif
    }

#if MODE == STANDARD
    token_type = get_unexpandable( skip_ws(), FALSE);
    if (macro_line == MACRO_ERROR)
        return  -1L;
    if (token_type != STR) {
        if (token_type == NO_TOKEN) {       /* Filename is absent   */
            return  (long) valp->val;
        } else {
            goto  not_fname;    /* Expanded macro should be a quoted string */
        }
    }
#else   /* MODE == PRE_STANDARD */
    if ((c = skip_ws()) == '\n') {
        unget();
        return  (long) valp->val;
    }
    if (scan_token( c, (workp = work, &workp), work_end) != STR)
        goto  not_fname;
#endif  /* MODE == PRE_STANDARD */
    *(workp - 1) = EOS;                     /* Ignore right '"'     */
    save = save_string( &work[ 1]);         /* Ignore left '"'      */

#if MODE == STANDARD
    if (get_unexpandable( skip_ws(), FALSE) != NO_TOKEN) {
        cerror( excess, work, 0L, NULLST);
        free( save);
        return  -1L;
    }
#else   /* MODE == PRE_STANDARD */
    if (mode == OLD_PREP) {
        skip_nl();
        unget();
    } else if ((c = skip_ws()) == '\n') {
        unget();
    } else {
        if (warn_level & 1) {
            scan_token( c, (workp = work, &workp), work_end);
            cwarn( excess, work, 0, NULLST);
        }
        skip_nl();
        unget();
    }
#endif  /* MODE == PRE_STANDARD */

    infile->filename = save;                /* New file name        */
                                /* Do not free() infile->filename   */
    return  (long) valp->val;               /* New line number      */

#if MODE == STANDARD
no_num:
    cerror( "No line number", NULLST, 0L, NULLST);          /* _E_  */
    return  -1L;
#endif
illeg_num:
    cerror( "Not a line number \"%s\"", work, 0L, NULLST);  /* _E_  */
    return  -1L;
not_fname:
    cerror( "Not a file name \"%s\"", work, 0L, NULLST);    /* _E_  */
    return  -1L;
}

/*
 *                  M a c r o  D e f i n i t i o n s
 *
 * Edit History (cpp4.c) of original version
 * 31-Aug-84    MM      USENET net.sources release
 *  2-May-85    MM      Latest revision
 */

/*
 * look_id()    Looks for the name in the defined symbol table.  Returns a
 *              pointer to the definition if found, or NULL if not present.
 * install()    Installs the definition.  Updates the symbol table.
 * undefine()   Deletes the definition from the symbol table.
 */

/*
 * Global work[] are used to store #define parameter lists and parlist[]
 * point to them.
 * 'nargs' contains the actual number of parameters stored.
 */
static char *   parlist[ NMACPARS]; /* -> Start of each parameter   */
static size_t   parlen[ NMACPARS];  /* Length of parameter name     */
static int      nargs;          /* Number of parameters         */
static char *   token_p;        /* Pointer to the token scanned */
static char *   repl_base;      /* Base of buffer for repl-text */
static char *   repl_end;       /* End of buffer for repl-text  */
static const char * const   no_ident = "No identifier";     /* _E_  */

DEFBUF *
#if PROTO
do_define( int ignore_redef)
#else
do_define( ignore_redef)
    int     ignore_redef;   /* Do not redefine   */
#endif
/*
 * Called from control() when a #define is scanned or called from do_options()
 *      when a -D option is scanned.  This module parses formal parameters
 *      by get_parm() and the replacement text by get_repl().
 *
 * There is some special case code to distinguish
 *      #define foo     bar     --  object-like macro
 * from #define foo()   bar     --  function-like macro with no parameter
 *
 * Also, we make sure that
 *      #define foo     foo
 * expands to "foo" but doesn't put MCPP into an infinite loop.
 *
 * A warning is printed if you redefine a symbol with a non-identical
 * text.  I.e,
 *      #define foo     123
 *      #define foo     123
 * is ok, but
 *      #define foo     123
 *      #define foo     +123
 * is not.
 *
 * The following subroutines are called from do_define():
 * get_parm()   parsing and remembering parameter names.
 * get_repl()   parsing and remembering replacement text.
 *
 * The following subroutines are called from get_repl():
 * is_formal()  is called when an identifier is scanned.  It checks through
 *              the array of formal parameters.  If a match is found, the
 *              identifier is replaced by a control byte which will be used
 *              to locate the parameter when the macro is expanded.
 * def_stringization()  is called when '#' operator is scanned.  It surrounds
 *              the token to stringize with magic-codes.
 *
 * MODEs other than STANDARD ignore difference of parameter names in macro
 * redefinition.
 */
{
#if MODE == STANDARD
    const char * const  predef = "\"%s\" shouldn't be redefined";   /* _E_  */
#endif
    char    repl_list[ NMACWORK + IDMAX];   /* Replacement text     */
    char    macroname[ IDMAX + 1];  /* Name of the macro defining   */
    register DEFBUF *   defp;       /* -> Old definition            */
    DEFBUF **   prevp;      /* -> Pointer to previous def in list   */
    int     c;
    int     redefined;                      /* TRUE if redefined    */
    int     dnargs;                         /* defp->nargs          */
    int     cmp;            /* Result of definition comparison      */

    repl_base = repl_list;
    repl_end = & repl_list[ NMACWORK];
    if ((c = skip_ws()) == '\n') {
        cerror( no_ident, NULLST, 0L, NULLST);
        unget();
        return  NULL;
    } else if (scan_token( c, (workp = work, &workp), work_end) != NAM) {
        cerror( not_ident, work, 0L, NULLST);
        return  NULL;
    } else {
        prevp = look_prev( identifier, &cmp);
        defp = *prevp;
#if MODE == STANDARD
        if (cmp || defp->push) {    /* Not known or 'pushed' macro  */
#else
        if (cmp) {
#endif
#if MODE == STANDARD
            if (str_eq( identifier, "defined")
                    || ((stdc_val || cplus)
                        &&  str_eq( identifier, "__VA_ARGS__"))) {
                cerror(
            "\"%s\" shouldn't be defined", identifier, 0L, NULLST); /* _E_  */
                return  NULL;
            }
#endif  /* MODE == STANDARD    */
            redefined = FALSE;              /* Quite new definition */
        } else {                            /* It's known:          */
            if (ignore_redef)
                return  defp;
            dnargs = (defp->nargs == DEF_NOARGS-1) ? DEF_NOARGS : defp->nargs;
#if MODE == STANDARD
            if (dnargs < DEF_NOARGS - 1     /* Standard predefined  */
#if OK_PRAGMA_OP
                    || dnargs == DEF_PRAGMA /* _Pragma() pseudo-macro       */
#endif
                    ) {
                cerror( predef, identifier, 0L, NULLST);
                return  NULL;
            } else
#endif  /* MODE == STANDARD    */
                redefined = TRUE;           /* Remember this fact   */
        }
    }
    strcpy( macroname, identifier);         /* Remember the name    */

    in_define = TRUE;                       /* Recognize '#', '##'  */
    if (get_parm() == FALSE) {              /* Get parameter list   */
        in_define = FALSE;
        return  NULL;                       /* Syntax error         */
    }
    if (get_repl( macroname) == FALSE) {    /* Get replacement text */
        in_define = FALSE;
        return  NULL;                       /* Syntax error         */
    }

    in_define = FALSE;
    if (redefined) {
        if (dnargs != nargs || ! str_eq( defp->repl, repl_list)
#if MODE == STANDARD
                || (mode == STD && ! str_eq( defp->parmnames, work))
#endif
                ) {             /* Warn if differently redefined    */
            if (warn_level & 1) {
                cwarn(
            "The macro is redefined", NULLST, 0L, NULLST);  /* _W1_ */
                if (! no_source_line)
                    dump_a_def( "    previously macro", defp, FALSE, FALSE
                            , TRUE, fp_err);
            }
        } else {                        /* Identical redefinition   */
            return  defp;
        }
    }                                   /* Else new or re-definition*/
    defp = install( macroname, nargs, work, repl_list, prevp, cmp);
#if MODE == STANDARD
    if (mode == STD && cplus && id_operator( macroname) && (warn_level & 1))
        /* These are operators, not identifiers, in C++98   */
        cwarn( "\"%s\" is defined as macro", macroname      /* _W1_ */
                , 0L, NULLST);
#endif
    return  defp;
}

static int
#if PROTO
get_parm( void)
#else
get_parm()
#endif
/*
 *   Get parameters i.e. numbers into nargs, name into work[], name-length
 * into parlen[].
 *   Return TRUE if the parameters are legal, else return FALSE.
 *   In STANDARD mode preprocessor must remember the parameter names, only for
 * checking the validity of macro redefinitions.  This is required by the
 * Standard (what an overhead !).
 */
{
    const char * const  many_parms
            = "More than %.0s%ld parameters";       /* _E_ _W4_     */
    const char * const  illeg_parm
            = "Illegal parameter \"%s\"";           /* _E_          */
    int     token_type;
    register int    c;

    parlist[ 0] = workp = work;
    work[ 0] = EOS;
#if MODE == STANDARD
    /* POST_STD mode    */
    insert_sep = NO_SEP;    /* Clear the inserted token separator   */
#endif
    c = get();

    if (c == '(') {                         /* With arguments?      */
        nargs = 0;                          /* Init parms counter   */
        if (skip_ws() == ')')
            return  TRUE;                   /* Macro with 0 parm    */
        else
            unget();

        do {                                /* Collect parameters   */
            if (nargs >= NMACPARS) {
                cerror( many_parms, NULLST, (long) NMACPARS, NULLST);
                return  FALSE;
            }
            parlist[ nargs] = workp;        /* Save its start       */
            if ((token_type = scan_token( c = skip_ws(), &workp, work_end))
                    != NAM) {
                if (c == '\n') {
                    break;
                } else if (c == ',' || c == ')') {
                    cerror( "Empty parameter", NULLST, 0L, NULLST); /* _E_  */
                    return  FALSE;
#if MODE == STANDARD
                /*
                 * Enable variable argument macro which is the feature of C99.
                 * We enable this even on C90 or C++ for GNU C compatibility.
                 */
                } else if ((stdc_val || cplus) && token_type == OPE
                        && openum == OP_ELL) {
                    if (skip_ws() != ')') {
                        cerror( "\"...\" isn't the last parameter", /* _E_  */
                                NULLST, 0L, NULLST);
                        return  FALSE;
                    }
                    parlen[ nargs++] = 3;
                    nargs |= VA_ARGS;
                    goto  ret;
#endif
                } else {
                    cerror( illeg_parm , parlist[ nargs], 0L, NULLST);
                    return  FALSE;          /* Bad parameter syntax */
                }
            }
#if MODE == STANDARD
            if ((stdc_val || cplus) && str_eq( identifier, "__VA_ARGS__")) {
                cerror( illeg_parm, parlist[ nargs], 0L, NULLST);
                return  FALSE;
                /* __VA_ARGS__ should not be used as a parameter    */
            }
#endif
            if (is_formal( parlist[ nargs], FALSE)) {
                cerror( "Duplicate parameter name \"%s\""   /* _E_  */
                        , parlist[ nargs], 0L, NULLST);
                return  FALSE;
            }
            parlen[ nargs] = (size_t) (workp - parlist[ nargs]);
                                            /* Save length of param */
            *workp++ = ',';
            nargs++;
        } while ((c = skip_ws()) == ',');   /* Get another parameter*/

        *--workp = EOS;                     /* Remove excessive ',' */
        if (c != ')') {                     /* Must end at )        */
            unget();                        /* Push back '\n'       */
            cerror(
        "Missing \",\" or \")\" in parameter list \"(%s\""  /* _E_  */
                    , work, 0L, NULLST);
            return  FALSE;
        }
    } else {
        /*
         * DEF_NOARGS is needed to distinguish between
         * "#define foo" and "#define foo()".
         */
        nargs = DEF_NOARGS;                 /* Object-like macro    */
        unget();
    }
#if MODE == STANDARD
ret:
#if NMACPARS > NMACPARS90MIN
    if ((warn_level & 4) && (nargs & ~VA_ARGS) > n_mac_pars_min)
        cwarn( many_parms, NULLST , (long) n_mac_pars_min , NULLST);
#endif
#endif
    return  TRUE;
}

static int
#if PROTO
get_repl( const char * macroname)
#else
get_repl( macroname)
    char *  macroname;
#endif
/*
 *   Get replacement text i.e. names of formal parameters are converted to
 * the magic numbers, and operators #, ## is converted to magic characters.
 *   Return TRUE if replacement list is legal, else return FALSE.
 *   Any token separator in the text is converted to a single space, no token
 * sepatator is inserted by MCPP.  Those are required by the Standard for
 * stringizing of an argument by # operator.
 *   In POST_STD mode, inserts a space between any tokens in source (except a
 * macro name and the next '(' in macro definition), hence presence or absence
 * of token separator makes no difference.
 */
{
#if MODE == STANDARD
    const char * const  mixed_ops
    = "Macro with mixing of ## and # operators isn't portable";     /* _W4_ */
    const char * const  multiple_cats
    = "Macro with multiple ## operators isn't portable";    /* _W4_ */
    char *  prev_token = NULL;              /* Preceding token      */
    char *  prev_prev_token;                /* Pre-preceding token  */
    int     multi_cats = FALSE;             /* Multiple ## operators*/
#endif
    register int    c;
    int     token_type;                     /* Type of token        */
    char *  temp;
    char *  repl_cur = repl_base;   /* Pointer into repl-text buffer*/

    *repl_cur = EOS;
    token_p = NULL;
#if MODE == STANDARD
    c = get();
    unget();
    if (((type[ c] & SPA) == 0) && (nargs < 0) && (warn_level & 1))
        cwarn( "No space between macro name \"%s\" and repl-text"   /* _W1_ */
                , macroname, 0L, NULLST);
#endif
    c = skip_ws();                           /* Get to the body      */

    while (c != CHAR_EOF && c != '\n') {
#if MODE == STANDARD
        prev_prev_token = prev_token;
        prev_token = token_p;
#endif
        token_p = repl_cur;                 /* Remember the pointer */
        token_type = scan_token( c, &repl_cur, repl_end);

        switch (token_type) {
#if MODE == STANDARD
        case OPE:                   /* Operator or punctuator       */
            switch (openum) {
            case OP_CAT:                    /* ##                   */
                if (prev_token == NULL) {
                    cerror( "No token before ##"            /* _E_  */
                            , NULLST, 0L, NULLST);
                    return  FALSE;
                } else if (*prev_token == CAT) {
                    cerror( "## after ##", NULLST, 0L, NULLST);     /* _E_  */
                    return  FALSE;
                } else if (prev_prev_token && *prev_prev_token == CAT) {
                    multi_cats = TRUE;
                } else if (prev_prev_token && *prev_prev_token == ST_QUOTE
                        && (warn_level & 4)) {      /* # parm ##    */
                    cwarn( mixed_ops, NULLST, 0L, NULLST);
                }
                repl_cur = token_p;
                *repl_cur++ = CAT;          /* Convert to CAT       */
                break;
            case OP_STR:                    /* #                    */
                if (nargs < 0)              /* In object-like macro */
                    break;                  /* '#' is an usual char */
                if (prev_token && *prev_token == CAT
                        && (warn_level & 4))        /* ## #         */
                    cwarn( mixed_ops, NULLST, 0L, NULLST);
                repl_cur = token_p;         /* Overwrite on #       */
                if ((temp = def_stringization( repl_cur)) == NULL) {
                    return  FALSE;          /* Error                */
                } else {
                    repl_cur = temp;
                }
                break;
            default:                    /* Any operator as it is    */
                break;
            }
            break;
#endif  /* MODE == STANDARD    */
        case NAM:
        /*
         * Replace this name if it's a parm.  Note that the macro name is a
         * possible replacement token.  We stuff DEF_MAGIC in front of the
         * token which is treated as a LETTER by the token scanner and eaten
         * by the macro expanding routine.  This prevents the macro expander
         * from looping if someone writes "#define foo foo".
         */
            temp = is_formal( identifier, TRUE);
            if (temp == NULL) {             /* Not a parameter name */
#if MODE == STANDARD
                if ((stdc_val || cplus)
                            && str_eq( identifier, "__VA_ARGS__")) {
                    cerror( "\"%s\" without corresponding \"...\""  /* _E_  */
                            , identifier, 0L, NULLST);
                    return  FALSE;
                }
                if ((temp = mgtoken_save( macroname)) != NULL)
                    repl_cur = temp;        /* Macro name           */
#endif
            } else {                        /* Parameter name       */
                repl_cur = temp;
            }
            break;

#if MODE == PRE_STANDARD
        case STR:                           /* String in mac. body  */
        case CHR:                           /* Character constant   */
            if (mode == OLD_PREP)
                repl_cur = str_parm_scan( repl_cur);
            break;
        case SEP:
            if (mode == OLD_PREP && c == COM_SEP)
                repl_cur--;                 /* Skip comment now     */
            break;
#endif
        default:                            /* Any token as it is   */
            break;
        }

        c = get();
        if (c == ' ') {
            *repl_cur++ = ' ';
            c = get();
        }
    }

    unget();                                /* For control check    */
#if MODE == STANDARD
    if (token_p && *token_p == CAT) {
        cerror( "No token after ##", NULLST, 0L, NULLST);   /* _E_  */
        return  FALSE;
    }
    if (multi_cats && (warn_level & 4))
        cwarn( multiple_cats, NULLST, 0L, NULLST);
    if ((nargs & VA_ARGS) && stdc_ver < 199901L && (warn_level & 2))
        /* Variable arg macro is the spec of C99, not C90 nor C++98 */
        cwarn( "Variable argument macro is defined",        /* _W2_ */
                NULLST, 0L, NULLST);
#endif
    *repl_cur = EOS;                        /* Terminate work       */

    return  TRUE;
}

static char *
#if PROTO
is_formal( const char * name, int conv)
#else
is_formal( name, conv)
    char *      name;
    int         conv;                   /* Convert to magic number? */
#endif
/*
 * If the identifier is a formal parameter, save the MAC_PARM and formal
 * offset, returning the advanced pointer into the replacement text.
 * Else, return NULL.
 */
{
    register char *   repl_cur;
    int     i;

    for (i = 0; i < (nargs & ~VA_ARGS); i++) { /* For each parameter   */
        if ((strlen( name) == parlen[ i]
                            /* Note: parlist[] are comma separated  */
                    && memcmp( name, parlist[ i], parlen[ i]) == 0)
#if MODE == STANDARD
                || ((nargs & VA_ARGS) && i == (nargs & ~VA_ARGS) - 1 && conv
                    && memcmp( name, "__VA_ARGS__", 12) == 0)
#endif
            ) {
                                            /* If it's known        */
            if (conv) {
                repl_cur = token_p;         /* Overwrite on the name*/
                *repl_cur++ = MAC_PARM;     /* Save the signal      */
                *repl_cur++ = i + 1;        /* Save the parm number */
                return  repl_cur;           /* Return "gotcha"      */
            } else {
                return  parlist[ i];        /* Duplicate parm name  */
            }
        }
    }

    return  NULL;                           /* Not a formal param   */
}

#if MODE == STANDARD

static char *
#if PROTO
def_stringization( char * repl_cur)
#else
def_stringization( repl_cur)
    char *  repl_cur;
#endif
/*
 * Define token stringization.
 * We store a magic cookie (which becomes " on output) preceding the
 * parameter as an operand of # operator.
 * Return the current pointer into replacement text if the token following #
 * is a parameter name, else return NULL.
 */
{
    int     c;
    register char *     temp;

    *repl_cur++ = ST_QUOTE;                 /* prefix               */
    if ((c = get()) == ' ') {
        *repl_cur++ = ' ';
        c = get();
    }
    token_p = repl_cur;                     /* Remember the pointer */
    if (scan_token( c, &repl_cur, repl_end) == NAM) {
        if ((temp = is_formal( identifier, TRUE)) != NULL) {
            repl_cur = temp;
            return  repl_cur;
        }
    }
    cerror( "Not a formal parameter \"%s\"", token_p, 0L, NULLST);  /* _E_  */
    return  NULL;
}

static char *
#if PROTO
mgtoken_save( const char * macroname)
#else
mgtoken_save( macroname)
    char *  macroname;
#endif
/*
 * A magic cookie is inserted if the token is identical to the macro name,
 * so the expansion doesn't recurse.
 * Return the advanced pointer into the replacement text or NULL.
 */
{
    register char *   repl_cur;

    if (str_eq( macroname, identifier)) {   /* Macro name in body   */
        repl_cur = token_p;                 /* Overwrite on token   */
        *repl_cur++ = DEF_MAGIC;            /* Save magic marker    */
        repl_cur = stpcpy( repl_cur, identifier);
                                            /* And save the token   */
        return  repl_cur;
    } else {
        return  NULL;
    }
}

#endif  /* MODE == STANDARD    */

#if MODE == PRE_STANDARD

static char *
#if PROTO
str_parm_scan( char * string_end)
#else
str_parm_scan( string_end)
    char *  string_end;
#endif
/*
 * String parameter scan.
 * This code -- if enabled -- recognizes a formal parameter in a string
 * literal or in a character constant.
 *      #define foo(bar, v) printf("%bar\n", v)
 *      foo( d, i)
 * expands to:
 *      printf("%d\n", i)
 * str_parm_scan() return the advanced pointer into the replacement text.
 * This has been superceded by # stringizing and string concatenation.
 * This routine is called only in OLD_PREP mode.
 */
{
    int     delim;
    int     c;
    char *  tp;
    register char *     wp;     /* Pointer into the quoted literal  */

    delim = *token_p;
    unget_string( ++token_p, NULLST);
    /* Pseudo-token-parsing in a string literal */
    wp = token_p;
    while ((c = get()) != delim) {
        token_p = wp;
        if (scan_token( c, &wp, string_end) != NAM)
            continue;
        if ((tp = is_formal( token_p, TRUE)) != NULL)
            wp = tp;
    }
    *wp++ = delim;
    return  wp;
}

#endif  /* MODE == PRE_STANDARD */

static void
#if PROTO
do_undef( void)
#else
do_undef()
#endif
/*
 * Remove the symbol from the defined list.
 * Called from control().
 */
{
    register DEFBUF *   defp;
    int     c;

    if ((c = skip_ws()) == '\n') {
        cerror( no_ident, NULLST, 0L, NULLST);
        unget();
        return;
    }
    if (scan_token( c, (workp = work, &workp), work_end) != NAM) {
        cerror( not_ident, work, 0L, NULLST);
        skip_nl();
        unget();
    } else {
        if ((defp = look_id( identifier)) == NULL) {
            if (warn_level & 8)
                cwarn( "\"%s\" wasn't defined"              /* _W8_ */
                        , identifier, 0L, NULLST);
#if MODE == STANDARD
        } else if (defp->nargs < DEF_NOARGS - 1 /* Standard predef  */
#if OK_PRAGMA_OP
                || defp->nargs == DEF_PRAGMA    /* _Pragma() pseudo-macro   */
#endif
                ) {
            cerror( "\"%s\" shouldn't be undefined"         /* _E_  */
                    , identifier, 0L, NULLST);
#endif
        } else {
#if MODE == STANDARD
            c = skip_ws();
            unget();
            if (c != '\n')                      /* Trailing junk    */
                return;
            else
                undefine( identifier);
#else
            undefine( identifier);
#endif
        }
    }
}

/*
 *                  C P P   S y m b o l   T a b l e s
 *
 * SBSIZE defines the number of hash-table slots for the symbol table.
 * It must be a power of 2.
 */

/* Symbol table queue headers.  */
static DEFBUF *     symtab[ SBSIZE];
#if MODE == STANDARD
static long         num_of_macro;
#endif

DEFBUF *
#if PROTO
look_id( const char * name)
#else
look_id( name)
    char *      name;
#endif
/*
 * Look for the identifier in the symbol table.
 * If found, return the table pointer;  Else return NULL.
 */
{
    DEFBUF **   prevp;
    int         cmp;

    prevp = look_prev( name, &cmp);

#if MODE == STANDARD
    return ((cmp == 0 && (*prevp)->push == 0) ? *prevp : (DEFBUF *)NULL);
#else
    return ((cmp == 0) ? *prevp : (DEFBUF *)NULL);
#endif
}

DEFBUF **
#if PROTO
look_prev( const char * name, int * cmp)
#else
look_prev( name, cmp)
    char *  name;                           /* Name of the macro    */
    int *   cmp;                            /* Result of comparison */
#endif
/*
 * Look for the place to insert the macro definition.
 * Return a pointer to the previous member in the linked list.
 */
{
    register const char *   np;
    DEFBUF **   prevp;
    DEFBUF *    dp;
    size_t      s_name;
    int         hash;

    for (hash = 0, np = name; *np != EOS; )
        hash += *np++;
    hash += s_name = (size_t)(np - name);
    s_name++;
    prevp = & symtab[ hash & SBMASK];
    *cmp = -1;                              /* Initialize !         */

    while ((dp = *prevp) != NULL) {
        if ((*cmp = memcmp( dp->name, name, s_name)) >= 0)
            break;
        prevp = &dp->link;
    }

    return  prevp;
}

DEFBUF *
#if PROTO
look_and_install( const char * name, int numargs, const char * parmnames
        , const char * repl)
#else
look_and_install( name, numargs, parmnames, repl)
    char *  name;                           /* Name of the macro    */
    int     numargs;                        /* The numbers of parms */
    char *  parmnames;          /* Names of parameters concatenated */
    char *  repl;                           /* Replacement text     */
#endif
/*
 * Look for the name and (re)define it.
 * Returns a pointer to the definition block.
 * Returns NULL if the symbol was Standard-predefined.
 */
{
    DEFBUF **   prevp;          /* Place to insert definition       */
    DEFBUF *    defp;                       /* New definition block */
    int         cmp;    /* Result of comparison of new name and old */

    prevp = look_prev( name, &cmp);
    defp = install( name, numargs, parmnames, repl, prevp, cmp);
    return  defp;
}

DEFBUF *
#if PROTO
install( const char * name, int numargs, const char * parmnames
        , const char * repl, DEFBUF ** prevp, int cmp)
#else
install( name, numargs, parmnames, repl, prevp, cmp)
    char *  name;                           /* Name of the macro    */
    int     numargs;                        /* The numbers of parms */
    char *  parmnames;          /* Names of parameters concatenated */
    char *  repl;                           /* Replacement text     */
    DEFBUF **  prevp;           /* The place to insert definition   */
    int     cmp;        /* Result of comparison of new name and old */
#endif
/*
 * Enter this name in the lookup table.
 * Returns a pointer to the definition block.
 * Returns NULL if the symbol was Standard-predefined.
 */
{
    register DEFBUF *   dp;
    register DEFBUF *   defp;
    size_t              s_name, s_parmnames, s_repl;

    defp = *prevp;
    if (cmp == 0 && defp->nargs < DEF_NOARGS - 1)
        return  NULL;                       /* Standard predefined  */
    s_parmnames = 0;
#if DEBUG
    if (parmnames == NULL || repl == NULL)      /* Shouldn't happen */
        cfatal( "Bug: Illegal macro installation of \"%s\"" /* _F_  */
                , name, 0L, NULLST);    /* Use "" instead of NULL   */
#endif
    s_name = strlen( name);
#if MODE == STANDARD
    if (mode == STD)
        s_parmnames = strlen( parmnames) + 1;
#endif
    s_repl = strlen( repl) + 1;
    dp = (DEFBUF *)
        xmalloc( sizeof (DEFBUF) + s_name + s_parmnames + s_repl);
    if (cmp
#if MODE == STANDARD
                || (*prevp)->push
#endif
                ) {                     /* New definition           */
        dp->link = defp;                /* Insert to linked list    */
        *prevp = dp;
    } else {                            /* Redefinition             */
        dp->link = defp->link;          /* Replace old def with new */
        *prevp = dp;
        free( defp);
    }
    dp->nargs = numargs;
#if MODE == STANDARD
    dp->push = 0;
#endif
#if MODE == STANDARD
    dp->parmnames = (char *)dp + sizeof (DEFBUF) + s_name;
    dp->repl = dp->parmnames + s_parmnames;
    if (mode == STD)
        memcpy( dp->parmnames, parmnames, s_parmnames);
#else
    dp->repl = (char *)dp + sizeof (DEFBUF) + s_name;
#endif
    memcpy( dp->name, name, s_name + 1);
    memcpy( dp->repl, repl, s_repl);
#if DEBUG                   /* Remember where the macro is defined  */
    dp->dir = *inc_dirp;
    dp->fname = cur_fname;
    dp->mline = line;
#endif
#if MODE == STANDARD
    if (cmp && ++num_of_macro == n_macro_min + 1 && n_macro_min
            && (warn_level & 4))
        cwarn( "More than %.0s%ld macros defined"           /* _W4_ */
                , NULLST , n_macro_min , NULLST);
#endif
    return  dp;
}

int
#if PROTO
undefine( const char * name)
#else
undefine( name)
    char *  name;                           /* Name of the macro    */
#endif
/*
 * Delete the macro definition from the symbol table.
 * Returns TRUE, if deleted;
 * Else returns FALSE (when the macro was not defined or was predefined).
 */
{
    register DEFBUF **  prevp;  /* Preceding definition in list     */
    DEFBUF *            dp;             /* Definition to delete     */
    int                 cmp;

    prevp = look_prev( name, &cmp);
    dp = *prevp;                        /* Definition to delete     */
    if (cmp || dp->nargs < DEF_NOARGS - 1)
        return  FALSE;                      /* Standard predefined  */
#if MODE == STANDARD
    if (dp->push)
        return  FALSE;                  /* 'Pushed' macro           */
#endif
    *prevp = dp->link;          /* Link the previous and the next   */
    free( dp);                          /* Delete the definition    */
#if MODE == STANDARD
    num_of_macro--;
#endif
    return  TRUE;
}

static void
#if PROTO
dump_repl( const DEFBUF * dp, FILE * fp)
#else
dump_repl( dp, fp)
    const DEFBUF *  dp;
    FILE *          fp;
#endif
/*
 * Dump replacement text.
 */
{
#if MODE == STANDARD
    int     numargs = dp->nargs;
    char *  cp1;
    size_t  i;
#endif
    int     c;
    register const char *   cp;

    for (cp = dp->repl; (c = *cp++ & UCHARMAX) != EOS; ) {

        switch (c) {
        case MAC_PARM:                              /* Parameter    */
            c = (*cp++ & UCHARMAX) - 1;
#if MODE == STANDARD
            if ((numargs & VA_ARGS) && c == (numargs & ~VA_ARGS) - 1) {
                fputs( "__VA_ARGS__", fp);
            } else {
                if (mode == STD) {
                    for (i = 0, cp1 = parlist[ c]; i < parlen[ c]; i++)
                        fputc( *cp1++, fp);
                } else {
                    fputc( 'a' + c % 26, fp);
                    if (c > 26)
                        fputc( '0' + c / 26, fp);
                }
            }
#else
            fputc( 'a' + c % 26, fp);
            if (c > 26)
                fputc( '0' + c / 26, fp);
#endif
            break;
#if MODE == STANDARD
        case DEF_MAGIC:
            break;
        case CAT:
            fputs( "##", fp);
            break;
        case ST_QUOTE:
            fputs( "#", fp);
            break;
#else
        case COM_SEP:
            if (mode == OLD_PREP)
                fputs( "/**/", fp);
            break;
#endif
        default:
            fputc( c, fp);
            break;
        }
    }
}

/*
 * If the compiler is so-called "one-pass" compiler, compiler-predefined
 * macros are commented out to avoid redefinition.
 */
#if ONE_PASS
#define CAN_REDEF   DEF_NOARGS
#else
#define CAN_REDEF   (DEF_NOARGS - 1)
#endif

void
#if PROTO
dump_a_def( const char * why, const DEFBUF * dp, int newdef, int dDflag
    , int comment, FILE * fp)
#else
dump_a_def( why, dp, newdef, dDflag, comment, fp)
    const char *    why;
    DEFBUF *        dp;
    int     newdef;         /* TRUE if parmnames are currently in parlist[] */
    int     dDflag;         /* TRUE if -dD option is used (for GNU C)       */
    int     comment;        /* Show location of the definition in comment   */
    FILE *  fp;
#endif
/*
 * Dump a macro definition.
 */
{
#if MODE == STANDARD
    char *  cp, * cp1;
#endif
    int     numargs = dp->nargs & ~VA_ARGS;
    int     commented;                      /* To be commented out  */
    register int    i;

#if MODE == STANDARD && OK_PRAGMA_OP
    if (numargs == DEF_PRAGMA)              /* _Pragma pseudo-macro */
        return;
#endif
    if ((numargs < CAN_REDEF) || (dDflag && (numargs == DEF_NOARGS - 1)))
        commented = TRUE;
#if MODE == STANDARD
    else if (dp->push)
        commented = TRUE;
#endif
    else
        commented = FALSE;
    if (! comment && commented)             /* For -dM option       */
        return;
    if (why)
        fprintf( fp, "%s \"%s\" defined as: ", why, dp->name);
    fprintf( fp, "%s#define %s", commented ? "/* " : "",
            dp->name);                      /* Macro name           */
    if (numargs >= 0) {                     /* Parameter list       */
#if MODE == STANDARD
        if (mode == STD) {
            fprintf( fp, "(%s)", dp->parmnames);
            if (! newdef) {
                for (i = 0, cp = dp->parmnames; i < numargs; i++, cp = cp1 + 1) {
                    if ((cp1 = strchr( cp, ',')) == NULL)   /* The last arg */
                        parlen[ i] = strlen( cp);
                    else
                        parlen[ i] = (size_t) (cp1 - cp);
                    parlist[ i] = cp;
                }
            }
        } else
#endif
        {
            if (newdef) {
                fprintf( fp, "(%s)", parlist[0]);
            } else if (numargs == 0) {
                fputs( "()", fp);
            } else {
                fputc( '(', fp);
                for (i = 0; i < numargs; i++) {     /* Make parameter list  */
                    fputc( 'a' + i % 26, fp);
                    if (i >= 26)
                        fputc( '0' + i / 26, fp);
                    if (i + 1 < numargs)
                        fputc( ',', fp);
                }
                fputc( ')', fp);
            }
        }
    }
    if (*dp->repl) {
        fputc( ' ', fp);
        dump_repl( dp, fp);                 /* Replacement text     */
    }
    if (commented)
            /* Standard predefined or one-pass-compiler-predefined  */
        fputs( " */", fp);
#if DEBUG
    if (comment)                            /* Not -dM option       */
        fprintf( fp, " \t/* %s%s:%ld\t*/", dp->dir, dp->fname, dp->mline);
#endif
    fputc( '\n', fp);
}

void
#if PROTO
dump_def( int dDflag, int comment)
#else
dump_def( dDflag)
    int     dDflag;                 /* -dD option (for GNU C)       */
    int     comment;        /* Location of definition in comment    */
#endif
/*
 * Dump all the current macro definitions to output stream.
 */
{
    register DEFBUF *   dp;
    DEFBUF **   syp;

    sharp();            /* Report the current source file & line    */
    if (comment)
        fputs( "/* Currently defined macros. */\n", fp_out);
    for (syp = symtab; syp < &symtab[ SBSIZE]; syp++) {
        if ((dp = *syp) != NULL) {
            do {
                dump_a_def( NULLST, dp, FALSE, dDflag, comment, fp_out);
            } while ((dp = dp->link) != NULL);
        }
    }
    wrong_line = TRUE;               /* Line number is out of sync   */
}

