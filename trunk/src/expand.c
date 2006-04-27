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
 *                          E X P A N D . C
 *                  M a c r o   E x p a n s i o n
 */

/*
 * CPP Version 2.0 / expand.c
 * 1998/08      kmatsui
 *      Split from cpp4.c and cpp6.c (support.c).
 *      Created Standard conforming mode of macro expansion.
 *      Created is_macro(), squeeze_ws(), skip_macro()
 *              as common routines,
 *          expand(), replace(), def_special(), prescan(), catenate(),
 *          stringize(), substitute(), rescan(), disable_repl(),
 *          enable_repl(), is_able_repl()
 *              for MODE >= STANDARD,
 *          expand(), conv_st_quote()
 *              for MODE < STANDARD.
 *      Split expcollect() into collect_args() and get_an_arg().
 *      Renamed macroid() to rescan(), expand() to replace(), expstuff() to
 *          substitute()    for MODE < STANDARD.
 *          dumpparm() to dump_args().
 *      Revised most of the functions.
 */

/*
 * CPP Version 2.2 / expand.c
 * 1998/11      kmatsui
 *      Revised several functions.
 */

/*
 * CPP Version 2.3 pre-release 1 / expand.c
 * 2002/08      kmatsui
 *      Fixed the bug of 0-parameter function-like macro.
 *      Modified re-examination of macro of the same name.
 *      Renamed several functions using underscore.
 *
 * CPP Version 2.3 pre-release 2 / expand.c
 * 2002/12      kmatsui
 *      Fixed the bug of rescan() warning innocent macro call.
 */

/*
 * MCPP Version 2.4 prerelease
 * 2003/11      kmatsui
 *      Revised def_special() and collect_args() according to the change of
 *          FILEINFO.
 *      Removed CON_NOEXPAND and CON_EXPAND modes of macro expansion, removed
 *          append_string(), catenate() and conv_st_quote() in pre-STANDARD
 *          modes accordingly.
 *
 * MCPP Version 2.4.1
 * 2004/03      kmatsui
 *      Added compat_mode (compatible mode to GNU C's expansion of recursive
 *          macro).
 *
 * MCPP Version 2.5
 * 2005/03      kmatsui
 *      Revised handling of token separator in STANDARD mode.
 *      Revised "blue painting" of the same name macro.
 *      Revised debugging information.
 *      Split is_macro_call() from is_macro().
 *      Absorbed POST_STANDARD into STANDARD and OLD_PREPROCESSOR into
 *          PRE_STANDARD.
 */

/*
 * The macro expansion routines are placed here.
 */

#if PREPROCESSED
#include    "mcpp.H"
#else
#include    "system.H"
#include    "internal.H"
#endif

#define ARG_ERROR   (-255)

#if PROTO

static DEFBUF * is_macro_call( DEFBUF * defp, char ** cp);
static int      collect_args( const DEFBUF * defp, char ** arglist);
static int      get_an_arg( int c, char ** argpp, char * arg_end
        , char ** seqp, int var_arg);
static int      squeeze_ws( char ** out);
static void     skip_macro( void);
#if DEBUG
static void     dump_args( const char * why, int nargs, char ** arglist);
#endif

#else   /* ! PROTO  */

static DEFBUF * is_macro_call();    /* Is this really a macro call ?*/
static int      collect_args(); /* Collect arguments of a macro call*/
static int      get_an_arg();   /* Get an argument          */
static int      squeeze_ws();   /* Squeeze white spaces to a space  */
static void     skip_macro();   /* Skip the rest of macro call      */
#if DEBUG
static void     dump_args();    /* Dump arguments list      */
#endif

#endif  /* ! PROTO  */

static int      rescan_level;       /* Times of macro rescan    */
static const char * const   macbuf_overflow
        = "Buffer overflow expanding macro \"%s\" at %.0ld\"%s\"";  /* _E_  */
static const char * const   empty_arg
        = "Empty argument in macro call \"%s\"";            /* _W2_ */
static const char * const   unterm_macro
        = "Unterminated macro call \"%s\"";                 /* _E_  */
static const char * const   narg_error
    = "%s than necessary %ld argument(s) in macro call \"%s\""; /* _E_ _W1_ */
#if MODE == PRE_STANDARD
static const char * const   only_name
        = "Macro \"%s\" needs arguments";                   /* _W8_ */
#endif

DEFBUF *
#if PROTO
is_macro( char ** cp)
#else
is_macro( cp)
    char **     cp;
#endif
/*
 * The name is already in 'identifier', the next token is not yet read.
 * Return TRUE if the name is a macro call, else return FALSE.
 */
{
    DEFBUF *    defp;

    if ((defp = look_id( identifier)) != NULL)  /* Is a macro name  */
        return  is_macro_call( defp, cp);
    else
        return  NULL;
}

static DEFBUF *
#if PROTO
is_macro_call( DEFBUF * defp, char ** cp)
#else
is_macro_call( defp, cp)
    DEFBUF *    defp;
    char **     cp;
#endif
/*
 * Return TRUE if the defp->name is a macro call, else return FALSE.
 */
{
    int    c;

    if (defp->nargs >= 0                    /* Function-like macro  */
#if MODE == STANDARD && OK_PRAGMA_OP
            || defp->nargs == DEF_PRAGMA    /* _Pragma() pseudo-macro       */
#endif
            ) {
        c = squeeze_ws( cp);                /* See the next char.   */
        if (c == CHAR_EOF) {                /* End of file          */
            unget_string( "\n", NULLST);    /* Restore skipped '\n' */
        } else {                            /* Still in the file    */
#if MODE == STANDARD
            if (c != RT_END)                /* Rescan boundary ?    */
#endif
                unget();                    /* To see it again      */
        }
        if (c != '(') {     /* Only the name of function-like macro */
#if MODE == PRE_STANDARD
            if (warn_level & 8)
                cwarn( only_name, defp->name, 0L, NULLST);
#endif
            return  NULL;
        }
    }
    return  defp;                           /* Really a macro call  */
}

/*
 * expand()     expands a macro call completely, and writes out the result to
 *      the specified output buffer and returns the advanced pointer.
 */


#if     MODE == STANDARD

/*
 *          T h e   S T A N D A R D   C o n f o r m i n g   M o d e
 *                  o f   M a c r o   E x p a n s i o n
 *
 * 1998/08      First released.     kmatsui
 */

/* Return value of is_able_repl()   */
#define NO          0               /* "Blue-painted"               */
#define YES         1               /* Not blue-painted             */
#define READ_OVER   2
            /* Still "blue-painted", yet has read over repl-list    */

static char *       macro_name;     /* Name of the macro to expand  */
static struct {
    const DEFBUF *  def;            /* Macro definition             */
    int             read_over;      /* Has read over repl-list      */
    /* 'read_over' is never used in POST_STD mode and in compat_mode*/
} replacing[ RESCAN_LIMIT];         /* Macros currently replacing   */

#if PROTO

static char *   replace( DEFBUF * defp, char * out, char * out_end
        , const DEFBUF * outer, FILEINFO * rt_file);
static DEFBUF * def_special( DEFBUF * defp);
static int      prescan( const DEFBUF * defp, char ** arglist, char * out
        , char * out_end);
static char *   catenate( const DEFBUF * defp, char ** arglist, char * out
        , char * out_end, char ** token_p);
