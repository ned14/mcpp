/*-
 * Copyright (c) 1998, 2002-2006 Kiyoshi Matsui <kmatsui@t3.rim.or.jp>
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
 * MCPP Version 2.6
 * 2006/07      kmatsui
 *      Integrated STANDARD and PRE_STANDARD modes into one executable.
 */

/*
 * The routines to handle directives other than #include and #pragma
 * are placed here.
 */

#if PREPROCESSED
#include    "mcpp.H"
#else
#include    "system.H"
#include    "internal.H"
#endif

static int      do_if( int hash);
                /* #if, #elif, #ifdef, #ifndef      */
static long     do_line( void);
                /* Process #line directive          */
static int      get_parm( void);
                /* Get parm., its nargs, names, lens*/
static int      get_repl( const char * macroname);
                /* Get replacement embedded parm-no.*/
static char *   is_formal( const char * name, int conv);
                /* If formal param., save the num.  */
static char *   def_stringization( char * repl_cur);
                /* Define stringization     */
static char *   mgtoken_save( const char * macroname);
                /* Prefix DEF_MAGIC to macro name   */
static char *   str_parm_scan( char * string_end);
                /* Scan the param. in string        */
static void     do_undef( void);
                /* Process #undef directive         */
static void     dump_repl( const DEFBUF * dp, FILE * fp);
                /* Dump replacement text            */

/*
 * Generate (by hand-inspection) a set of unique values for each directive.
 * MCPP won't compile if there are hash conflicts.
 */

#define L_if            ('i' ^ (EOS << 1))
#define L_ifdef         ('i' ^ ('e' << 1))
#define L_ifndef        ('i' ^ ('d' << 1))
#define L_elif          ('e' ^ ('f' << 1))
#define L_else          ('e' ^ ('e' << 1))
#define L_endif         ('e' ^ ('i' << 1))
#define L_define        ('d' ^ ('i' << 1))
#define L_undef         ('u' ^ ('e' << 1))
#define L_line          ('l' ^ ('e' << 1))
#define L_include       ('i' ^ ('l' << 1))
#if COMPILER == GNUC
#define L_include_next  ('i' ^ ('l' << 1) ^ ('_' << 1))
#endif
#define L_error         ('e' ^ ('o' << 1))
#define L_pragma        ('p' ^ ('g' << 1))

static const char * const   not_ident
            = "Not an identifier \"%s\"";               /* _E_      */
static const char * const   no_arg = "No argument";     /* _E_      */
static const char * const   excess
            = "Excessive token sequence \"%s\"";        /* _E_ _W1_ */

int control( void)
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
    int     token_type;
    int     c;
    int     hash;
    char *  tp;

    in_directive = TRUE;
    if (keep_comments) {
        fputc( '\n', fp_out);       /* Possibly flush out comments  */
        newlines--;
    }
    c = skip_ws();
    if (c == '\n')                              /* 'null' directive */
        goto  ret;
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
    case L_elif:    tp = "elif";    break;
    case L_else:    tp = "else";    break;
    case L_endif:   tp = "endif";   break;
    case L_define:  tp = "define";  break;
    case L_undef:   tp = "undef";   break;
    case L_line:    tp = "line";    break;
    case L_include: tp = "include"; break;
#if COMPILER == GNUC
    case L_include_next:    tp = "include_next";    break;