static char *   stringize( const DEFBUF * defp, char * argp, char * out);
static char *   substitute( char ** arglist, const char * in, char * out
        , char * out_end);
static char *   rescan( const DEFBUF * outer, const char * in, char * out
        , char * out_end);
static int      disable_repl( const DEFBUF * defp);
static void     enable_repl( const DEFBUF * defp, int done);
static int      is_able_repl( const DEFBUF * defp);

#else   /* ! PROTO  */

static char *   replace();          /* Replace a macro recursively      */
static DEFBUF * def_special();      /* Re-define __LINE__, __FILE__     */
static int      prescan();          /* Process #, ## operator           */
static char *   catenate();         /* Catenate tokens                  */
static char *   stringize();        /* Stringize an argument            */
static char *   substitute();       /* Substitute parms with arguments  */
static char *   rescan();           /* Rescan once replaced sequences   */
static int      disable_repl();     /* Disable the macro once replaced  */
static void     enable_repl();      /* Enable the macro for later use   */
static int      is_able_repl();     /* Is the macro allowed to replace? */

#endif  /* ! PROTO  */

char *
#if PROTO
expand( DEFBUF * defp, char * out, char * out_end)
#else
expand( defp, out, out_end)
    DEFBUF *    defp;                       /* Macro definition     */
    char *  out;                            /* Output buffer        */
    char *  out_end;                        /* End of output buffer */
#endif
/*
 * Expand a macro call completely, write the results to the specified buffer
 * and return the advanced pointer.
 */
{
    char    macrobuf[ NMACWORK + IDMAX];    /* Buffer for replace() */
    char *  out_p = out;
    size_t  len;
    int     c, c1;
    register char *     cp;

    macro_line = line;                      /* Line number for diag */
    macro_name = save_string( identifier);
    rescan_level = 0;
    if (replace( defp, macrobuf, macrobuf + NMACWORK, (DEFBUF *) NULLST
            , infile) == NULL) {            /* Illegal macro call   */
        skip_macro();
        macro_line = MACRO_ERROR;
        goto  exp_end;
    }
    len = (size_t) (out_end - out);
    if (strlen( macrobuf) > len) {
        cerror( macbuf_overflow, defp->name, 0, macrobuf);
        memcpy( out, macrobuf, len);
        out_p = out + len;
        macro_line = MACRO_ERROR;
        goto  exp_end;
    }

    cp = macrobuf;
    c1 = '\0';
    while ((c = *cp++) != EOS) {
        if (c == DEF_MAGIC)
            continue;                       /* Skip DEF_MAGIC       */
        if (mode == STD) {
            if (c == IN_SRC)
                continue;                   /* Skip IN_SRC          */
            else if (c == TOK_SEP) {
                if (c1 == ' ')
                    continue;
                c = ' ';
            }
        }
        c1 = c;
        *out_p++ = c;
    }

    macro_line = 0;
exp_end:
    *out_p = EOS;
#if DEBUG
    if (debug & EXPAND)
        dump_string( "expand exit", out);
#endif
    free( macro_name);

    return  out_p;
}

static char *
#if PROTO
replace( DEFBUF * defp, char * out, char * out_end, const DEFBUF * outer
        , FILEINFO * rt_file)
#else
replace( defp, out, out_end)
    register DEFBUF *   defp;               /* Macro to be replaced */
    char *  out;                            /* Output Buffer        */
    char *  out_end;                        /* End of output buffer */
    DEFBUF *    outer;                      /* Outer macro replacing*/
    FILEINFO *  rt_file;                    /* Repl-text "file"     */
#endif
/*
 * Replace a possibly nested macro recursively.
 * replace() and rescan() call each other recursively.
 * Return the advanced output pointer or NULL on error.
 */
{
    char ** arglist = NULL;                 /* Pointers to arguments*/
    int     nargs;                  /* Number of arguments expected */
    char *  catbuf;                         /* Buffer for prescan() */
    char *  expbuf;                 /* Buffer for  substitute()     */
    char *  out_p;                          /* Output pointer       */

#if DEBUG
    if (debug & EXPAND) {
        dump_a_def( "replace entry", defp, FALSE, FALSE, TRUE, fp_debug);
        dump_unget( "replace entry");
    }
#endif
    nargs = (defp->nargs == DEF_PRAGMA) ? 1 : (defp->nargs & ~VA_ARGS);

    if (nargs < DEF_NOARGS - 2) {           /* __FILE__, __LINE__   */
        defp = def_special( defp);
        if (mode == STD) {
            *out++ = TOK_SEP;       /* Wrap repl-text with token    */
            out = stpcpy( out, defp->repl); /*   separators to pre- */
            *out++ = TOK_SEP;               /*   vent token merging.*/
            *out = EOS;
        } else {
            out = stpcpy( out, defp->repl);
        }
        return  out;
    } else if (nargs >= 0) {                /* Function-like macro  */
        squeeze_ws( (char **)NULL);         /* Skip to '('          */
        arglist = (char **) xmalloc( (nargs + 1) * sizeof (char *));
        arglist[ 0] = xmalloc( (size_t) (NMACWORK + IDMAX * 2));
                            /* Note: arglist[ n] may be reallocated */
                            /*   and re-written by collect_args()   */
        if (collect_args( defp, arglist) == ARG_ERROR) {
            free( arglist[ 0]);             /* Syntax error         */
            free( arglist);
            return  NULL;
        }
        if (mode == STD && outer && rt_file != infile) {
                                 /* Has read over replacement-text  */
            if (compat_mode) {
                enable_repl( outer, FALSE); /* Enable re-expansion  */
#if DEBUG
                if (debug & EXPAND)
                    dump_string( "enabled re-expansion"
                            , outer ? outer->name : "<arg>");
#endif
            } else {
                replacing[ rescan_level-1].read_over = READ_OVER;
            }
        }
    }

    catbuf = xmalloc( (size_t) (NMACWORK + IDMAX));
#if DEBUG
    if (debug & EXPAND) {
        fprintf( fp_debug, "(%s)", defp->name);
        dump_string( "prescan entry", defp->repl);
    }
#endif
    if (prescan( defp, arglist, catbuf, catbuf + NMACWORK) == FALSE) {
                                    /* Process #, ## operators      */
        cerror( macbuf_overflow, defp->name, 0L, catbuf);
        if (nargs >= 0) {
            free( arglist[ 0]);
            free( arglist);
        }
        free( catbuf);
        return  NULL;
    }
    catbuf = xrealloc( catbuf, strlen( catbuf) + 1);
                                            /* Use memory sparingly */
#if DEBUG
    if (debug & EXPAND) {
        fprintf( fp_debug, "(%s)", defp->name);
        dump_string( "prescan exit", catbuf);
    }
#endif

    if (nargs > 0) {    /* Function-like macro with any argument    */
        expbuf = xmalloc( (size_t) (NMACWORK + IDMAX));
#if DEBUG
        if (debug & EXPAND) {
            fprintf( fp_debug, "(%s)", defp->name);
            dump_string( "substitute entry", catbuf);
        }
#endif
        out_p = substitute( arglist, catbuf, expbuf, expbuf + NMACWORK);
                                            /* Expand each arguments*/
        free( arglist[ 0]);
        free( arglist);
        free( catbuf);
        expbuf = xrealloc( expbuf, strlen( expbuf) + 1);
                                            /* Use memory sparingly */
#if DEBUG
        if (debug & EXPAND) {
            fprintf( fp_debug, "(%s)", defp->name);
            dump_string( "substitute exit", expbuf);
        }
#endif
    } else {                                /* Object-like macro or */
        if (nargs == 0) {   /* Function-like macro with no argument */
            free( arglist[ 0]);
            free( arglist);
        }
        out_p = expbuf = catbuf;
    }

    if (out_p)
        out_p = rescan( defp, expbuf, out, out_end);
#if OK_PRAGMA_OP
    if (out_p && defp->nargs == DEF_PRAGMA)
        has_pragma = TRUE;
        /* Inform mcpp_main() that _Pragma() was found  */
#endif
    free( expbuf);
    return  out_p;
}