#endif
    case L_error:   tp = "error";   break;
    case L_pragma:  tp = "pragma";  break;
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
        case L_elif :                       /*   if 0, compile.     */
            if (! standard) {
                if (warn_level & 8)
                    do_old();               /* Unknown directive    */
                goto  skip_line;            /* Skip the line        */
            }   /* Else fall through    */
        case L_endif:                       /* Un-nest #if          */
            break;
        case L_if   :                       /* These can't turn     */
        case L_ifdef:                       /*  compilation on, but */
        case L_ifndef:                      /*   we must nest #if's.*/
            if (&ifstack[ BLK_NEST] < ++ifptr)
                goto  if_nest_err;
            if (standard && (warn_level & 8)
                    && &ifstack[ blk_nest_min + 1] == ifptr)
                cwarn( many_nesting, NULLST, (long) blk_nest_min, in_skipped);
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
        if (standard && (warn_level & 4) &&
                &ifstack[ blk_nest_min + 1] == ifptr)
            cwarn( many_nesting, NULLST , (long) blk_nest_min, NULLST);
        ifptr->stat = WAS_COMPILING;
        ifptr->ifline = line;
        goto  ifdo;

    case L_elif:
        if (! standard) {
            do_old();               /* Unrecognized directive       */
            break;
        }
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
        c = do_if( hash);
        if (debug & IF) {
            fprintf( fp_debug
                    , "#if (#elif, #ifdef, #ifndef) evaluate to %s.\n"
                    , compiling ? "TRUE" : "FALSE");
            fprintf( fp_debug, "line %ld: %s", line, infile->buffer);
        }
        if (c == FALSE) {                   /* Error                */
            compiling = FALSE;              /* Skip this group      */
            goto  skip_line;    /* Prevent an extra error message   */
        }
        break;

    case L_else:
        if (ifptr == &ifstack[0])
            goto  nest_err;
        if (ifptr == infile->initif) {
            if (standard)
                goto  in_file_nest_err;
            else if (warn_level & 1)
                cwarn( not_in_section, NULLST, 0L, NULLST);
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
            if (standard)
                goto  in_file_nest_err;
            else if (warn_level & 1)
                cwarn( not_in_section, NULLST, 0L, NULLST);
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

    case L_error:
        if (! standard) {
            do_old();               /* Unrecognized directive       */
            break;
        }
        cerror( infile->buffer                              /* _E_  */
                , NULLST, 0L, NULLST);
        break;

    case L_pragma:
        if (! standard) {
            do_old();               /* Unrecognized directive       */
            break;
        }
        do_pragma();
        break;

    default:                /* Non-Standard or unknown directives   */
        do_old();
        break;
    }

    switch (hash) {
    case L_if       :
    case L_define   :
    case L_line     :
        goto  skip_line;    /* To prevent duplicate error message   */
#if COMPILER == GNUC
    case L_include_next  :
        newlines = -1;
#endif
    case L_error    :
        if (standard)
            goto  skip_line;
        /* Else fall through    */
    case L_include  :
    case L_pragma   :
        if (standard)
            break;          /* Already read over the line           */
        /* Else fall through    */
    default :               /* L_else, L_endif, L_undef, etc.       */
        if (mode == OLD_PREP) {
        /*
         * Ignore the rest of the #control line so you can write
         *          #if     foo
         *          #endif  foo
         */
            skip_nl();
            break;
        }
        if (skip_ws() != '\n') {
            if (standard)
                cerror( excess, infile->bptr-1, 0L, NULLST);
            else if (warn_level & 1)
                cwarn( excess, infile->bptr-1, 0L, NULLST);
            skip_nl();
        }
    }
    goto  ret;

in_file_nest_err:
    cerror( not_in_section, NULLST, 0L, NULLST);
    goto  skip_line;
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

static int  do_if( int hash)
/*
 * Process an #if (#elif), #ifdef or #ifndef.  The latter two are straight-
 * forward, while #if needs a subroutine of its own to evaluate the
 * expression.
 * do_if() is called only if compiling is TRUE.  If false, compilation is
 * always supressed, so we don't need to evaluate anything.  This supresses
 * unnecessary warnings.
 */
{
    int     c;
    int     found;

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

static long do_line( void)
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
    const char * const  out_of_range
        = "Line number \"%s\" is out of range of [1,%ld]";      /* _E_ _W1_ */
    int     token_type;
    VAL_SIGN *      valp;
    char *  save;
    int     c;

    if ((c = skip_ws()) == '\n') {
        cerror( no_arg, NULLST, 0L, NULLST);
        unget();                            /* Push back <newline>  */
        return  -1L;                /* Line number is not changed   */
    }

    if (standard) {
        token_type = get_unexpandable( c, FALSE);
        if (macro_line == MACRO_ERROR)      /* Unterminated macro   */
            return  -1L;                    /*   already diagnosed. */
        if (token_type == NO_TOKEN) /* Macro expanded to 0 token    */
            goto  no_num;
        if (token_type != NUM)
            goto  illeg_num;
    } else if (scan_token( c, (workp = work, &workp), work_end) != NUM) {
        goto  illeg_num;
    }
    for (workp = work; *workp != EOS; workp++) {
        if (! isdigit( *workp & UCHARMAX)) {
            if (standard) {
                cerror( not_digits, work, 0L, NULLST);
                return  -1L;
            } else if (warn_level & 1) {
                cwarn( not_digits, work, 0L, NULLST);
            }
        }
    }
    valp = eval_num( work);                 /* Evaluate number      */
    if (valp->sign == VAL_ERROR) {  /* Error diagnosed by eval_num()*/
        return  -1;
    } else if (standard && (line_limit < valp->val || valp->val <= 0L)) {
        if (valp->val < LINE99LIMIT && valp->val > 0L) {
            if (warn_level & 1)
                cwarn( out_of_range, work, line_limit, NULLST);
        } else {
            cerror( out_of_range, work, line_limit, NULLST);
            return  -1L;
        }
    }

    if (standard) {
        token_type = get_unexpandable( skip_ws(), FALSE);
        if (macro_line == MACRO_ERROR)
            return  -1L;
        if (token_type != STR) {
            if (token_type == NO_TOKEN) {   /* Filename is absent   */
                return  (long) valp->val;
            } else {    /* Expanded macro should be a quoted string */
                goto  not_fname;
            }
        }
    } else {
        if ((c = skip_ws()) == '\n') {
            unget();
            return  (long) valp->val;
        }
        if (scan_token( c, (workp = work, &workp), work_end) != STR)
            goto  not_fname;
    }
    *(workp - 1) = EOS;                     /* Ignore right '"'     */
    save = save_string( &work[ 1]);         /* Ignore left '"'      */

    if (standard) {
        if (get_unexpandable( skip_ws(), FALSE) != NO_TOKEN) {
            cerror( excess, work, 0L, NULLST);
            free( save);
            return  -1L;
        }
    } else if (mode == OLD_PREP) {
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

    infile->filename = save;                /* New file name        */
                                /* Do not free() infile->filename   */
    return  (long) valp->val;               /* New line number      */

no_num:
    cerror( "No line number", NULLST, 0L, NULLST);          /* _E_  */
    return  -1L;
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

DEFBUF *    do_define(
    int     ignore_redef        /* Do not redefine   */
)
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
 * modes other than STD ignore difference of parameter names in macro
 * redefinition.
 */
{
    const char * const  predef = "\"%s\" shouldn't be redefined";   /* _E_  */
    char    repl_list[ NMACWORK + IDMAX];   /* Replacement text     */
    char    macroname[ IDMAX + 1];  /* Name of the macro defining   */
    DEFBUF *    defp;               /* -> Old definition            */
    DEFBUF **   prevp;      /* -> Pointer to previous def in list   */
    int     c;
    int     redefined;                      /* TRUE if redefined    */
    int     dnargs;                         /* defp->nargs          */
    int     cmp;                    /* Result of name comparison    */

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
        if (standard) {
            if (cmp || defp->push) {    /* Not known or 'pushed' macro      */
                if (str_eq( identifier, "defined")
                        || ((stdc_val || cplus)
                            &&  str_eq( identifier, "__VA_ARGS__"))) {
                    cerror(
            "\"%s\" shouldn't be defined", identifier, 0L, NULLST); /* _E_  */
                    return  NULL;
                }
                redefined = FALSE;          /* Quite new definition */
            } else {                        /* It's known:          */
                if (ignore_redef)
                    return  defp;
                    dnargs = (defp->nargs == DEF_NOARGS-1) ? DEF_NOARGS
                            : defp->nargs;
                if (dnargs < DEF_NOARGS - 1 /* Standard predefined  */
                        || dnargs == DEF_PRAGMA /* _Pragma() pseudo-macro   */
                        ) {
                    cerror( predef, identifier, 0L, NULLST);
                    return  NULL;
                } else {
                    redefined = TRUE;       /* Remember this fact   */
                }
            }
        } else {
            if (cmp) {
                redefined = FALSE;          /* Quite new definition */
            } else {                        /* It's known:          */
                if (ignore_redef)
                    return  defp;
                dnargs = (defp->nargs == DEF_NOARGS-1) ? DEF_NOARGS
                        : defp->nargs;
                redefined = TRUE;
            }
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
                || (mode == STD && ! str_eq( defp->parmnames, work))
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
    if (mode == STD && cplus && id_operator( macroname) && (warn_level & 1))
        /* These are operators, not identifiers, in C++98   */
        cwarn( "\"%s\" is defined as macro", macroname      /* _W1_ */
                , 0L, NULLST);
    return  defp;
}

static int  get_parm( void)
/*
 *   Get parameters i.e. numbers into nargs, name into work[], name-length
 * into parlen[].
 *   Return TRUE if the parameters are legal, else return FALSE.
 *   In STD mode preprocessor must remember the parameter names, only for
 * checking the validity of macro redefinitions.  This is required by the
 * Standard (what an overhead !).
 */
{
    const char * const  many_parms
            = "More than %.0s%ld parameters";       /* _E_ _W4_     */
    const char * const  illeg_parm
            = "Illegal parameter \"%s\"";           /* _E_          */
    int     token_type;
    int     c;

    parlist[ 0] = workp = work;
    work[ 0] = EOS;

    /* POST_STD mode    */
    insert_sep = NO_SEP;    /* Clear the inserted token separator   */
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
                } else if (standard && (stdc_val || cplus)
                        && token_type == OPE && openum == OP_ELL) {
                    /*
                     * Enable variable argument macro which is a feature of
                     * C99.  We enable this even on C90 or C++ for GCC
                     * compatibility.
                     */
                    if (skip_ws() != ')') {
                        cerror( "\"...\" isn't the last parameter", /* _E_  */
                                NULLST, 0L, NULLST);
                        return  FALSE;
                    }
                    parlen[ nargs++] = 3;
                    nargs |= VA_ARGS;
                    goto  ret;
                } else {
                    cerror( illeg_parm , parlist[ nargs], 0L, NULLST);
                    return  FALSE;          /* Bad parameter syntax */
                }
            }
            if (standard && (stdc_val || cplus)
                    && str_eq( identifier, "__VA_ARGS__")) {
                cerror( illeg_parm, parlist[ nargs], 0L, NULLST);
                return  FALSE;
                /* __VA_ARGS__ should not be used as a parameter    */
            }
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
ret:
#if NMACPARS > NMACPARS90MIN
    if ((warn_level & 4) && (nargs & ~VA_ARGS) > n_mac_pars_min)
        cwarn( many_parms, NULLST , (long) n_mac_pars_min , NULLST);
#endif
    return  TRUE;
}

static int  get_repl( const char * macroname)
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
    const char * const  mixed_ops
    = "Macro with mixing of ## and # operators isn't portable";     /* _W4_ */
    const char * const  multiple_cats
    = "Macro with multiple ## operators isn't portable";    /* _W4_ */
    char *  prev_token = NULL;              /* Preceding token      */
    char *  prev_prev_token = NULL;         /* Pre-preceding token  */
    int     multi_cats = FALSE;             /* Multiple ## operators*/
    int     c;
    int     token_type;                     /* Type of token        */
    char *  temp;
    char *  repl_cur = repl_base;   /* Pointer into repl-text buffer*/

    *repl_cur = EOS;
    token_p = NULL;
    if (STD) {
        c = get();
        unget();
        if (((type[ c] & SPA) == 0) && (nargs < 0) && (warn_level & 1))
            cwarn( "No space between macro name \"%s\" and repl-text"/* _W1_ */
                , macroname, 0L, NULLST);
    }
    c = skip_ws();                           /* Get to the body      */

    while (c != CHAR_EOF && c != '\n') {
        if (standard) {
            prev_prev_token = prev_token;
            prev_token = token_p;
        }
        token_p = repl_cur;                 /* Remember the pointer */
        token_type = scan_token( c, &repl_cur, repl_end);

        switch (token_type) {
        case OPE:                   /* Operator or punctuator       */
            if (! standard)
                break;
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
                if (! standard)
                    break;
                if ((stdc_val || cplus)
                            && str_eq( identifier, "__VA_ARGS__")) {
                    cerror( "\"%s\" without corresponding \"...\""  /* _E_  */
                            , identifier, 0L, NULLST);
                    return  FALSE;
                }
                if ((temp = mgtoken_save( macroname)) != NULL)
                    repl_cur = temp;        /* Macro name           */
            } else {                        /* Parameter name       */
                repl_cur = temp;
            }
            break;

        case STR:                           /* String in mac. body  */
        case CHR:                           /* Character constant   */
            if (mode == OLD_PREP)
                repl_cur = str_parm_scan( repl_cur);
            break;
        case SEP:
            if (mode == OLD_PREP && c == COM_SEP)
                repl_cur--;                 /* Skip comment now     */
            break;
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
    if (standard) {
        if (token_p && *token_p == CAT) {
            cerror( "No token after ##", NULLST, 0L, NULLST);       /* _E_  */
            return  FALSE;
        }
        if (multi_cats && (warn_level & 4))
            cwarn( multiple_cats, NULLST, 0L, NULLST);
        if ((nargs & VA_ARGS) && stdc_ver < 199901L && (warn_level & 2))
            /* Variable arg macro is the spec of C99, not C90 nor C++98     */
            cwarn( "Variable argument macro is defined",    /* _W2_ */
                    NULLST, 0L, NULLST);
    }
    *repl_cur = EOS;                        /* Terminate work       */

    return  TRUE;
}

static char *   is_formal(
    const char *    name,
    int         conv                    /* Convert to magic number? */
)
/*
 * If the identifier is a formal parameter, save the MAC_PARM and formal
 * offset, returning the advanced pointer into the replacement text.
 * Else, return NULL.
 */
{
    char *  repl_cur;
    int     i;

    for (i = 0; i < (nargs & ~VA_ARGS); i++) {      /* For each parameter   */
        if ((strlen( name) == parlen[ i]
                            /* Note: parlist[] are comma separated  */
                    && memcmp( name, parlist[ i], parlen[ i]) == 0)
                || (standard && (nargs & VA_ARGS)
                    && i == (nargs & ~VA_ARGS) - 1 && conv
                    && memcmp( name, "__VA_ARGS__", 12) == 0)) {
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

static char *   def_stringization( char * repl_cur)
/*
 * Define token stringization.
 * We store a magic cookie (which becomes " on output) preceding the
 * parameter as an operand of # operator.
 * Return the current pointer into replacement text if the token following #
 * is a parameter name, else return NULL.
 */
{
    int     c;
    char *  temp;

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

static char *   mgtoken_save( const char * macroname)
/*
 * A magic cookie is inserted if the token is identical to the macro name,
 * so the expansion doesn't recurse.
 * Return the advanced pointer into the replacement text or NULL.
 */
{
    char *   repl_cur;

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

static char *   str_parm_scan( char * string_end)
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
    char *  wp;             /* Pointer into the quoted literal  */

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

static void do_undef( void)
/*
 * Remove the symbol from the defined list.
 * Called from control().
 */
{
    DEFBUF *    defp;
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
        } else if (standard 
                && (defp->nargs < DEF_NOARGS - 1        /* Standard predef  */
                    || defp->nargs == DEF_PRAGMA)) {
                                        /* _Pragma() pseudo-macro   */
            cerror( "\"%s\" shouldn't be undefined"         /* _E_  */
                    , identifier, 0L, NULLST);
        } else if (standard) {
            c = skip_ws();
            unget();
            if (c != '\n')                      /* Trailing junk    */
                return;
            else
                undefine( identifier);
        } else {
            undefine( identifier);
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
static long         num_of_macro;

DEFBUF *    look_id( const char * name)
/*
 * Look for the identifier in the symbol table.
 * If found, return the table pointer;  Else return NULL.
 */
{
    DEFBUF **   prevp;
    int         cmp;

    prevp = look_prev( name, &cmp);

    if (standard)
        return ((cmp == 0 && (*prevp)->push == 0) ? *prevp : (DEFBUF *)NULL);
    else
        return ((cmp == 0) ? *prevp : (DEFBUF *)NULL);
}

DEFBUF **   look_prev(
    const char *    name,                   /* Name of the macro    */
    int *   cmp                             /* Result of comparison */
)
/*
 * Look for the place to insert the macro definition.
 * Return a pointer to the previous member in the linked list.
 */
{
    const char *    np;
    DEFBUF **   prevp;
    DEFBUF *    dp;
    size_t      s_name;
    int         hash;

    for (hash = 0, np = name; *np != EOS; )
        hash += *np++;
    hash += s_name = (size_t)(np - name);
    s_name++;
    prevp = & symtab[ hash & SBMASK];
    *cmp = -1;                              /* Initialize           */

    while ((dp = *prevp) != NULL) {
        if ((*cmp = memcmp( dp->name, name, s_name)) >= 0)
            break;
        prevp = &dp->link;
    }

    return  prevp;
}

DEFBUF *    look_and_install(
    const char *    name,                   /* Name of the macro    */
    int     numargs,                        /* The numbers of parms */
    const char *    parmnames,  /* Names of parameters concatenated */
    const char *    repl                    /* Replacement text     */
)
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

DEFBUF *    install(
    const char *    name,                   /* Name of the macro    */
    int     numargs,                        /* The numbers of parms */
    const char *    parmnames,  /* Names of parameters concatenated */
    const char *    repl,                   /* Replacement text     */
    DEFBUF **  prevp,           /* The place to insert definition   */
    int     cmp         /* Result of comparison of new name and old */
)
/*
 * Enter this name in the lookup table.
 * Returns a pointer to the definition block.
 * Returns NULL if the symbol was Standard-predefined.
 */
{
    DEFBUF *    dp;
    DEFBUF *    defp;
    size_t      s_name, s_parmnames, s_repl;

    defp = *prevp;
    if (cmp == 0 && defp->nargs < DEF_NOARGS - 1)
        return  NULL;                       /* Standard predefined  */
    s_parmnames = 0;
    if (parmnames == NULL || repl == NULL)      /* Shouldn't happen */
        cfatal( "Bug: Illegal macro installation of \"%s\"" /* _F_  */
                , name, 0L, NULLST);    /* Use "" instead of NULL   */
    s_name = strlen( name);
    if (mode == STD)
        s_parmnames = strlen( parmnames) + 1;
    s_repl = strlen( repl) + 1;
    dp = (DEFBUF *)
        xmalloc( sizeof (DEFBUF) + s_name + s_parmnames + s_repl);
    if (cmp || (standard && (*prevp)->push)) {  /* New definition   */
        dp->link = defp;                /* Insert to linked list    */
        *prevp = dp;
    } else {                            /* Redefinition             */
        dp->link = defp->link;          /* Replace old def with new */
        *prevp = dp;
        free( defp);
    }
    dp->nargs = numargs;
    if (standard) {
        dp->push = 0;
        dp->parmnames = (char *)dp + sizeof (DEFBUF) + s_name;
        dp->repl = dp->parmnames + s_parmnames;
        if (mode == STD)
            memcpy( dp->parmnames, parmnames, s_parmnames);
    } else {
        dp->repl = (char *)dp + sizeof (DEFBUF) + s_name;
    }
    memcpy( dp->name, name, s_name + 1);
    memcpy( dp->repl, repl, s_repl);
    /* Remember where the macro is defined  */
    dp->dir = *inc_dirp;
    dp->fname = cur_fname;
    dp->mline = line;
    if (standard && cmp && ++num_of_macro == n_macro_min + 1 && n_macro_min
            && (warn_level & 4))
        cwarn( "More than %.0s%ld macros defined"           /* _W4_ */
                , NULLST , n_macro_min , NULLST);
    return  dp;
}

int undefine(
    const char *  name                      /* Name of the macro    */
)
/*
 * Delete the macro definition from the symbol table.
 * Returns TRUE, if deleted;
 * Else returns FALSE (when the macro was not defined or was predefined).
 */
{
    DEFBUF **   prevp;          /* Preceding definition in list     */
    DEFBUF *    dp;                     /* Definition to delete     */
    int         cmp;

    prevp = look_prev( name, &cmp);
    dp = *prevp;                        /* Definition to delete     */
    if (cmp || dp->nargs < DEF_NOARGS - 1)
        return  FALSE;                      /* Standard predefined  */
    if (standard && dp->push)
        return  FALSE;                  /* 'Pushed' macro           */
    *prevp = dp->link;          /* Link the previous and the next   */
    free( dp);                          /* Delete the definition    */
    if (standard)
        num_of_macro--;
    return  TRUE;
}

static void
dump_repl( const DEFBUF * dp, FILE * fp)
/*
 * Dump replacement text.
 */
{
    int     numargs = dp->nargs;
    char *  cp1;
    size_t  i;
    int     c;
    const char *    cp;

    for (cp = dp->repl; (c = *cp++ & UCHARMAX) != EOS; ) {

        switch (c) {
        case MAC_PARM:                              /* Parameter    */
            c = (*cp++ & UCHARMAX) - 1;
            if (standard) {
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
            } else {
                fputc( 'a' + c % 26, fp);
                if (c > 26)
                    fputc( '0' + c / 26, fp);
            }
            break;
        case DEF_MAGIC:
            if (! standard)
                fputc( c, fp);
            /* Else skip    */
            break;
        case CAT:
            if (standard)
                fputs( "##", fp);
            else
                fputc( c, fp);
            break;
        case ST_QUOTE:
            if (standard)
                fputs( "#", fp);
            else
                fputc( c, fp);
            break;
        case COM_SEP:
            /*
             * Though TOK_SEP coincides to COM_SEP, this cannot appear in
             * Standard mode.
             */
            if (mode == OLD_PREP)
                fputs( "/**/", fp);
            break;
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

void    dump_a_def(
    const char *    why,
    const DEFBUF *  dp,
    int     newdef,         /* TRUE if parmnames are currently in parlist[] */
    int     dDflag,         /* TRUE if -dD option is used (for GCC)         */
    int     comment,        /* Show location of the definition in comment   */
    FILE *  fp
)
/*
 * Dump a macro definition.
 */
{
    char *  cp, * cp1;
    int     numargs = dp->nargs & ~VA_ARGS;
    int     commented;                      /* To be commented out  */
    int     i;

    if (standard && numargs == DEF_PRAGMA)  /* _Pragma pseudo-macro */
        return;
    if ((numargs < CAN_REDEF) || (dDflag && (numargs == DEF_NOARGS - 1))
            || (standard && dp->push))
        commented = TRUE;
    else
        commented = FALSE;
    if (! comment && commented)             /* For -dM option       */
        return;
    if (why)
        fprintf( fp, "%s \"%s\" defined as: ", why, dp->name);
    fprintf( fp, "%s#define %s", commented ? "/* " : "",
            dp->name);                      /* Macro name           */
    if (numargs >= 0) {                     /* Parameter list       */
        if (mode == STD) {
            fprintf( fp, "(%s)", dp->parmnames);
            if (! newdef) {
                for (i = 0, cp = dp->parmnames; i < numargs;
                        i++, cp = cp1 + 1) {
                    if ((cp1 = strchr( cp, ',')) == NULL)   /* The last arg */
                        parlen[ i] = strlen( cp);
                    else
                        parlen[ i] = (size_t) (cp1 - cp);
                    parlist[ i] = cp;
                }
            }
        } else {
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
    if (comment)                            /* Not -dM option       */
        fprintf( fp, " \t/* %s%s:%ld\t*/", dp->dir, dp->fname, dp->mline);
    fputc( '\n', fp);
}

void    dump_def(
    int     dDflag,                 /* -dD option (for GCC)         */
    int     comment         /* Location of definition in comment    */
)
/*
 * Dump all the current macro definitions to output stream.
 */
{
    DEFBUF *    dp;
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