static DEFBUF *
#if PROTO
def_special( DEFBUF * defp)
#else
def_special( defp)
    register DEFBUF *   defp;               /* Macro definition     */
#endif
/*
 * Re-define __LINE__, __FILE__.
 * Return the new definition.
 */
{
    const FILEINFO *    file;
    DEFBUF **   prevp;
    int         cmp;

    switch (defp->nargs) {
    case DEF_NOARGS - 3:                    /* __LINE__             */
        if ((line > line_limit || line <= 0) && (warn_level & 1))
            cwarn( "Line number %.0s\"%ld\" is out of range"        /* _W1_ */
                    , NULLST, line, NULLST);
        sprintf( defp->repl, "%ld", line);          /* Re-define    */
        break;
    case DEF_NOARGS - 4:                    /* __FILE__             */
        for (file = infile; file != NULL; file = file->parent) {
            if (file->fp != NULL) {
                sprintf( work, "\"%s%s\"", *(file->dirp), file->filename);
                if (str_eq( work, defp->repl))
                    break;                          /* No change    */
                defp->nargs = DEF_NOARGS;   /* Enable to redefine   */
                prevp = look_prev( defp->name, &cmp);
                defp = install( "__FILE__", DEF_NOARGS - 4, "", work, prevp
                        , cmp);             /* Re-define            */
                break;     
            }
        }
        break;
    }
    return  defp;
}

static int
#if PROTO
prescan( const DEFBUF * defp, char ** arglist, char * out, char * out_end)
#else
prescan( defp, arglist, out, out_end)
    DEFBUF *    defp;               /* Definition of the macro      */
    char **     arglist;            /* Pointers to actual arguments */
    char *      out;                /* Output buffer                */
    char *      out_end;            /* End of output buffer         */
#endif
/*
 * Concatenate the tokens surounding ## by catenate(), and stringize the
 * argument following # by stringize().
 */
{
    FILEINFO *  file;
    char *      prev_token;         /* Preceding token              */
    register int    c;              /* Value of a character         */
    /*
     * The replacement lists are --
     *          stuff1<SEP>stuff2
     *      or  stuff1<SEP>stuff2<SEP>stuff3...
     * where <SEP> is CAT, maybe with preceding space and following space,
     * stuff might be
     *          ordinary-token
     *          MAC_PARM<n>
     *      or  <QUO>MAC_PARM<n>
     * where <QUO> is ST_QUO, possibly with following space.
     */

    if (mode == POST_STD) {
        file = unget_string( defp->repl, defp->name);
    } else {
        *out++ = TOK_SEP;                       /* Wrap replacement     */
        workp = work;                           /*  text with token     */
        workp = stpcpy( workp, defp->repl);     /*   separators to      */
        *workp++ = TOK_SEP;                     /*    prevent unintended*/
        *workp = EOS;                           /*     token merging.   */
        file = unget_string( work, defp->name);
    }

    while (c = get(), file == infile) {     /* To the end of repl   */

        switch (c) {
        case ST_QUOTE:
            skip_ws();                      /* Skip space, MAC_PARM */
            c = get() - 1;                  /* Parameter number     */
            prev_token = out;               /* Remember the token   */
            out = stringize( defp, arglist[ c], out);
                                    /* Stringize without expansion  */
            break;
        case CAT:
            if (*prev_token == DEF_MAGIC)
                *prev_token++ = ' ';        /* Remove DEF_MAGIC     */
            else if (*prev_token == IN_SRC)
                *prev_token++ = ' ';        /* Remove IN_SRC        */
            out = catenate( defp, arglist, out, out_end, &prev_token);
            break;
        case MAC_PARM:
            prev_token = out;
            *out++ = MAC_PARM;
            *out++ = get();                 /* Parameter number     */
            break;
        case TOK_SEP:
        /* Fall through */
        case ' ':
            *out++ = c;
            break;
        default:
            prev_token = out;
            scan_token( c, &out, out_end);  /* Ordinary token       */
            break;
        }

        *out = EOS;                         /* Ensure termination   */
        if (out_end <= out)                 /* Buffer overflow      */
            return  FALSE;
    }

    *out = EOS;         /* Ensure terminatation in case of no token */
    unget();
    return  TRUE;
}

static char *
#if PROTO
catenate( const DEFBUF * defp, char ** arglist, char * out, char * out_end
         , char ** token_p)
#else
catenate( defp, arglist, out, out_end, token_p)
    DEFBUF *    defp;               /* The macro definition         */
    char ** arglist;                /* Pointers to actual arguments */
    char *  out;                    /* Output buffer                */
    char *  out_end;                /* End of output buffer         */
    char ** token_p;        /* Address of preceding token pointer   */
#endif
/*
 * Concatenate the previous and the following tokens.
 *   Note: The parameter codes may coincide with white spaces or any
 * other characters.
 */
{
    FILEINFO *      file;
    char *  prev_prev_token;
    const char *    invalid_token
    = "Not a valid preprocessing token \"%s\"";     /* _E_ _W2_     */
    const char *    argp;           /* Pointer to an actual argument*/
    char *  prev_token = *token_p;  /* Preceding token              */
    int     in_arg = FALSE;
    register int    c;              /* Value of a character         */

    while ((*(out - 1) == ' ' || *(out - 1) == TOK_SEP)
            && (*(out - 2) != MAC_PARM || prev_token != out - 2))
        out--;          /* Remove spaces and inserted separators    */
    *out = EOS;

    /* Get the previous token   */
    if (*prev_token == MAC_PARM) {          /* Formal parameter     */
        c = (*(prev_token + 1) & UCHARMAX) - 1;     /* Parm number  */
        argp = arglist[ c];                 /* Actual argument      */
        out = prev_token;                   /* To overwrite         */
        if ((mode == POST_STD && *argp == EOS)
                || (mode == STD && *argp == RT_END)) {
            *out = EOS;                     /* An empty argument    */
        } else {
            if (mode == POST_STD) {
                file = unget_string( argp, NULLST);
                while (c = get(), file == infile) {
                    prev_token = out;   /* Remember the last token      */
                    scan_token( c, &out, out_end);
                }           /* Copy actual argument without expansion   */
                unget();
            } else {
                unget_string( argp, NULLST);
                while ((c = get()) != RT_END) {
                    prev_prev_token = prev_token;
                    prev_token = out;   /* Remember the last token      */
                    scan_token( c, &out, out_end);
                }           /* Copy actual argument without expansion   */
                if (*prev_token == TOK_SEP) {
                    out = prev_token;
                    prev_token = prev_prev_token;   /* Skip separator   */
                }
            }
            if (*prev_token == DEF_MAGIC 
                    || (mode == STD && *prev_token == IN_SRC)) {
                memmove( prev_token, prev_token + 1
                        , (size_t) (out-- - prev_token));
            /* Remove DEF_MAGIC enabling the name to replace later  */
            }
        }
    }
    c = skip_ws();

    /* Catenate */
    switch (c) {
    case ST_QUOTE:          /* First stringize and then catenate    */
        skip_ws();                  /* Skip MAC_PARM, ST_QUOTE      */
        c = get() - 1;
        out = stringize( defp, arglist[ c], out);
        break;
    case MAC_PARM:
        c = get() - 1;                      /* Parameter number     */
        argp = arglist[ c];                 /* Actual argument      */
        if ((mode == POST_STD && *argp == EOS)
                || (mode == STD && *argp == RT_END))
            *out = EOS;                     /* An empty argument    */
        else {
            unget_string( argp, NULLST);
            if ((c = get()) == DEF_MAGIC)   /* Remove DEF_MAGIC     */
                c = get();                  /*  enabling to replace */
            else if (c == IN_SRC)           /* Remove IN_SRC        */
                c = get();
            scan_token( c, &out, out_end);  /* The first token      */
            if (*infile->bptr)              /* There are more tokens*/
                in_arg = TRUE;
        }
        break;
    case DEF_MAGIC:
    case IN_SRC:
        c = get();                      /* Skip DEF_MAGIC, IN_SRC   */
        /* Fall through */
    default:
        scan_token( c, &out, out_end);      /* Copy the token       */
        break;
    }

    /* The generated sequence is a valid preprocessing-token ?      */
    if (*prev_token) {                      /* There is any token   */
        unget_string( prev_token, NULLST);  /* Scan once more       */
        c = get();  /* This line should be before the next line.    */
        infile->fp = (FILE *)-1;            /* To check token length*/
#if DEBUG
        if (debug & EXPAND)
            dump_string( "checking generated token", infile->buffer);
#endif
        scan_token( c, (workp = work, &workp), work_end);
        infile->fp = NULL;
        if (*infile->bptr != EOS) {         /* More than a token    */
            if (lang_asm || compat_mode) {
                                /* Assembler source or compat_mode  */
                if (warn_level & 2)
                    cwarn( invalid_token, prev_token, 0L, NULLST);
            } else {
                cerror( invalid_token, prev_token, 0L, NULLST);
            }
            infile->bptr += strlen( infile->bptr);
        }
        get();                              /* To the parent "file" */
        unget();
    }

    if (mode == STD) {
        *out++ = TOK_SEP;                   /* Prevent token merging*/
        *out = EOS;
    }
    if (in_arg) {
        if (mode == POST_STD) {
            file = infile;
            while (c = get(), file == infile) {
                prev_token = out;       /* Remember the last token      */
                scan_token( c, &out, out_end);
            }            /* Copy rest of argument without expansion     */
            unget();
        } else {
            while ((c = get()) != RT_END) {
                prev_token = out;       /* Remember the last token      */
                scan_token( c, &out, out_end);
            }           /* Copy rest of argument without expansion      */
        }
    }
    *token_p = prev_token;      /* Report back the generated token  */

    return  out;
}

static char *
#if PROTO
stringize( const DEFBUF * defp, char * argp, char * out)
#else
stringize( defp, argp, out)
    DEFBUF *    defp;                       /* The macro definition */
    char *      argp;                       /* Pointer to argument  */
    char *      out;                        /* Output buffer        */
#endif
/*
 * Make a string literal from an argument.
 */
{
    FILEINFO *      file;
    int             stray_bsl = FALSE;      /* '\\' not in literal  */
    char *          out_p = out;
    int             token_type;
    register int    c;

    *out_p++ = '"';

    file = unget_string( argp, NULLST);

    while ((c = get()), ((mode == POST_STD && file == infile)
            || (mode == STD && c != RT_END))) {
        if (c == ' ') {
            *out_p++ = ' ';
            continue;
        }
        if (mode == STD) {
            if (c == TOK_SEP)
                continue;                   /* Skip inserted separ  */
            else if (c == IN_SRC)
                continue;                   /* Skip in-src magic    */
        }
        if (c == '\\')
            stray_bsl = TRUE;               /* May cause a trouble  */
        token_type = scan_token( c, (workp = work, &workp), work_end);

        switch (token_type) {
        case WSTR:
        case WCHR:
        case STR:
        case CHR:
            workp = work;
            while ((c = *workp++ & UCHARMAX) != EOS) {
#if BSL_IN_MBCHAR
                if (type[ c] & mbstart) {   /* Multi-byte character */
                    mb_read( c, &workp, (*out_p++ = c, &out_p));
                                            /* Copy as it is        */
                    continue;
                }
#endif
                if (c == '"') {
                    *out_p++ = '\\';        /* Insert '\\'          */
                } else if (c == '\\') {
#if OK_UCN
                    if (mode == POST_STD || ! stdc3
                            || (*workp != 'u' && *workp != 'U'))
                                            /* Not UCN              */
#endif
                        *out_p++ = '\\';
                }
                *out_p++ = c;
            }
            *out_p = EOS;
            break;
        default:
            out_p = stpcpy( out_p, work);
            break;
        }
    }

    if (mode == POST_STD)
        unget();
    *out_p++ = '"';
    *out_p = EOS;

    if (stray_bsl) {
        int     invalid = FALSE;
        unget_string( out, defp->name);
#if DEBUG
        if (debug & EXPAND)
            dump_string( "checking generated token", infile->buffer);
#endif
        scan_quote( get(), work, work_end, TRUE);
            /* Unterminated or too long string will be diagnosed    */
        if (*infile->bptr != EOS)           /* More than a token    */
            invalid = TRUE; /* Diagnose after clearing the "file"   */
        infile->bptr += strlen( infile->bptr);
        get();                              /* Clear the "file"     */
        unget();
        if (invalid)
            cerror( "Not a valid string literal %s"         /* _E_  */
                    , out, 0L, NULLST);
    }
#if NWORK-2 > SLEN90MIN
    else if ((warn_level & 4) && out_p - out > str_len_min)
        cwarn( "String literal longer than %.0s%ld bytes %s"        /* _W4_ */
                , NULLST , (long) str_len_min, out);
#endif
    return  out_p;
}

static char *
#if PROTO
substitute( char ** arglist, const char * in, char * out, char * out_end)
#else
substitute( arglist, in, out, out_end)
    char **     arglist;        /* Pointers to actual arguments     */
    char *      in;                         /* Replacement text     */
    char *      out;                        /* Output buffer        */
    char *      out_end;                    /* End of output buffer */
#endif
/*
 * Replace completely each actual arguments of the macro, and substitute for
 * the formal parameters in the replacement list.
 */
{
    register int    c;

    *out = EOS;                             /* Ensure to termanate  */
    while ((c = *in++) != EOS) {
        if (c == MAC_PARM) {                /* Formal parameter     */
            c = *in++ & UCHARMAX;           /* Parameter number     */
#if DEBUG
            if (debug & EXPAND) {
                fprintf( fp_debug, " (expanding arg[%d])", c);
                dump_string( NULLST, arglist[ c - 1]);
            }
#endif
            if ((out = rescan( (DEFBUF *)NULL, arglist[ c - 1], out, out_end))
                    == NULL)                /* Replace completely   */
                return  NULL;               /* Error                */
        } else {
            *out++ = c;                     /* Copy the character   */
        }
    }
    *out = EOS;
    return  out;
}

static char *
#if PROTO
rescan( const DEFBUF * outer, const char * in, char * out, char * out_end)
#else
rescan( outer, in, out, out_end)
    DEFBUF *    outer;              /* Outer macro just replacing   */
    char *  in;                     /* Sequences to be rescanned    */
    char *  out;                            /* Output buffer        */
    char *  out_end;                        /* End of output buffer */
#endif
/*
 * Re-scan the once replaced sequences to replace the remaining macros
 * completely.
 * rescan() and replace() call each other recursively.
 *
 * Note: POST_STD mode does not use IN_SRC nor TOK_SEP and seldom uses RT_END.
 * Checking of those are unnecessary overhead for POST_STD mode.  To integrate
 * the code for POST_STD with STD mode, however, we use these checkings
 * commonly.
 * compat_mode also does not use IN_SRC.
 */
{
    char *  cur_cp;
    char *  tp = NULL;              /* Temporary pointer into buffer*/
    char *  out_p = out;            /* Current output pointer       */
    FILEINFO *  file;       /* Input sequences stacked on a "file"  */
    DEFBUF *    inner;              /* Inner macro to replace       */
    int     is_able;                /* Macro is not "blue-painted"  */
    register int    c;              /* First character of token     */

#if DEBUG
    if (debug & EXPAND) {
        fprintf( fp_debug, "rescan_level--%d (%s) "
                , rescan_level + 1, outer ? outer->name : "<arg>");
        dump_string( "rescan entry", in);
    }
#endif
    if (! disable_repl( outer)) /* Don't re-replace replacing macro */
        return  NULL;               /* Too deeply nested macro call */
    if (mode == STD) {
        get();                      /* Clear empty "file"s          */
        unget();                    /*      for diagnostic          */
        cur_cp = infile->bptr;      /* Remember current location    */
    }
    file = unget_string( in, outer ? outer->name : NULLST);
                                    /* Stack input on a "file"      */

    while ((c = get()), file == infile
        /* Rescanning is limited to the "file"  */
            && c != RT_END) {
            /*
             * This is the trick of STD mode.  collect_args() via replace()
             * may read over to file->parent (provided the "file" is macro)
             * unless stopped by RT_END.
             */
        if (c == ' ' || c == TOK_SEP) {
            *out_p++ = c;
            continue;
        }
        if (scan_token( c, (tp = out_p, &out_p), out_end) == NAM
                && c != DEF_MAGIC && (inner =
                look_id((mode == STD && c == IN_SRC) ? tp+1 : tp)) != NULL) {
                                            /* A macro name         */
            if (is_macro_call( inner, &out_p)
                    && ((mode == POST_STD && is_able_repl( inner))
                    || (mode == STD && (is_able = is_able_repl( inner)) == YES
                        || (is_able == READ_OVER && c == IN_SRC)))) {
                                            /* Really a macro call  */
                if ((out_p = replace( inner, tp, out_end, outer, file))
                        == NULL)            /* Error of macro call  */
                    break;
            } else if ((is_able = is_able_repl( inner)) == NO
                    || (mode == STD && is_able == READ_OVER && c != IN_SRC)) {
                if (mode == POST_STD || c != IN_SRC)
                    memmove( tp + 1, tp, (size_t) (out_p++ - tp));
                *tp = DEF_MAGIC;            /* Mark not to replace  */
            }                               /* Else not a macro call*/
        }
        if (out_end <= out_p) {
            *out_p = EOS;
            cerror( macbuf_overflow, outer ? outer->name : in, 0L, out);
            out_p = NULL;
            break;
        }
    }

    if (out_p) {
        *out_p = EOS;
        if (mode == STD) {
            if  (c != RT_END) {
                unget();
                if (outer != NULL) {    /* outer isn't a macro in argument  */
                    if (infile && infile->bptr != cur_cp
                                    /* Have overrun replacement list*/
                            && !(tp && *tp == DEF_MAGIC)
                                                /* Macro is enabled */
                            && ((!compat_mode && (warn_level & 1))
                                || (compat_mode && (warn_level & 8)))) {
                        cwarn(
"Replacement text \"%s\" of macro %.0ld\"%s\" involved subsequent text" /* _W1_ */
                            , in, 0L, outer->name);
                        if (! no_source_line)
                            dump_a_def( "    macro", outer, FALSE, FALSE, TRUE
                                    , fp_err);
                    }
                }
            }                       /* Else remove RT_END           */
        } else {
            unget();
        }
    }
    enable_repl( outer, TRUE);      /* Enable macro for later text  */
#if DEBUG
    if (debug & EXPAND) {
        fprintf( fp_debug, "rescan_level--%d (%s) "
                , rescan_level + 1, outer ? outer->name : "<arg>");
        dump_string( "rescan exit", out);
    }
#endif
    return  out_p;
}

static int
#if PROTO
disable_repl( const DEFBUF * defp)
#else
disable_repl( defp)
    DEFBUF *    defp;
#endif
/*
 * Register the macro name currently replacing.
 */
{
    if (defp == NULL)
        return  TRUE;
    if (rescan_level >= RESCAN_LIMIT) {
        cerror(
            "Rescanning macro \"%s\" more than %ld times at \"%s\"" /* _E_  */
                , macro_name, (long) RESCAN_LIMIT, defp->name);
        return  FALSE;
    }
    replacing[ rescan_level].def = defp;
    replacing[ rescan_level++].read_over = 0;
    return  TRUE;
}

static void
#if PROTO
enable_repl( const DEFBUF * defp, int done)
#else
enable_repl( defp, done)
    DEFBUF *    defp;
    int         done;
#endif
/*
 * Un-register the macro name just replaced for later text.
 */
{
    if (defp == NULL)
        return;
    replacing[ rescan_level - 1].def = NULL;
    if (done && rescan_level)
        rescan_level--;
}

static int
#if PROTO
is_able_repl( const DEFBUF * defp)
#else
is_able_repl( defp)
    DEFBUF *    defp;
#endif
/*
 * The macro is permitted to replace ?
 */
{
    register int    i;

    if (defp == NULL)
        return  YES;
    for (i = rescan_level-1; i >= 0; i--) {
        if (defp == replacing[ i].def) {
            return  replacing[ i].read_over;
        }
    }
    return  YES;
}

#endif  /* MODE == STANDARD   */


#if     MODE == PRE_STANDARD

/*
 *  M a c r o   E x p a n s i o n   i n   P R E - S T A N D A R D   M o d e
 */

#include    "setjmp.h"

static jmp_buf  jump;

static char *   arglist[ NMACPARS];         /* Pointers to args     */

#if PROTO

static int      rescan( int c, char * mp, char * mac_end);
static int      replace( DEFBUF * defp);
static void     substitute( DEFBUF * defp);

#else   /* ! PROTO  */

static int      rescan();           /* Replace a macro repeatedly   */
static int      replace();          /* Replace a macro once         */
static void     substitute();       /* Substitute parms with args   */

#endif  /* ! PROTO  */

char *
#if PROTO
expand( DEFBUF * defp, char * out, char * out_end)
#else
expand( defp, out, out_end)
    DEFBUF *    defp;                       /* Macro definition     */
    char *  out;                            /* Output buffer        */
    char *  out_end;                        /* End of output buffer */
#endif
/*
 * Expand a macro call completely, write the results to the specified buffer
 * and return the advanced pointer.
 */
{
    char    macrobuf[ NMACWORK + IDMAX];    /* Buffer for rescan()  */
    char *  mac_end = &macrobuf[ NMACWORK]; /* End of macrobuf[]    */
    char *  out_p;                          /* Pointer into out[]   */
    register char *     mp = macrobuf;      /* Pointer into macrobuf*/
    int     len;                            /* Length of a token    */
    int     token_type;                     /* Type of token        */
    int     c;

    macro_line = line;                      /* Line number for diag.*/
    unget_string( identifier, identifier);  /* To re-read           */
    rescan_level = 0;
    if (setjmp( jump) == 1) {
        skip_macro();
        mp = macrobuf;
        macro_line = MACRO_ERROR;
        goto  err_end;
    }

    while ((c = get()) != CHAR_EOF && infile->fp == NULL) {
                            /* While the input stream is a macro    */
        while (c == ' ') {                  /* Output the spaces    */
            *mp++ = c;
            c = get();
            if (infile == NULL || infile->fp != NULL)
                goto  exp_end;
        }
        token_type = rescan( c, mp, mac_end);   /* Scan token and   */
                /*  expand.  Result of expansion is written at mp.  */

        switch (token_type) {
        case STR:                           /* String literal       */
        case CHR:                           /* Character constant   */
        case NUM:                           /* Number token         */
        case OPE:                           /* Operator or punct.   */
        case NAM:                           /* Identifier           */
            len = strlen( mp);
            mp += len;
            break;
        case SEP:                           /* Special character    */
            switch( *mp) {
            case COM_SEP:
                if (mode == OLD_PREP)
                    break;  /* Zero-length comment is removed now   */
                /* Else fall through    */
            default:                        /* Who knows ?          */
                mp++;                       /* Copy the character   */
                break;
            }
            break;
        case NO_TOKEN:  break;              /* End of file          */
        default:                            /* Unkown token char.   */
            mp++;                           /* Copy the character   */
            break;
        }

        if (mac_end <= mp) {
            *mp = EOS;
            cerror( macbuf_overflow, defp->name, 0L, macrobuf);
            longjmp( jump, 1);
        }
#if DEBUG
        if (debug & GETC) {
            *mp = EOS;
            dump_string( "macrobuf", macrobuf);
        }
#endif
    }

exp_end:
    unget();
    while (macrobuf < mp && *(mp - 1) == ' ')
        mp--;                           /* Remove trailing blank    */
    macro_line = 0;
err_end:
    *mp = EOS;
    out_p = stpcpy( out, macrobuf);
#if DEBUG
    if (debug & EXPAND) {
        dump_string( "expand exit", out);
    }
#endif
    return  out_p;
}

static int
#if PROTO
rescan( int c, char * mp, char * mac_end)
#else
rescan( c, mp, mac_end)
    register int    c;              /* First character of token     */
    char *  mp;                     /* Output buffer                */
    char *  mac_end;                /* End of output buffer         */
#endif
/*
 * If the token is a macro name, replace the macro repeatedly until the first
 * token becomes a non-macro and return the type of token after expansion.
 */
{
    int     token_type;             /* Type of token                */
    char *  cp = mp;        /* Value of mp should not be changed    */
    DEFBUF *    defp;
    FILEINFO *  file;

    while ((token_type = scan_token( c, &cp, mac_end)) == NAM
            && (defp = look_id( identifier)) != NULL) { /* Macro    */
        if (replace( defp) == FALSE)
            break;                  /* Macro name with no argument  */
        file = infile;
        c = get();
        if (file != infile) {       /* Replaced to 0 token          */
            unget();
            token_type = NO_TOKEN;
            break;
        }
        cp = mp;                    /* Overwrite on the macro call  */
    }                               /* The macro call is replaced   */
    return  token_type;
}

static int
#if PROTO
replace( DEFBUF * defp)
#else
replace( defp)
    DEFBUF *    defp;               /* Definition of the macro      */
#endif
/*
 * Replace a macro one level.  Called from expand() (via rescan()) when a
 * identifier is found in the symbol table.  It calls collect_args() to parse
 * actual arguments, checking for the correct number.  It then creates a
 * "file" containing single line containing the replacement text with actual
 * arguments inserted appropriately.  This is "pushed back" onto the input
 * stream.  (When the get() routine runs off the end of the macro line, it
 * will dismiss the macro itself.)
 */
{
    int         nargs;                      /* Number of arguments  */
    register int        c;

#if DEBUG
    if (debug & EXPAND) {
        dump_a_def( "replace entry", defp, FALSE, FALSE, TRUE, fp_debug);
        dump_unget( "replace entry");
    }
#endif
    if (++rescan_level >= RESCAN_LIMIT) {
        cerror( "Recursive macro definition of \"%s\""      /* _E_  */
                , defp->name, 0L, NULLST);
        longjmp( jump, 1);
    }

    /*
     * Here's a macro to replace.
     */
    switch (defp->nargs) {

    case DEF_NOARGS:                /* No argument just stuffs      */
    case DEF_NOARGS - 1:                    /* Non-standard predef  */
        break;

    default:                                /* defp->nargs >= 0     */
        /*
         * Nothing funny about this macro.
         */
        c = squeeze_ws( (char **)NULL);     /* Look for and skip '('*/
        if (c != '(') {
            /*
             * If the programmer writes
             *      #define foo() ...
             *      ...
             *      foo [no ()]
             * just write foo to output.
             */
            unget();
            if (warn_level & 8)
                cwarn( only_name, defp->name, 0L, NULLST);
            return  FALSE;
        } else {
            arglist[ 0] = xmalloc( (size_t) (NMACWORK + IDMAX * 2));
            nargs = collect_args( defp, arglist);
                                            /* Collect arguments    */
            if (nargs == ARG_ERROR) {       /* End of input         */
                free( arglist[ 0]);
                longjmp( jump, 1);
            }
        }
        break;
    }                                       /* defp->nargs switch   */

    if (defp->nargs > 0)
        substitute( defp);                  /* Do actual arguments  */
    else
        unget_string( defp->repl, defp->name);

#if DEBUG
    if (debug & EXPAND)
        dump_unget( "replace exit");
#endif
    if (defp->nargs >= 0)
        free( arglist[ 0]);
    return  TRUE;
}

static void
#if PROTO
substitute( DEFBUF * defp)
#else
substitute( defp)
    DEFBUF *        defp;           /* Current macro being replaced */
#endif
/*
 * Stuff the macro body, substituting formal parameters with actual arguments.
 */
{
    register int    c;                      /* Current character    */
    FILEINFO *      file;                   /* Funny #include       */
    char *          out_end;                /* -> output buffer end */
    char *          in_p;                   /* -> replacement text  */
    register char * out_p;                  /* -> macro output buff */

    file = get_file( defp->name, (size_t) (NMACWORK + 1));
    in_p = defp->repl;                      /* -> macro replacement */
    out_p = file->buffer;                   /* -> output buffer     */
    out_end = file->buffer + NMACWORK;      /* -> buffer end        */

    while ((c = *in_p++) != EOS) {
        if (c == MAC_PARM) {
            c = (*in_p++ & UCHARMAX) - 1;   /* Parm number          */
            /*
             * Substitute formal parameter with actual argument.
             */
            if (out_end <= (out_p + strlen( arglist[ c])))
                goto nospace;
            out_p = stpcpy( out_p, arglist[ c]);
        } else {
            *out_p++ = c;
        }
        if (out_end <= out_p)
            goto  nospace;
    }

    *out_p = EOS;
    file->buffer = xrealloc( file->buffer, strlen( file->buffer) + 1);
    file->bptr = file->buffer;              /* Truncate buffer      */
#if DEBUG
    if (debug & EXPAND)
        dump_string( "substitute macroline", file->buffer);
#endif
    return;

nospace:
    *out_p = EOS;
    cerror( macbuf_overflow, defp->name, 0L, file->buffer);
    longjmp( jump, 1);
}

#endif  /* MODE == PRE_STANDARD */


/*
 *                  C O M M O N   R O U T I N E S
 *  f o r   S T A N D A R D   a n d   p r e - S T A N D A R D   M o d e s
 */

static int
#if PROTO
collect_args( const DEFBUF * defp, char ** arglist)
#else
collect_args( defp, arglist)
    DEFBUF *    defp;           /* Definition of the macro          */
    char **     arglist;        /* Pointers to actual arguments     */
#endif
/*
 *   Collect the actual arguments for the macro, checking for correct number
 * of arguments.
 *   Variable arguments (on MODE == STANDARD) are read as a merged argument.
 *   Return number of arguments, or ARG_ERROR on error of unterminated macro.
 *   collect_args() may read over to the next line unless 'in_directive' is
 * set to TRUE.
 *   collect_args() may read over into file->parent to complete a macro call
 * unless stopped by RT_END (provided the "file" is macro).  This is a key
 * trick of STD mode macro expansion.  Meanwhile, POST_STD mode limits the
 * arguments in the "file" (macro or not).
 *   Note: arglist[ n] may be reallocated by collect_args().
 */
{
    const char *    name = defp->name;
    char *  argp = arglist[ 0];         /* Pointer to an argument   */
    char *  arg_end;                    /* End of arguments buffer  */
    char *  valid_argp;                 /* End of valid arguments   */
    char *  sequence;           /* Token sequence for diagnostics   */
    char *  seq;                /* Current pointer into 'sequence'  */
    char *  seq_end;                            /* Limit of buffer  */
    int     args;               /* Number of arguments expected     */
    int     nargs = 0;                  /* Number of collected args */
    int     var_arg = defp->nargs & VA_ARGS;    /* Variable args    */
    int     more_to_come = FALSE;       /* Next argument is expected*/
    int     ret = ARG_ERROR;                    /* Return value     */
    register int    c;

#if DEBUG
    if (debug & EXPAND)
        dump_unget( "collect_args entry");
#endif
    args = (defp->nargs == DEF_PRAGMA) ? 1 : (defp->nargs & ~VA_ARGS);
    if (args == 0)                      /* Need no argument         */
        valid_argp = argp;
    *argp = EOS;                        /* Make sure termination    */
    arg_end = argp + NMACWORK/2;
    seq = sequence = arg_end + IDMAX;   /* Use latter half of argp  */
    seq_end = seq + NMACWORK/2;
    seq = stpcpy( seq, name);
    *seq++ = '(';
#if MODE == STANDARD
/*
 * in_getarg is set TRUE while getting macro arguments, for the sake of
 * diagnostic's convenience.
 * in_getarg is used only in STD mode.
 */
    in_getarg = TRUE;
#endif

    while (1) {
        c = squeeze_ws( &seq);          /* Skip white spaces        */
        if (c == ')' || c == ',')
            scan_token( c, &seq, seq_end);  /* Ensure token parsing */
        else
            *seq = EOS;

        switch (c) {                    /* First character of token */
        case ')':
            if (! more_to_come)         /* Zero argument            */
                break;                  /* Else fall through        */
        case ',':                       /* Empty argument           */
            if (warn_level & 2)
                cwarn( empty_arg, sequence, 0L, NULLST);
#if MODE == STANDARD
            if (var_arg && nargs == args - 1)
                /* Variable arguments begin with an empty argument  */
                c = get_an_arg( c, &argp, arg_end, &seq, 1);
            else
#endif
            {
#if MODE == STANDARD
                if (mode == STD)
                    *argp++ = RT_END;
#endif
                *argp++ = EOS;
            }
            if (++nargs == args)
                valid_argp = argp;
            if (c == ',') {
                more_to_come = TRUE;
                continue;
            } else {                    /* ')'                      */
                break;
            }
        case '\n':      /* Unterminated macro call in control line  */
            unget();                    /* Fall through             */
#if MODE == STANDARD
        case RT_END:                    /* Error of missing ')'     */
            cerror( unterm_macro, sequence, 0L, NULLST);
            if (! no_source_line)
                dump_a_def( "    macro", defp, FALSE, FALSE, TRUE, fp_err);
                                        /* Fall through             */
#endif
        case CHAR_EOF:                  /* End of file in macro call*/
            goto  arg_ret;              /* Diagnosed by at_eof()    */
        default:                        /* Nomal argument           */
            break;
        }

        if (c == ')')                   /* At end of all args       */
            break;

        c = get_an_arg( c, &argp, arg_end, &seq,
            (var_arg && nargs == args - 1) ? 1 : 0);

        if (++nargs == args)
            valid_argp = argp;          /* End of valid arguments   */
        if (c == ')')
            break;
        if (c == 0)                     /* End of file              */
            goto  arg_ret;              /* Diagnosed by at_eof()    */
        if (c == -1) {                  /* Untermanated macro call  */
            cerror( unterm_macro, sequence, 0L, NULLST);
            if (! no_source_line)
                dump_a_def( "    macro", defp, FALSE, FALSE, TRUE, fp_err);
            goto  arg_ret;
        }
        more_to_come = (c == ',');
    }                                   /* Collected all arguments  */

    if (nargs == 0 && args == 1) {      /* Only and empty argument  */
        if (warn_level & 1)
            cwarn( empty_arg, sequence, 0L, NULLST);
    } else if (nargs != args) {         /* Wrong number of arguments*/
#if MODE == STANDARD
        if (var_arg && (nargs == args -1)) {        /* Empty variable args  */
            if (warn_level & 1) {
                cwarn( narg_error, nargs < args ? "Less" : "More"
                        , (long) args, sequence);
                if (! no_source_line)
                    dump_a_def( "    macro", defp, FALSE, FALSE, TRUE, fp_err);
            }
        } else
#endif
        {
#if MODE == PRE_STANDARD
            if (mode == OLD_PREP) { 
                if (warn_level & 1)
                    cwarn( narg_error, nargs < args ? "Less" : "More"
                        , (long) args, sequence);
            } else {
                cerror( narg_error, nargs < args ? "Less" : "More"
                    , (long) args, sequence);
            }
#else
            cerror( narg_error, nargs < args ? "Less" : "More"
                    , (long) args, sequence);
#endif
            if (! no_source_line)
                dump_a_def( "    macro", defp, FALSE, FALSE, TRUE, fp_err);
        }
    }
    if (args < nargs) {
        argp = valid_argp;              /* Truncate excess arguments*/
    } else {
        for (c = nargs; c < args; c++) {
#if MODE == STANDARD
            if (mode == STD)
                *argp++ = RT_END;       /* For rescan()             */
#endif
            *argp++ = EOS;              /* Missing arguments        */
        }
        if (c == 0)
            argp++;                     /* Ensure positive length   */
    }
    arglist[ 0] = argp
            = xrealloc( arglist[ 0], (size_t)(argp - arglist[ 0]));
                                        /* Use memory sparingly     */
    for (c = 1; c < args; c++)
        arglist[ c] = argp += strlen( argp) + 1;

    ret = nargs;
arg_ret:
#if DEBUG
    if (debug & EXPAND) {
        if (nargs > 0)
            dump_args( "collect_args exit"
                    , nargs < args ? nargs : args, arglist);
        dump_unget( "collect_args exit");
    }
#endif
#if MODE == STANDARD
    in_getarg = FALSE;
#endif
    return  ret;
}

static int
#if PROTO
get_an_arg( int c, char ** argpp, char * arg_end, char ** seqp, int var_arg)
#else
get_an_arg( c, argpp, arg_end, seqp, var_arg)
    register int    c;
    char ** argpp;      /* Address of pointer into argument list    */
    char *  arg_end;                /* End of argument list buffer  */
    char ** seqp;                   /* Buffer for diagnostics       */
    int     var_arg;                /* 1 on __VA_ARGS__, 0 on others*/
#endif
/*
 * Get an argument of macro into '*argpp', return the next punctuator.
 * Variable arguments are read as a merged argument.
 */
{
    int     end_an_arg = FALSE;             /* End-of-an-arg flag   */
    int     paren = var_arg;                /* For embedded ()'s    */
    int     token_type;
    char *  prevp;
    char *  argp = *argpp;

    while (1) {
        if (c == '\n'                       /* In control line      */
#if MODE == STANDARD
                || c == RT_END              /* Boundary of rescan   */
#endif
                ) {
            if (c == '\n')
                unget();
            break;
        }
        token_type = scan_token( c, (prevp = argp, &argp), arg_end);
                                            /* Scan the next token  */
        switch (c) {
        case '(' :                          /* Worry about balance  */
            paren++;                        /* To know about commas */
            break;
        case ')' :                          /* Other side too       */
            if (paren-- == var_arg)         /* At the end?          */
                end_an_arg = TRUE;          /* Else more to come    */
            break;
        case ',' :
            if (paren == 0)                 /* Comma delimits arg   */
                end_an_arg = TRUE;
            break;
        case CHAR_EOF:                      /* Unexpected EOF       */
            return  0;
        default :                           /* Any token            */
#if MODE == STANDARD
            if (mode == STD && ! compat_mode && token_type == NAM
                    && c != IN_SRC && c != DEF_MAGIC && infile->fp) {
                memmove( prevp + 1, prevp, (size_t) (argp++ - prevp));
                *prevp = IN_SRC;
            }           /* Mark the name is read from source file   */
#endif
            break;
        }                                   /* End of switch        */

        if (end_an_arg)                     /* End of an argument   */
            break;
        c = squeeze_ws( &argp);             /* To the next token    */
    }                                       /* Collected an argument*/

    *argp = EOS;
    *seqp = stpcpy( *seqp, *argpp);         /* Save the sequence    */
#if MODE == STANDARD
    if (c == '\n' || c == RT_END)
#else
    if (c == '\n')
#endif
        return  -1;                         /* Unterminated macro   */
    argp--;                                 /* Remove the punctuator*/
    while (*argpp < argp && *(argp - 1) == ' ')
        --argp;                     /* Remove trailing blanks       */
#if MODE == STANDARD
    if (mode == STD)
        *argp++ = RT_END;                   /* For rescan()         */
#endif
    *argp++ = EOS;                          /* Terminate an argument*/
    *argpp = argp;
    return  c;
}

static int
#if PROTO
squeeze_ws( char ** out)
#else
squeeze_ws( out)
    char **     out;                /* pointer to output pointer    */
#endif
/*
 * Squeeze white spaces to one space.
 * White spaces are ' ' ('\t', '\r', '\v', '\f' converted to ' ' by get()),
 * and '\n' unless in_directive is set.
 * COM_SEP is skipped.  TOK_SEPs are squeezed to one TOK_SEP.
 * If white spaces are found and out is not NULL, write a space to *out and
 * increment *out.
 * Return the next character.
 */
{
    register int    c;
    int     space = 0;
#if MODE == STANDARD
    FILEINFO *      file = infile;
    int     tsep = 0;
    FILE *  fp = infile->fp;
#endif

#if MODE == STANDARD
    while ((type[ c = get()] & SPA) && ((mode == POST_STD && file == infile)
            || (mode == STD && ((macro_line != 0 && macro_line != MACRO_ERROR)
                || file == infile))))
#else   /* MODE == PRE_STANDARD */
    while (type[ c = get()] & SPA)
#endif  /* MODE == PRE_STANDARD */
    {
        if (c == '\n') {
            if (in_directive)           /* If scanning control line */
                break;                  /*   do not skip newline.   */
            else
                wrong_line = TRUE;
        }
#if MODE == STANDARD
        if (c == TOK_SEP) {
            tsep++;
            continue;
        }
#else
        if (c == COM_SEP)               /* Skip COM_SEP             */
            continue;
#endif
        space++;
    }

    if (out) {
        if (space)              /* Write a space to output pointer  */
            *(*out)++ = ' ';    /*   and increment the pointer.     */
#if MODE == STANDARD
        else if (tsep)
            *(*out)++ = TOK_SEP;
#endif
    }
#if MODE == STANDARD
    if (mode == POST_STD && file != infile) {
        unget();                /* Arguments cannot cross "file"s   */
        c = fp ? CHAR_EOF : RT_END; /* EOF is diagnosed by at_eof() */
    } else if (mode == STD && macro_line == MACRO_ERROR && file != infile) {
                                /* EOF  */
        unget();                /*   diagnosed by at_eof() or only  */
        c = CHAR_EOF;           /*   name of a function-like macro. */
    }                       /* at_eof() resets macro_line on error  */
#endif
    return  c;                  /* Return the next character        */
}

static void
#if PROTO
skip_macro( void)
#else
skip_macro()
#endif
/*
 * Clear the stacked (i.e. half-expanded) macro, called on macro error.
 */
{
    if (infile == NULL)                     /* End of input         */
        return;
    if (infile->fp)                         /* Source file          */
        return;
    while (infile->fp == NULL) {            /* Stacked stuff        */
        infile->bptr += strlen( infile->bptr);
        get();                              /* To the parent "file" */
    }
    unget();
}

#if DEBUG

static void
#if PROTO
dump_args( const char * why, int nargs, char ** arglist)
#else
dump_args( why, nargs, arglist)
    char    *why;
    int     nargs;
    char    **arglist;
#endif
/*
 * Dump arguments list.
 */
{
    register int    i;

    fprintf( fp_debug, "dump of %d actual arguments %s\n", nargs, why);
    for (i = 0; i < nargs; i++) {
        fprintf( fp_debug, "arg[%d]", i + 1);
        dump_string( NULLST, arglist[ i]);
    }
}

#endif  /* DEBUG    */

