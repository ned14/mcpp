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
 *                          S U P P O R T . C
 *                  S u p p o r t   R o u t i n e s
 *
 * Edit History of DECUS CPP / cpp6.c
 * 31-Aug-84 MM         USENET net.sources release
 * 30-May-85            Latest revision
 */

/*
 * CPP Version 2.0 / support.c
 * 1998/08      kmatsui
 *      Renamed cpp6.c support.c.
 *      Created get_unexpandable(), scantoken(), cat_line(), scanop(),
 *          parse_line(), last_is_mbchar(), cnv_digraph(), at_eof(),
 *          xrealloc(), putline(), dumptoken().
 *      Split getline(), read_a_comment() from get().
 *      Extended cfatal(), cerror(), cwarn(), removing cierror(), ciwarn().
 *      Removed save(), cget().
 *      Moved macroid(), catenate(), appendstring() from cpp6.c to expand.c,
 *          lookid(), defendel() from cpp6.c to control.c.
 *      Renamed scanstring() to scanquote(), getmem() to xmalloc().
 *      Revised most of the functions.
 *      Revised line splicing and tokenization.
 */

/*
 * CPP Version 2.1 / support.c
 * 1998/09      kmatsui
 *      Enabled UCN sequence in identifier.
 *      Made implementable multi-byte characters in identifier.
 */

/*
 * CPP Version 2.2 / support.c
 * 1998/11      kmatsui
 *      Updated UCN constraint on C++ according to C++ Standard.
 */

/*
 * CPP Version 2.3 pre-release 1 / support.c
 * 2002/08      kmatsui
 *      Fixed the bug of handling digraphs.
 *      Implemented UCN in pp-number and string-literal.
 *      Renamed the several functions using underscore.
 *
 * CPP Version 2.3 pre-release 2 / support.c
 * 2002/12      kmatsui
 *      Slightly modified.
 *
 * CPP Version 2.3 release / support.c
 * 2003/02      kmatsui
 *      Created id_operator() for C++98 identifier-like operators.
 *      Modified the format of diagnostics (for GNU C testsuite).
 */

/*
 * MCPP Version 2.4 prerelease
 * 2003/11      kmatsui
 *      Revised get(), get_file() and do_msg() according to the change of
 *          FILEINFO.
 *
 * MCPP Version 2.4 release
 * 2004/02      kmatsui
 *      Revised scan_token(), scan_quote() and some others according to the new
 *          routine for multi-byte character handling.
 */

/*
 * MCPP Version 2.5
 * 2005/03      kmatsui
 *      Absorbed POST_STANDARD into STANDARD and OLD_PREPROCESSOR into
 *          PRE_STANDARD.
 */

/*
 * The common routines used by several source files are placed here.
 */

/*
 * The following are global functions.
 *
 * get_unexpandable()   Gets the next unexpandable token in the line, expanding
 *              macros.
 *              Called from #if, #line and #include processing routines.
 * skip_nl()    Skips over a line.
 * skip_ws()    Skips over white spaces but not skip over the end of the line.
 *              skip_ws() skips also COM_SEP and TOK_SEP.
 * scan_token() Reads the next token of any type into the specified output
 *              pointer, advances the pointer, returns the type of token.
 * scan_quote() Reads a string literal, character constant or header-name from
 *              the input stream, writes out to the specified buffer and
 *              returns the advanced output pointer.
 * get()        Reads the next byte from the current input stream, handling
 *              end of (macro/file) input and embedded comments appropriately.
 * cnv_trigraph()   Maps trigraph sequence to C character.
 * cnv_digraph()    Maps digraph sequence to C character.
 * id_operator()    See whether the identifier is an operator in C++.
 * unget()      Pushs last gotten character back on the input stream.
 * unget_string()   Pushs sequence on the input stream.
 * save_string() Saves a string in malloc() memory.
 * get_file()   Initializes a new FILEINFO structure, called when #include
 *              opens a new file.
 * xmalloc()    Gets a specified number of bytes from heap memory.
 *              If malloc() returns NULL, exits with a message.
 * xrealloc()   realloc().  If it fails, exits with a message.
 * cfatal(), cerror(), cwarn()
 *              These routines format print messages to the user.
 */

#if PREPROCESSED
#include    "mcpp.H"
#else
#include    "system.H"
#include    "internal.H"
#endif

#if PROTO

static void     scan_id( int c);
static char *   scan_number( int c, char * out, char * out_end);
#if MODE == STANDARD && OK_UCN
static char *   scan_ucn( int cnt, char * out);
#endif
static char *   scan_op( int c, char * out);
static char *   parse_line( void);
static char *   read_a_comment( char * sp);
static char *   get_line( int in_comment);
static void     at_eof( int in_comment);
static void     do_msg( const char * severity, const char * format
        , const char * arg1, long arg2, const char * arg3);
static char *   cat_line( int del_bsl);
#if BSL_IN_MBCHAR
static int      last_is_mbchar( const char * in, int len);
#endif
#if MODE == PRE_STANDARD
static void     put_line( char * out, FILE * fp);
#endif
#if DEBUG
static void     dump_token( int token_type, const char * cp);
#endif

#else   /* ! PROTO  */

static void     scan_id();          /* Scan an identifier           */
static char *   scan_number();      /* Scan a preprocessing number  */
#if MODE == STANDARD && OK_UCN
static char *   scan_ucn();         /* Scan an UCN sequence         */
#endif
static char *   scan_op();          /* Scan an operator or punctuat.*/
static char *   parse_line();       /* Parse a logical line         */
static char *   read_a_comment();   /* Read over a comment          */
static char *   get_line();         /* Get a logical line from file */
static void     at_eof();           /* Check erroneous end of file  */
static void     do_msg();           /* Putout diagnostic message    */
static char *   cat_line();         /* Splice the line              */
#if BSL_IN_MBCHAR
static int      last_is_mbchar();   /* The line ends with MBCHAR ?  */
#endif
#if PRE_STANDARD
static void     put_line();         /* Put out a logical line       */
#endif
#if DEBUG
static void     dump_token();       /* Dump a token and its type    */
#endif

#endif  /* ! PROTO  */

#if MODE == PRE_STANDARD
static int  in_string = FALSE;  /* For get() and parse_line()   */
#endif

int
#if PROTO
get_unexpandable( int c, int diag)
#else
get_unexpandable( c, diag)
    int     c;                              /* First of token       */
    int     diag;                           /* Flag of diagnosis    */
#endif
/*
 * Get the next unexpandable token in the line, expanding macros.
 * Return the token type.  The token is written in work[].
 * Called only from the routines processing #if (#elif, #assert), #line
 * and #include directives.
 */
{
    DEFBUF *    defp = NULL;
    FILEINFO *  file;
    FILE *  fp = NULL;
    int     token_type = NO_TOKEN;

    while (c != EOS && c != '\n'                /* In a line        */
            && (fp = infile->fp         /* Preserve current state   */
                , (token_type
                        = scan_token( c, (workp = work, &workp), work_end))
                    == NAM)                     /* Identifier       */
            && fp != NULL                       /* In source        */
            && (defp = is_macro( (char **)NULL)) != NULL) { /* Macro*/
        expand( defp, work, work_end);  /* Expand the macro call    */
        file = unget_string( work, defp->name); /* Stack to re-read */
        c = skip_ws();                          /* Skip TOK_SEP     */
        if (file != infile && macro_line != MACRO_ERROR && (warn_level & 1)) {
            /* This diagnostic is issued even if "diag" is FALSE.   */
            cwarn( "Macro \"%s\" is expanded to 0 token"    /* _W1_ */
                    , defp->name, 0L, NULLST);
            if (! no_source_line)
                dump_a_def( "    macro", defp, FALSE, FALSE, TRUE, fp_err);
        }
    }

    if (c == '\n' || c == EOS) {
        unget();
        return  NO_TOKEN;
    }

    if (diag && fp == NULL && defp && token_type == NAM) {
#if MODE == STANDARD
        if (str_eq( identifier, "defined") && (warn_level & 1))
            cwarn( "Macro \"%s\" is expanded to \"defined\""        /* _W1_ */
                    , defp->name, 0L, NULLST);
#endif
#if OK_SIZE
        if (str_eq( identifier, "sizeof") && (warn_level & 1))
            cwarn( "Macro \"%s\" is expanded to \"sizeof\""         /* _W1_ */
                    , defp->name, 0L, NULLST);
#endif
    }
    return  token_type;
}

void
#if PROTO
skip_nl( void)
#else
skip_nl()
#endif
/*
 * Skip to the end of the current input line.
 */
{
#if MODE == STANDARD
    insert_sep = NO_SEP;
#endif
    while (infile && infile->fp == NULL) {  /* Stacked text         */
        infile->bptr += strlen( infile->bptr);
        get();                              /* To the parent        */
    }
    if (infile)
        infile->bptr += strlen( infile->bptr);  /* Source line      */
}

int
#if PROTO
skip_ws( void)
#else
skip_ws()
#endif
/*
 * Skip over whitespaces other than <newline>.
 * Note: POST_STD mode does not use TOK_SEP, and KR mode does not use COM_SEP.
 */
{
    register int            c;

    do {
        c = get();
    }
#if MODE == STANDARD
    while (c == ' ' || c == TOK_SEP);
#else
    while (c == ' ' || c == COM_SEP);
#endif
    return  c;
}

#if MODE == STANDARD
/*
 * The following macros are defined locally for scan_token(), scan_id(),
 * scan_quote(), scan_number(), scan_ucn() and scan_op() in STANDARD mode to
 * simpify tokenization.  Any token cannot cross "file"s.
 */
#define get()   (*infile->bptr++ & UCHARMAX)
#define unget() (infile->bptr--)
#endif  /* MODE == STANDARD */

int
#if PROTO
scan_token( int c, char ** out_pp, char * out_end)
#else
scan_token( c, out_pp, out_end)
    int     c;                  /* The first character of the token */
    char ** out_pp;             /* Pointer to pointer to output buf */
    char *  out_end;            /* End of output buffer             */
#endif
/*
 *   Scan the next token of any type.
 *   The token is written out to the specified buffer and the output pointer
 * is advanced.  Token is terminated by EOS.  Return the type of token.
 *   If the token is an identifier, the token is also in identifier[].
 *   If the token is a operator or punctuator, return OPE.
 *   If 'c' is token separator, then return SEP.
 *   If 'c' is not the first character of any known token and not a token
 * separator, return SPE.
 *   In POST_STD mode, inserts token separator (a space) between any tokens of
 * source.
 */
{
    register char *     out = *out_pp;  /* Output pointer           */
    int     ch_type;                    /* Type of character        */
    int     token_type = 0;             /* Type of token            */
    int     ch;

    ch_type = type[ c & UCHARMAX] & mbmask;
    c = c & UCHARMAX;

    switch (ch_type) {
    case LET:                           /* An identifier            */
        switch (c) {
#if MODE == STANDARD
        case 'L':
            ch = get();
            if (type[ ch] & QUO) {      /* type[ ch] == QUO         */
                if (ch == '"')
                    token_type = WSTR;  /* Wide-char string literal */
                else
                    token_type = WCHR;  /* Wide-char constant       */
                c = ch;
                *out++ = 'L';
                break;                  /* Fall down to "case QUO:" */
            } else {
                unget();
            }                           /* Fall through             */
#endif
        default:
#if MODE == STANDARD && (OK_UCN || OK_MBIDENT)
ident:
#endif
            scan_id( c);
            out = stpcpy( out, identifier);
            token_type = NAM;
            break;
        }
        if (token_type == NAM)
            break;
        /* Else fall through    -- i.e. WSTR, WCHR  */
    case QUO:                   /* String or character constant     */
        out = scan_quote( c, out, out_end, FALSE);
        if (token_type == 0) {
            if (c == '"')
                token_type = STR;
            else
                token_type = CHR;
        }   /* Else WSTR or WCHR    */
        break;
    case DOT:
        ch = get();
        unget();
        if ((type[ ch] & DIG) == 0)     /* Operator '.' or '...'    */
            goto  operat;
        /* Else fall through    */
    case DIG:                           /* Preprocessing number     */
        out = scan_number( c, out, out_end);
        token_type = NUM;
        break;
    case PUNC:
operat: out = scan_op( c, out);         /* Operator or punctuator   */
        token_type = OPE;       /* Number is set in global "openum" */
        break;
    default:
#if MODE == STANDARD
#if OK_UCN
        if (mode == STD && c == '\\' && stdc2) {
            ch = get();
            unget();
            if (ch == 'U' || ch == 'u')
                goto  ident;            /* Universal-Characte-Name  */
        }
#endif
#if OK_MBIDENT
        if (mode == STD && (type[ c] & mbstart) && stdc3) {
            char *  bptr = infile->bptr;
            mb_read( c, &infile->bptr, &out);
            infile->bptr = bptr;
            out = *out_pp;
            goto  ident;
        }
#endif
#endif  /* MODE == STANDARD */
        if
#if MODE == PRE_STANDARD
            (type[ c] & SPA)
#else
            ((type[ c] & SPA) || c == CAT || c == ST_QUOTE)
#endif
            token_type = SEP;       /* Token separator or magic char*/
        else
            token_type = SPE;
            /* Unkown token ($, @, multi-byte character or Latin    */
        *out++ = c;
        *out = EOS;
        break;
    }

    if (out_end < out)
        cfatal( "Buffer overflow scanning token \"%s\""     /* _F_  */
                , *out_pp, 0L, NULLST);
#if DEBUG
    if (debug & TOKEN)
        dump_token( token_type, *out_pp);
#endif
#if MODE == STANDARD
    if (mode == POST_STD && token_type != SEP && infile->fp != NULL
            && (type[ *infile->bptr & UCHARMAX] & SPA) == 0)
        insert_sep = INSERT_SEP;    /* Insert token separator       */
#endif
    *out_pp = out;

    return  token_type;
}

static void
#if PROTO
scan_id( int c)
#else
scan_id( c)
    int     c;                              /* First char of id     */
#endif
/*
 * Reads the next identifier and put it into identifier[].
 * The caller has already read the first character of the identifier.
 */
{
#if DOLLAR_IN_NAME
    static int      diagnosed = FALSE;  /* Flag of diagnosing '$'   */
#endif
    static char * const     limit = &identifier[ IDMAX];
#if MODE == STANDARD
#if OK_UCN
    int     uc2 = 0, uc4 = 0;           /* Count of UCN16, UCN32    */
#endif
#if OK_MBIDENT
    int     mb = 0;                     /* Count of MBCHAR  */
#endif
#endif  /* MODE == STANDARD */
    size_t  len;                        /* Length of identifier     */
    register char *     bp = identifier;

    do {
        if (bp < limit)
            *bp++ = c;
#if MODE == STANDARD
#if OK_UCN
        if (mode == STD && c == '\\' && stdc2) {
            int     cnt;
            char *  tp = bp;
            
            if ((c = get()) == 'u') {
                cnt = 4;
            } else if (c == 'U') {
                cnt = 8;
            } else {
                unget();
                bp--;
                break;
            }
            *bp++ = c;
            if ((bp = scan_ucn( cnt, bp)) == NULL)      /* Error    */
                return;
            if (cnt == 4)
                uc2++;
            else if (cnt == 8)
                uc4++;
            if (limit <= tp)            /* Too long identifier      */
                bp = tp;                /* Back the pointer         */
            goto  next_c;
        }
#endif  /* OK_UCN   */
#if OK_MBIDENT
        if (mode == STD && (type[ c] & mbstart) && stdc3) {
            len = mb_read( c, &infile->bptr, &bp);
            if (len & MB_ERROR) {
                if (infile->fp)
                    cerror(
                    "Illegal multi-byte character sequence."    /* _E_  */
                            , NULLST, 0L, NULLST);
            } else {
                mb += len;
            }
        }
#endif  /* OK_MBIDENT   */
#if OK_UCN
next_c:
#endif
#endif  /* MODE == STANDARD */
        c = get();
    } while ((type[ c] & (LET | DIG))           /* Letter or digit  */
#if MODE == STANDARD
#if OK_UCN
            || (mode == STD && c == '\\' && stdc2)
#endif
#if OK_MBIDENT
            || (mode == STD && (type[ c] & mbstart) && stdc3)
#endif
#endif  /* MODE == STANDARD */
        );

    unget();
    *bp = EOS;

    if (bp >= limit && (warn_level & 1))        /* Limit of token   */
        cwarn( "Too long identifier truncated to \"%s\""    /* _W1_ */
                , identifier, 0L, NULLST);

    len = bp - identifier;
#if MODE == STANDARD && IDMAX > IDLEN90MIN
    /* UCN16, UCN32, MBCHAR are counted as one character for each.  */
#if OK_UCN
    if (mode == STD)
        len -= (uc2 * 5) - (uc4 * 9);
#endif
#if OK_MBIDENT
    if (mode == STD)
        len -= mb;
#endif
    if (infile->fp && len > id_len_min && (warn_level & 4))
        cwarn( "Identifier longer than %.0s%ld characters \"%s\""   /* _W4_ */
                , NULLST, (long) id_len_min, identifier);
#endif  /* MODE == STANDARD && IDMAX > IDLEN90MIN   */

#if DOLLAR_IN_NAME
    if (diagnosed == FALSE && (warn_level & 2)
            && strchr( identifier, '$') != NULL) {
        cwarn( "'$' in identifier \"%s\"", identifier, 0L, NULLST); /* _W2_ */
        diagnosed = TRUE;                   /* Diagnose only once   */
    }
#endif
}

char *
#if PROTO
scan_quote( int delim, char * out, char * out_end, int diag)
#else
scan_quote( delim, out, out_end, diag)
    int         delim;              /* ', " or < (header-name)      */
    char *      out;                /* Output buffer                */
    char *      out_end;            /* End of output buffer         */
    int         diag;               /* Diagnostic should be output  */
#endif
/*
 * Scan off a string literal or character constant to the output buffer.
 * Report diagnosis if the quotation is terminated by newline or character
 * constant is empty (provided 'diag' is TRUE).
 * Return the next output pointer or NULL (on error).
 */
{
    const char * const      skip_line = ", skipped the line";   /* _E_  */
    const char * const      unterm_string
                        = "Unterminated string literal%s";
    const char * const      unterm_char
                        = "Unterminated character constant %s%.0ld%s";
    const char *    skip;
    size_t      len;
    register int    c;
    char *      out_p = out;

    *out_p++ = delim;
    if (delim == '<')               /*   header-name by <, > is an  */
        delim = '>';                /*     obsolescent feature.     */

scan:
    while ((c = get()) != EOS) {

#if USE_MBCHAR
        if (type[ c] & mbstart) {
            /* First of multi-byte character (or shift-sequence)    */
            char *  bptr = infile->bptr;
            len = mb_read( c, &infile->bptr, (*out_p++ = c, &out_p));
            if (len & MB_ERROR) {
                if (infile->fp != NULL && compiling && diag) {
                    if (warn_level & 1) {
                        char *  buf;
                        size_t  chlen;
                        buf = xmalloc( chlen = infile->bptr - bptr + 2);
                        memcpy( buf, bptr, chlen - 1);
                        buf[ chlen - 1] = EOS;
                        cwarn(
    "Illegal multi-byte character sequence \"%s\" in quotation",    /* _W1_ */
                        buf, 0L, NULLST);
                        free( buf);
                    }
                }
                continue;
            } else {        /* Valid multi-byte character (or sequence) */
                goto  chk_limit;
            }
        }
#endif
        if (c == delim) {
            break;
        } else if (c == '\\'
                && delim != '>'             /* In string literal    */
            ) {
#if MODE == STANDARD && OK_UCN
            if (mode == STD && stdc2) {
                int         cnt;
                char *      tp;

                *out_p++ = c;
                if ((c = get()) == 'u') {
                    cnt = 4;
                } else if (c == 'U') {
                    cnt = 8;
                } else {
                    goto  escape;
                }
                *out_p++ = c;
                if ((tp = scan_ucn( cnt, out_p)) != NULL)
                    out_p = tp;
                /* Else error   */
                continue;       /* Error or not, anyway continue    */
            }
#endif  /* MODE == STANDARD && OK_UCN   */
            *out_p++ = c;                   /* Escape sequence      */
            c = get();
escape:
#if USE_MBCHAR
            if (type[ c] & mbstart) {   /* '\\' followed by multi-byte char */
                unget();
                continue;
            }
#endif
#if MODE == PRE_STANDARD
            if (c == '\n') {                /* <backslash><newline> */
                out_p--;                    /* Splice the lines     */
                if (cat_line( TRUE) == NULL)        /* End of file  */
                    break;
                c = get();
            }
#endif
#if MODE == STANDARD
        } else if (c == ' ' && delim == '>' && infile->fp == NULL) {
            continue;   /* Skip space possibly inserted by macro expansion  */
#endif
        } else if (c == '\n') {
            break;
        }
        if (diag && iscntrl( c) && ((type[ c] & SPA) == 0) && (warn_level & 1))
            cwarn(
            "Illegal control character %.0s0lx%02x in quotation"    /* _W1_ */
                    , NULLST, (long) c, NULLST);
        *out_p++ = c;
chk_limit:
        if (out_end < out_p) {
            *out_end = EOS;
            cfatal( "Too long quotation", NULLST, 0L, NULLST);      /* _F_  */
        }
    }

    if (c == '\n' || c == EOS)
        unget();
    if (c == delim)
        *out_p++ = delim;
    *out_p = EOS;
    if (diag) {                         /* At translation phase 3   */
        skip = (infile->fp == NULL) ? NULLST : skip_line;
        if (c != delim) {
#if MODE == PRE_STANDARD
            if (mode == OLD_PREP        /* Implicit closing of quote*/
                    && (delim == '"' || delim == '\''))
                return  out_p;
            if (delim == '"') {                     /* mode == KR   */
                if (lang_asm)
#else   /* MODE == STANDARD */
            if (delim == '"') {
                if (mode == STD && lang_asm)
#endif
                /* Concatenate the unterminated string to the next line */
                {
                    if (warn_level & 1)
                        cwarn( unterm_string
                                , ", catenated to the next line"    /* _W1_ */
                                , 0L, NULLST);
                    if (cat_line( FALSE) != NULL)
                        goto  scan;         /* Splice the lines     */
                    /* Else end of file     */
                } else {
                    cerror( unterm_string, skip, 0L, NULLST);       /* _E_  */
                }
            } else if (delim == '\'') {
                cerror( unterm_char, out, 0L, skip);        /* _E_  */
            }
            else
                cerror( "Unterminated header name %s%.0ld%s"        /* _E_  */
                        , out, 0L, skip);
            out_p = NULL;
        } else if (delim == '\'' && out_p - out <= 2) {
            cerror( "Empty character constant %s%.0ld%s"    /* _E_  */
                    , out, 0L, skip);
            out_p = NULL;
        }
#if MODE == STANDARD
        else if (mode == POST_STD && delim == '>' && (warn_level & 2))
            cwarn(
        "Header-name enclosed by <, > is an obsolescent feature %s" /* _W2_ */
                    , out, 0L, skip);
#endif
#if MODE == STANDARD && NWORK-2 > SLEN90MIN
        if (out_p - out > str_len_min && (warn_level & 4))
            cwarn( "Quotation longer than %.0s%ld bytes"    /* _W4_ */
                    , NULLST, str_len_min, NULLST);
#endif
    }

    return  out_p;
}

static char *
#if PROTO
cat_line( int del_bsl)
#else
cat_line( del_bsl)
    int     del_bsl;        /* Delete the <backslash><newline> ?    */
#endif
/*
 * If del_bsl == TRUE:
 *     Delete <backslash><newline> sequence in string literal.
 * FALSE: Overwrite the <newline> with <backslash>'n'.
 * Return NULL on end of file.  Called only from scan_quote().
 * This routine is never called in POST_STD mode.
 */
{
    size_t  len;
    char *  save1, * save2;

    if (del_bsl) {          /* Delete the <backslash><newline>      */
        infile->bptr -= 2;
        len = infile->bptr - infile->buffer;
    } else {        /* Overwrite the <newline> with <backslash>'n'  */
        strcpy( infile->bptr, "\\n");
        len = strlen( infile->buffer);
    }
    save1 = save_string( infile->buffer);
    save2 = get_line( FALSE);   /* infile->buffer is overwritten    */
    if (save2 == NULL) {
        free( save1);
        return  NULL;
    }
    save2 = save_string( infile->buffer);
    memcpy( infile->buffer, save1, len);
    strcpy( infile->buffer + len, save2);               /* Catenate */
    free( save1);
    free( save2);
    if (! del_bsl)
        len -= 2;
    infile->bptr = infile->buffer + len;
    return  infile->bptr;
}

#if MODE == STANDARD                /* Standard conforming version  */

static char *
#if PROTO
scan_number( int c, char * out, char * out_end)
#else
scan_number(c, out, out_end)
    register int    c;                      /* First char of number */
    char *  out;                            /* Output buffer        */
    char *  out_end;                /* Limit of output buffer       */
#endif
/*
 * Read a preprocessing number.  We know that c is from 0 to 9 or dot, and if
 * c is dot then the next character is digit.
 * Returns the advanced output pointer.
 * Note: preprocessing number permits non-numeric forms such as 3E+xy,
 *   which are used in stringization or token-concatenation.
 */
{
    char *      out_p = out;        /* Current output pointer       */

    do {
        *out_p++ = c;
        if (c == 'E' || c == 'e'    /* Sign should follow 'E', 'e', */
                || (stdc3 && (c == 'P' || c == 'p'))
                                            /* 'P' or 'p'.          */
                ) {
            c = get();
            if (c == '+' || c == '-') {
                *out_p++ = c;
                c = get();
            }
#if OK_UCN
        } else if (mode == STD && c == '\\' && stdc3) {
            int     cnt;
            char *  tp;

            if ((c = get()) == 'u') {
                cnt = 4;
            } else if (c == 'U') {
                cnt = 8;
            } else {
                unget();
                out_p--;
                break;
            }
            *out_p++ = c;
            if ((tp = scan_ucn( cnt, out_p)) == NULL)      /* Error    */
                break;
            else
                out_p = tp;
            c = get();
#endif  /* OK_UCN   */
#if OK_MBIDENT
        } else if (mode == STD && (type[ c] & mbstart) && stdc3) {
            len = mb_read( c, &infile->bptr, &out_p);
            if (len & MB_ERROR) {
                if (infile->fp)
                    cerror(
                    "Illegal multi-byte character sequence."    /* _E_  */
                            , NULLST, 0L, NULLST);
            }
#endif  /* OK_MBIDENT   */
        } else {
            c = get();
        }
    } while ((type[ c] & (DIG | DOT | LET)) /* Digit, dot or letter */
#if MODE == STANDARD
#if OK_UCN
            || (mode == STD && c == '\\' && stdc3)
#endif
#if OK_MBIDENT
            || (mode == STD && (type[ c] & mbstart) && stdc3)
#endif
#endif  /* MODE == STANDARD */
        );

    *out_p = EOS;
    if (out_end < out_p)
        cfatal( "Too long pp-number token \"%s\""           /* _F_  */
                , out, 0L, NULLST);
    unget();
    return  out_p;
}

#else   /* MODE == PRE_STANDARD */
/* Original version of DECUS CPP, too exact for STANDARD preprocessing.     */

static char *
#if PROTO
scan_number( int c, char * out, char * out_end)
#else
scan_number(c, out, out_end)
    register int    c;                      /* First char of number */
    char *          out;                    /* Output buffer        */
    char *          out_end;        /* Limit of output buffer       */
#endif
/*
 * Process a number.  We know that c is from 0 to 9 or dot.
 * Algorithm from Dave Conroy's Decus C.
 * Returns the advanced output pointer.
 */
{
    char * const    out_s = out;            /* For diagnostics      */
    register int    radix;                  /* 8, 10, or 16         */
    int             expseen;                /* 'e' seen in floater  */
    int             octal89;                /* For bad octal test   */
    int             dotflag;                /* TRUE if '.' was seen */

    expseen = FALSE;                        /* No exponent seen yet */
    octal89 = FALSE;                        /* No bad octal yet     */
    radix = 10;                             /* Assume decimal       */
    if ((dotflag = (c == '.')) != FALSE) {  /* . something?         */
        *out++ = '.';                       /* Always out the dot   */
        if ((type[(c = get())] & DIG) == 0) {   /* If not a float numb, */
            goto  nomore;                   /* All done for now     */
        }
    }                                       /* End of float test    */
    else if (c == '0') {                    /* Octal or hex?        */
        *out++ = c;                         /* Stuff initial zero   */
        radix = 8;                          /* Assume it's octal    */
        c = get();                          /* Look for an 'x'      */
        if (c == 'x' || c == 'X') {         /* Did we get one?      */
            radix = 16;                     /* Remember new radix   */
            *out++ = c;                     /* Stuff the 'x'        */
            c = get();                      /* Get next character   */
        }
    }
    while (1) {                             /* Process curr. char.  */
        /*
         * Note that this algorithm accepts "012e4" and "03.4"
         * as legitimate floating-point numbers.
         */
        if (radix != 16 && (c == 'e' || c == 'E')) {
            if (expseen)                    /* Already saw 'E'?     */
                break;                      /* Exit loop, bad nbr.  */
            expseen = TRUE;                 /* Set exponent seen    */
            radix = 10;                     /* Decimal exponent     */
            *out++ = c;                     /* Output the 'e'       */
            if ((c = get()) != '+' && c != '-')
                continue;
        }
        else if (radix != 16 && c == '.') {
            if (dotflag)                    /* Saw dot already?     */
                break;                      /* Exit loop, two dots  */
            dotflag = TRUE;                 /* Remember the dot     */
            radix = 10;                     /* Decimal fraction     */
        }
        else {                              /* Check the digit      */
            switch (c) {
            case '8': case '9':             /* Sometimes wrong      */
                octal89 = TRUE;             /* Do check later       */
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
                break;                      /* Always ok            */

            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                if (radix == 16)            /* Alpha's are ok only  */
                    break;                  /* if reading hex.      */
            default:                        /* At number end        */
                goto done;                  /* Break from for loop  */
            }                               /* End of switch        */
        }                                   /* End general case     */
        *out++ = c;                         /* Accept the character */
        c = get();                          /* Read another char    */
    }                                       /* End of scan loop     */

    if (out_end < out)                      /* Buffer overflow      */
        goto  nomore;
    /*
     * When we break out of the scan loop, c contains the first
     * character (maybe) not in the number.  If the number is an
     * integer, allow a trailing 'L' for long.  for unsigned.  If not
     * those, push the trailing character back on the input stream.
     * Floating point numbers accept a trailing 'L' for "long double"
     * or a trailing 'F' for explicit float.
     */
done:
    if (! (dotflag || expseen)) {           /* Not floating point   */
        /*
         * We know that dotflag and expseen are both zero, now:
         *   dotflag signals "saw 'L'".
         * We assume that 12F is not a floating constant.
         */
        for (;;) {
            switch (c) {
            case 'l':
            case 'L':
                if (dotflag)
                    goto nomore;
                dotflag = TRUE;
                break;
            default:
                goto nomore;
            }
            *out++ = c;                     /* Got 'L' .            */
            c = get();                      /* Look at next, too.   */
        }
    }

nomore: *out = EOS;
    if (out_end < out)
        goto  overflow;
    unget();                                /* Not part of a number */
    if (octal89 && radix == 8 && (warn_level & 1))
        cwarn( "Illegal digit in octal number \"%s\""       /* _W1_ */
                , out_s, 0L, NULLST);
    return  out;

overflow:
    cfatal( "Too long number token \"%s\"", out_s, 0L, NULLST);     /* _F_  */
    return  out;
}
#endif  /* MODE == PRE_STANDARD */

#if MODE == STANDARD && OK_UCN
static char *
#if PROTO
scan_ucn( int cnt, char * out)
#else
scan_ucn( cnt, out)
    int     cnt;                            /* Bytes of sequence    */
    char *  out;                            /* Output buffer        */
#endif
/*
 * Scan an UCN sequence and put the sequence to 'out'.
 * Return the advanced pointer or NULL on failure.
 * This routine is never called in POST_STD mode.
 */
{
    uexpr_t value;                              /* Value of UCN     */
    int     i, c;

    value = 0L;
    for (i = 0; i < cnt; i++) {
        c = get();
        if (! isxdigit( c)) {
            if (infile->fp)
                cerror( "Illegal UCN sequence"              /* _E_  */
                        , NULLST, 0L, NULLST);
                *out = EOS;
                unget();
                return  NULL;
        }
        c = tolower( c);
        *out++ = c;
        c = (isdigit( c) ? (c - '0') : (c - 'a' + 10));
        value = (value << 4) | c;
    }
    if (infile->fp                              /* In source        */
            && ((value >= 0L && value <= 0x9FL
                && value != 0x24L && value != 0x40L && value != 0x60L)
                                    /* Basic source character       */
            || (stdc3 && (value >= 0xD800L && value <= 0xDFFFL))))
                                    /* Reserved for special chars   */
        cerror( "UCN cannot specify the value %.0s\"%08lx\""    /* _E_    */
                    , NULLST, (long) value, NULLST);
    return  out;
}
#endif  /* MODE == STANDARD && OK_UCN   */

static char *
#if PROTO
scan_op( int c, char * out)
#else
scan_op( c, out)
    register int    c;                  /* First char of the token  */
    char *  out;                        /* Output buffer            */
#endif
/*
 * Scan C operator or punctuator into the specified buffer.
 * Return the advanced output pointer.
 * The code-number of the operator is stored to global variable 'openum'.
 * Note: '#' is not an operator nor a punctuator in other than control line,
 *   nevertheless is handled as a punctuator in this cpp for convenience.
 */
{
    int     c2, c3;
#if MODE == STANDARD && OK_DIGRAPHS
    int     c4;
#endif

    *out++ = c;

    switch (c) {
    case '~':   openum = OP_COM;    break;
    case '(':   openum = OP_LPA;    break;
    case ')':   openum = OP_RPA;    break;
    case '?':   openum = OP_QUE;    break;
    case ';':    case '[':    case ']':    case '{':
    case '}':    case ',':
        openum = OP_1;
        break;
    default:
        openum = OP_2;                  /* Tentative guess          */
    }

    if (openum != OP_2) {               /* Single byte operators    */
        *out = EOS;
        return  out;
    }

    c2 = get();                         /* Possibly two bytes ops   */
    *out++ = c2;

    switch (c) {
    case '=':
        openum = ((c2 == '=') ? OP_EQ : OP_1);          /* ==, =    */
        break;
    case '!':
        openum = ((c2 == '=') ? OP_NE : OP_NOT);        /* !=, !    */
        break;
    case '&':
        switch (c2) {
        case '&':   openum = OP_ANA;        break;      /* &&       */
        case '=':   /* openum = OP_2; */    break;      /* &=       */
        default :   openum = OP_AND;        break;      /* &        */
        }
        break;
    case '|':
        switch (c2) {
        case '|':   openum = OP_ORO;        break;      /* ||       */
        case '=':   /* openum = OP_2; */    break;      /* |=       */
        default :   openum = OP_OR;         break;      /* |        */
        }
        break;
    case '<':
        switch (c2) {
        case '<':   c3 = get();
            if (c3 == '=') {
                openum = OP_3;                          /* <<=      */
                *out++ = c3;
            } else {
                openum = OP_SL;                         /* <<       */
                unget();
            }
            break;
        case '=':   openum = OP_LE;         break;      /* <=       */
#if MODE == STANDARD && OK_DIGRAPHS
        case ':':                                   /* <: i.e. [    */
            if (mode == STD && digraphs)
                openum = OP_LBRCK_D;
            else
                openum = OP_LT;
            break;
        case '%':                                   /* <% i.e. {    */
            if (mode == STD && digraphs)
                openum = OP_LBRACE_D;
            else
                openum = OP_LT;
            break;
#endif
        default :   openum = OP_LT;         break;      /* <        */
        }
        break;
    case '>':
        switch (c2) {
        case '>':   c3 = get();
            if (c3 == '=') {
                openum = OP_3;                          /* >>=      */
                *out++ = c3;
            } else {
                openum = OP_SR;                         /* >>       */
                unget();
            }
            break;
        case '=':   openum = OP_GE;     break;          /* >=       */
        default :   openum = OP_GT;     break;          /* >        */
        }
        break;
    case '#':
#if MODE == STANDARD
        if (in_define || macro_line)        /* in #define or macro  */
            openum = ((c2 == '#') ? OP_CAT : OP_STR);   /* ##, #    */
        else
#endif
            openum = OP_1;                              /* #        */
        break;
    case '+':
        switch (c2) {
        case '+':                                       /* ++       */
        case '=':   /* openum = OP_2; */    break;      /* +=       */
        default :   openum = OP_ADD;        break;      /* +        */
        }
        break;
    case '-':
        switch (c2) {
        case '-':                                       /* --       */
        case '=':                                       /* -=       */
            /* openum = OP_2;   */
            break;
        case '>':
#if MODE == STANDARD
            if (cplus) {
                if ((c3 = get()) == '*') {              /* ->*      */
                    openum = OP_3;
                    *out++ = c3;
                } else {
                    /* openum = OP_2;   */
                    unget();
                }
            }   /* else openum = OP_2;  */              /* ->       */
#else   /* MODE == PRE_STANDARD */
            /* openum = OP_2;   */
#endif
            break;
        default :   openum = OP_SUB;        break;      /* -        */
        }
        break;
    case '%':
        switch (c2) {
        case '=':                           break;      /* %=       */
#if MODE == STANDARD && OK_DIGRAPHS
        case '>':                                   /* %> i.e. }    */
            if (mode == STD && digraphs)
                openum = OP_RBRACE_D;
            else
                openum = OP_MOD;
            break;
        case ':':
            if (mode == STD && digraphs) {
                if ((c3 = get()) == '%') {
                    if ((c4 = get()) == ':') {      /* %:%: i.e. ## */
                        openum = OP_DSHARP_D;
                        *out++ = c3;
                        *out++ = c4;
                    } else {
                        unget();
                        unget();
                        openum = OP_SHARP_D;        /* %: i.e. #    */
                    }
                } else {
                    unget();
                    openum = OP_SHARP_D;            /* %: i.e. #    */
                }
                if (in_define) {                    /* in #define   */
                    if (openum == OP_DSHARP_D)
                        openum = OP_CAT;
                    else
                        openum = OP_STR;
                }
            } else {
                openum = OP_MOD;
            }
            break;
#endif  /* MODE == STANDARD && OK_DIGRAPHS  */
        default :   openum = OP_MOD;        break;      /* %        */
        }
        break;
    case '*':
        if (c2 != '=')                                  /* *        */
            openum = OP_MUL;
        /* else openum = OP_2;  */                      /* *=       */
        break;
    case '/':
        if (c2 != '=')                                  /* /        */
            openum = OP_DIV;
        /* else openum = OP_2;  */                      /* /=       */
        break;
    case '^':
        if (c2 != '=')                                  /* ^        */
            openum = OP_XOR;
        /* else openum = OP_2;  */                      /* ^=       */
        break;
    case '.':
#if MODE == STANDARD
        if (c2 == '.') {
            c3 = get();
            if (c3 == '.') {
                openum = OP_ELL;                        /* ...      */
                *out++ = c3;
                break;
            } else {
                unget();
                openum = OP_1;
            }
        }
        else if (cplus && c2 == '*')                    /* .*       */
            /* openum = OP_2    */  ;
        else                                            /* .        */
            openum = OP_1;
#else   /* MODE == PRE_STANDARD */
        openum = OP_1;
#endif
        break;
    case ':':
#if MODE == STANDARD
        if (cplus && c2 == ':')                         /* ::       */
            /* openum = OP_2    */  ;
#if OK_DIGRAPHS
        else if (mode == STD && c2 == '>' && digraphs)
            openum = OP_RBRCK_D;                    /* :> i.e. ]    */
#endif
        else                                            /* :        */
            openum = OP_COL;
#else   /* MODE == PRE_STANDARD */
        openum = OP_COL;
#endif
        break;
    default:                                        /* Who knows ?  */
#if DEBUG
        cfatal( "Bug: Punctuator is mis-implemented %.0s0lx%x"      /* _F_  */
                , NULLST, (long) c, NULLST);
#endif
        openum = OP_1;
        break;
    }

    switch (openum) {
#if MODE == STANDARD
    case OP_STR:
#if MODE == STANDARD && OK_DIGRAPHS
        if (mode == STD && c == '%')    break;              /* %:   */
#endif
#endif
    case OP_1:
    case OP_NOT:    case OP_AND:    case OP_OR:     case OP_LT:
    case OP_GT:     case OP_ADD:    case OP_SUB:    case OP_MOD:
    case OP_MUL:    case OP_DIV:    case OP_XOR:    case OP_COM:
    case OP_COL:
                        /* Any single byte operator or punctuator   */
        unget();
        out--;
        break;
    default:        /* Two or more bytes operators or punctuators   */
        break;
    }

    *out = EOS;
    return  out;
}

#if MODE == STANDARD
int
#if PROTO
id_operator( const char * name)
#else
id_operator( name)
    char *  name;
#endif
/*
 * Check whether the name is identifier-like operator in C++.
 * Return the operator number if matched, return 0 if not matched.
 * Note: these identifiers are defined as macros in <iso646.h> in C95.
 * This routine is never called in POST_STD mode.
 */
{
    typedef struct  id_op {
        const char *    name;
        int             op_num;
    } ID_OP;

    ID_OP   id_ops[] = {
        { "and",    OP_ANA},
        { "and_eq", OP_2},
        { "bitand", OP_AND},
        { "bitor",  OP_OR},
        { "compl",  OP_COM},
        { "not",    OP_NOT},
        { "not_eq", OP_NE},
        { "or",     OP_ORO},
        { "or_eq",  OP_2},
        { "xor",    OP_XOR},
        { "xor_eq", OP_2},
        { NULL,     0},
    };

    ID_OP *     id_p = id_ops;

    while (id_p->name != NULL) {
        if (str_eq( name, id_p->name))
            return  id_p->op_num;
        id_p++;
    }
    return  0;
}
#endif  /* MODE == STANDARD */

#if MODE == STANDARD
#undef  get
#undef  unget
#endif

/*
 *                      G E T
 */

int
#if PROTO
get( void)
#else
get()
#endif
/*
 * Return the next character from a macro or the current file.
 * Always return the value representable by unsigned char.
 */
{
#if MODE == PRE_STANDARD
    static int          squeezews = FALSE;
#endif
    int                     len;
    register int            c;
    register FILEINFO *     file;

    if ((file = infile) == NULL)
        return  CHAR_EOF;                   /* End of all input     */

#if MODE == STANDARD
    if (mode == POST_STD && file->fp) {     /* In a source file     */
        switch (insert_sep) {
        case NO_SEP:
            break;
        case INSERT_SEP:                /* Insert a token separator */
            insert_sep = INSERTED_SEP;      /* Remember this fact   */
            return  ' ';                    /*   for unget().       */
        case INSERTED_SEP:                  /* Has just inserted    */
            insert_sep = NO_SEP;            /* Clear the flag       */
            break;
        }
    }
#endif
#if MODE == PRE_STANDARD
    if (squeezews) {
        if (*file->bptr == ' ')
            file->bptr++;                   /* Squeeze white spaces */
        squeezews = FALSE;
    }
#endif

#if DEBUG
    if (debug & GETC) {
        fprintf( fp_debug, "get(%s), line %ld, bptr = %d, buffer"
        , file->fp ? cur_fullname : file->filename ? file->filename : "NULL"
            , line, (int) (file->bptr - file->buffer));
        dump_string( NULLST, file->buffer);
        dump_unget( "get entrance");
    }
#endif

    /*
     * Read a character from the current input logical line or macro.
     * At EOS, either finish the current macro (freeing temporary storage)
     * or get another logical line by parse_line().
     * At EOF, exit the current file (#include) or, at EOF from the MCPP input
     * file, return CHAR_EOF to finish processing.
     * The character is converted to int with no sign-extension.
     */
    if ((c = (*file->bptr++ & UCHARMAX)) != EOS) {
#if MODE == STANDARD
        return  c;                          /* Just a character     */
#else   /* MODE == PRE_STANDARD */
        if (! in_string && c == '\\' && *file->bptr == '\n'
                && in_define        /* '\\''\n' is deleted in #define line, */
#if BSL_IN_MBCHAR   /*   provided the '\\' is not the 2nd byte of mbchar.   */
                && ! last_is_mbchar( file->buffer, strlen( file->buffer) - 2)
#endif
            ) {
            if (*(file->bptr - 2) == ' ')
                squeezews = TRUE;
        } else {
            return  c;
        }
#endif  /* MODE == PRE_STANDARD */
    }

    /*
     * Nothing in current line or macro.  Get next line (if input from a
     * file), or do end of file/macro processing, and reenter get() to
     * restart from the top.
     */
    if (file->fp &&                         /* In source file       */
            parse_line() != NULL)           /* Get line from file   */
        return  get();
    /*
     * Free up space used by the (finished) file or macro and restart
     * input from the parent file/macro, if any.
     */
    infile = file->parent;                  /* Unwind file chain    */
    if (infile == NULL)                     /* If at end of input,  */
        return  CHAR_EOF;                   /*   return end of file.*/
    free( file->buffer);                    /* Free buffer          */
    if (file->fp) {                         /* Source file included */
        char *  cp;
        fclose( file->fp);                  /* Close finished file  */
        cp = stpcpy( cur_fullname, *(infile->dirp));
        strcpy( cp, infile->filename);
        cur_fname = infile->filename;       /* Restore current fname*/
        if (infile->pos != 0L) {            /* Includer is closed   */
            infile->fp = fopen( cur_fullname, "r");
            fseek( infile->fp, infile->pos, SEEK_SET);
        }   /* Re-open the includer and restore the file-position   */
        len = (int) (infile->bptr - infile->buffer);
        infile->buffer = xrealloc( infile->buffer, NBUFF);
            /* Restore full size buffer to get the next line        */
        infile->bptr = infile->buffer + len;
        line = infile->line;                /* Reset line number    */
        inc_dirp = infile->dirp;            /* Includer's directory */
        include_nest--;
        sharp();                            /* Need a #line now     */
    }
    free( file);                            /* Free file space      */
    return  get();                          /* Get from the parent  */
}

static char *
#if PROTO
parse_line( void)
#else
parse_line()
#endif
/*
 * ANSI (ISO) C: translation phase 3.
 * Parse a logical line.
 * Check illegal control characters.
 * Check unterminated string literal, character constant or comment.
 * Convert each comment to one space.
 * Squeeze succeding white spaces other than <newline> (including comments) to
 * one space.
 * The lines might be spliced by comments which cross the lines.
 */
{
    char *      temp;                       /* Temporary buffer     */
    char *      limit;                      /* Buffer end           */
    char *      tp;     /* Current pointer into temporary buffer    */
    char *      sp;                 /* Pointer into input buffer    */
    register int        c;

    if ((sp = get_line( FALSE)) == NULL)    /* Next logical line    */
        return  NULL;                       /* End of a file        */
#if MODE == PRE_STANDARD
    if (in_asm) {                           /* In #asm block        */
        while (type[ *sp++ & UCHARMAX] & SPA)
            ;
        if (*--sp == '#')                   /* Directive line       */
            infile->bptr = sp;
        return  infile->bptr;               /* Don't tokenize       */
    }
#endif
    tp = temp = xmalloc( (size_t) NBUFF);
    limit = temp + NBUFF - 2;

    while ((c = *sp++ & UCHARMAX) != '\n') {

        switch (c) {
        case '/':
            switch (*sp++) {
            case '*':                       /* Start of a comment   */
                if ((sp = read_a_comment( sp)) == NULL) {
                    free( temp);            /* End of file with un- */
                    return  NULL;           /*   terminated comment */
                }
#if MODE == STANDARD
                if (mode == POST_STD && (temp < tp && *(tp - 1) != ' '))
                    *tp++ = ' ';            /* Skip line top spaces */
                else if (mode == STD && (temp == tp || *(tp - 1) != ' '))
                    *tp++ = ' ';            /* Squeeze white spaces */
#else
                if (mode == OLD_PREP && (temp == tp
                        || (*(tp - 1) != ' ' && *(tp - 1) != COM_SEP)))
                    *tp++ = COM_SEP;        /* Convert to magic character   */
                else if (mode == KR && (temp == tp || *(tp - 1) != ' '))
                    *tp++ = ' ';            /* Squeeze white spaces */
#endif
                break;
#if MODE == STANDARD
            case '/':
                /* Comment when C++ or __STDC_VERSION__ >= 199901L      */
                /* Need not to convert to a space because '\n' follows  */
                if (! stdc2 && (warn_level & 2))
                    cwarn( "Parsed \"//\" as comment"       /* _W2_ */
                            , NULLST, 0L, NULLST);
                if (keep_comments) {
                    fputs( "/*", fp_out);   /* Convert to C style   */
                    while ((c = *sp++) != '\n')
                        fputc( c, fp_out);  /* Until the end of line*/
                    fputs( "*/", fp_out);
                }
                goto  end_line;
#endif
            default:                        /* Not a comment        */
                *tp++ = '/';
                sp--;                       /* To re-read           */
                break;
            }
            break;
        case '\r':                          /* Vertical white spaces*/
        case '\f':
#ifdef VT
        case VT:
#else
        case '\v':
#endif                                      /* Convert to a space   */
            if (warn_level & 4)
                cwarn( "Converted %.0s0x%02lx to a space"   /* _W4_ */
                    , NULLST, (long) c, NULLST);
        case '\t':                          /* Horizontal space     */
        case ' ':
#if MODE == STANDARD
            if (mode == POST_STD && temp < tp && *(tp - 1) != ' ')
                *tp++ = ' ';                /* Skip line top spaces */
            else if (mode == STD && (temp == tp || *(tp - 1) != ' '))
                *tp++ = ' ';                /* Squeeze white spaces */
#else
            if (mode == OLD_PREP && temp < tp && *(tp - 1) == COM_SEP)
                *(tp - 1) = ' ';    /* Squeeze COM_SEP with spaces  */
            else if (temp == tp || *(tp - 1) != ' ')
                *tp++ = ' ';                /* Squeeze white spaces */
#endif
            break;
        case '"':                           /* String literal       */
        case '\'':                          /* Character constant   */
            infile->bptr = sp;
#if MODE == STANDARD
            tp = scan_quote( c, tp, limit, TRUE);
#else
            in_string = TRUE;       /* Enable line splicing by scan_quote() */
            tp = scan_quote( c, tp, limit, TRUE);   /*   (not by get()).    */
            in_string = FALSE;
#endif
            if (tp == NULL) {
                free( temp);                /* Unbalanced quotation */
                return  parse_line();       /* Skip the line        */
            }
            sp = infile->bptr;
            break;
        default:
            if (iscntrl( c)) {
                cerror(             /* Skip the control character   */
    "Illegal control character %.0s0x%lx, skipped the character"    /* _E_  */
                        , NULLST, (long) c, NULLST);
            } else {                        /* Any valid character  */
                *tp++ = c;
            }
            break;
        }

        if (limit < tp) {
            *tp = EOS;
            cfatal( "Too long line spliced by comments"     /* _F_  */
                    , NULLST, 0L, NULLST);
        }
    }

#if MODE == STANDARD
end_line:
#endif
    if (temp < tp && *(tp - 1) == ' ')
        tp--;                       /* Remove trailing white space  */
    *tp++ = '\n';
    *tp = EOS;
    infile->bptr = strcpy( infile->buffer, temp);   /* Write back to buffer */
    free( temp);
    if (macro_line != 0 && macro_line != MACRO_ERROR) { /* Expanding macro  */
        temp = infile->buffer;
        if (*temp == ' ')
            temp++;
        if (*temp == '#'
#if MODE == STANDARD
                    || (*temp == '%' && *(temp + 1) == ':')
#endif
        )
            if (warn_level & 1)
                cwarn(
    "Macro started at line %.0s%ld swallowed directive-like line"   /* _W1_ */
                    , NULLST, macro_line, NULLST);
    }
    return  infile->buffer;
}

static char *
#if PROTO
read_a_comment( char * sp)
#else
read_a_comment( sp)
    char *      sp;
#endif
/*
 * Read over a comment (which may cross the lines).
 */
{
    register int        c;

    if (keep_comments)                      /* If writing comments  */
        fputs( "/*", fp_out);               /* Write the initializer*/
    c = *sp++;

    while (1) {                             /* Eat a comment        */
        if (keep_comments)
            fputc( c, fp_out);

        switch (c) {
        case '/':
            if ((c = *sp++) != '*')         /* Don't let comments   */
                continue;                   /*   nest.              */
            if (warn_level & 1)
                cwarn( "\"/*\" within comment", NULLST, 0L, NULLST);/* _W1_ */
            if (keep_comments)
                fputc( c, fp_out);
                                            /* Fall into * stuff    */
        case '*':
            if ((c = *sp++) != '/')         /* If comment doesn't   */
                continue;                   /*   end, look at next. */
            if (keep_comments) {            /* Put out comment      */
                fputc( c, fp_out);          /*   terminator, too.   */
                fputc( '\n', fp_out);       /* Newline to avoid mess*/
            }
            return  sp;                     /* End of comment       */
        case '\n':
            if (! keep_comments)            /* We'll need a #line   */
                wrong_line = TRUE;          /*   later...           */
            if ((sp = get_line( TRUE)) == NULL) /* End of file      */
                return  NULL;               /*   within comment     */
            break;
        default:                            /* Anything else is     */
            break;                          /*   just a character   */
        }                                   /* End switch           */

        c = *sp++;
    }                                       /* End comment loop     */

    return  sp;                             /* Never reach here     */
}

static char *
#if PROTO
get_line( int in_comment)
#else
get_line( in_comment)
    int     in_comment;
#endif
/*
 * ANSI (ISO) C: translation phase 1, 2.
 * Get the next logical line from source file.
 */
{
#if MODE == STANDARD && (OK_TRIGRAPHS || OK_DIGRAPHS)
    int     converted = FALSE;
#endif
#if MODE == STANDARD
    int     esc;                            /* Line ends with \     */
#endif
    int     len;                            /* Line length - alpha  */
    char *  ptr;

    if (infile == NULL)                     /* End of a source file */
        return  NULL;
    ptr = infile->bptr = infile->buffer;

    while (fgets( ptr, (int) (infile->buffer + NBUFF - ptr), infile->fp)
            != NULL) {
        /* Translation phase 1  */
        line++;                     /* Gotten next physical line    */
#if MODE == STANDARD
        if (line == line_limit + 1 && (warn_level & 1))
            cwarn( "Line number %.0s\"%ld\" got beyond range"       /* _W1_ */
                    , NULLST, line, NULLST);
#endif
#if DEBUG
        if (debug & (TOKEN | GETC)) {       /* Dump it to fp_debug  */
            fprintf( fp_debug, "\n#line %ld (%s)", line, cur_fullname);
            dump_string( NULLST, ptr);
        }
#endif
        len = strlen( ptr);
        if (NBUFF - 1 <= ptr - infile->buffer + len
                && *(ptr + len - 1) != '\n') {
            if (NBUFF - 1 <= len)
                cfatal( "Too long source line"              /* _F_  */
                        , NULLST, 0L, NULLST);
            else
                cfatal( "Too long logical line"             /* _F_  */
                        , NULLST, 0L, NULLST);
        }
        if (*(ptr + len - 1) != '\n')   /* Unterminated source line */
            break;
#if MODE == STANDARD && OK_TRIGRAPHS
        if (tflag)
            converted = cnv_trigraph( ptr);
#endif
#if MODE == STANDARD && OK_DIGRAPHS
        if (mode == POST_STD && digraphs)
            converted += cnv_digraph( ptr);
#endif
#if MODE == STANDARD && (OK_TRIGRAPHS || OK_DIGRAPHS)
        if (converted)
            len = strlen( ptr);
#endif
#if MODE == STANDARD
        /* Translation phase 2  */
        len -= 2;
        if (len >= 0) {
            esc = (*(ptr + len) == '\\');
#if BSL_IN_MBCHAR
            esc = esc && ! last_is_mbchar( ptr, len);   /* MBCHAR ? */
#endif
            if (esc) {                      /* <backslash><newline> */
                ptr = infile->bptr += len;  /* Splice the lines     */
                wrong_line = TRUE;
                continue;
            }
        }
#endif  /* MODE == STANDARD */
#if MODE == STANDARD && NBUFF-2 > SLEN90MIN
        if (ptr - infile->buffer + len + 2 > str_len_min + 1
                && (warn_level & 4))        /* +1 for '\n'          */
            cwarn( "Logical source line longer than %.0s%ld bytes"  /* _W4_ */
                    , NULLST, str_len_min, NULLST);
#endif
        return  infile->bptr = infile->buffer;      /* Logical line */
    }

    /* End of a (possibly included) source file */
    if (ferror( infile->fp))
        cfatal( "File read error", NULLST, 0L, NULLST);     /* _F_  */
    at_eof( in_comment);                    /* Check at end of file */
    if (zflag) {
        no_output--;                        /* End of included file */
        keep_comments = cflag && compiling && !no_output;
    }
    return  NULL;
}

#if MODE == STANDARD && OK_TRIGRAPHS

#define TRIOFFSET       10

int
#if PROTO
cnv_trigraph( char * in)
#else
cnv_trigraph( in)
    register char *     in;
#endif
/*
 * Perform in-place trigraph replacement on a physical line.  This was added
 * to the C90.  In an input text line, the sequence ??[something] is
 * transformed to a character (which might not appear on the input keyboard).
 */
{
    const char * const  tritext = "=(/)'<!>-\0#[\\]^{|}~";
    /*                             ^          ^
     *                             +----------+
     *                             this becomes this
     */
    int     count = 0;
    register const char *   tp;

    while ((in = strchr( in, '?')) != NULL) {
        if (*++in != '?')
            continue;
        while (*++in == '?')
            ;
        if ((tp = strchr( tritext, *in)) == NULL)
            continue;
        in[ -2] = tp[ TRIOFFSET];
        in--;
        memmove( in, in + 2, strlen( in + 1));
        count++;
    }

    if (count && (warn_level & 16))
        cwarn( "%.0s%ld trigraph(s) converted"          /* _W16_    */
                , NULLST, (long) count, NULLST);
    return  count;
}

#endif  /* MODE == STANDARD && OK_TRIGRAPHS */

#if MODE == STANDARD && OK_DIGRAPHS

int
#if PROTO
cnv_digraph( char * in)
#else
cnv_digraph( in)
    register char *     in;
#endif
/*
 * Perform in-place digraph replacement on a physical line.
 * Called only in POST_STD mode.
 */
{
    int     count = 0;
    int     i;
    int     c1, c2;

    while ((i = strcspn( in, "%:<")), (c1 = in[ i]) != '\0') {
        in += i + 1;
        c2 = *in;
        switch (c1) {
        case '%'    :
            switch (c2) {
            case ':'    :   *(in - 1) = '#';    break;
            case '>'    :   *(in - 1) = '}';    break;
            default     :   continue;
            }
            break;
        case ':'    :
            switch (c2) {
            case '>'    :   *(in - 1) = ']';    break;
            default     :   continue;
            }
            break;
        case '<'    :
            switch (c2) {
            case '%'    :   *(in - 1) = '{';    break;
            case ':'    :   *(in - 1) = '[';    break;
            default     :   continue;
            }
            break;
        }
        memmove( in, in + 1, strlen( in));
        count++;
    }

    if (count && (warn_level & 16))
        cwarn( "%.0s%ld digraph(s) converted"           /* _W16_    */
                , NULLST, (long) count, NULLST);
    return  count;
}

#endif  /* MODE == STANDARD && OK_DIGRAPHS */

#if BSL_IN_MBCHAR
static int
#if PROTO
last_is_mbchar( const char * in, int len)
#else
last_is_mbchar( in, len)
    char *  in;                     /* Input physical line          */
    int     len;                    /* Length of the line minus 2   */
#endif
/*
 * Return 2, if the last char of the line is second byte of SJIS or BIGFIVE,
 * else return 0.
 */
{
    register const char *   cp = in + len;
    const char * const      endp = in + len;    /* -> the char befor '\n'   */

    if ((mbchar & (SJIS | BIGFIVE)) == 0)
        return  0;
    while (in <= --cp) {                    /* Search backwardly    */
        if ((type[ *cp & UCHARMAX] & mbstart) == 0)
            break;                  /* Not the first byte of MBCHAR */
    }
    if ((endp - cp) & 1)
        return  0;
    else
        return  2;
}
#endif  /* BSL_IN_MBCHAR    */

static void
#if PROTO
at_eof( int in_comment)
#else
at_eof( in_comment)
    int     in_comment;
#endif
/*
 * Check the partial line, unterminated comment, unbalanced #if block,
 * uncompleted macro call at end of file or at end of input.
 */
{
    register const char * const     format
            = "End of %s with %.0ld%s";                 /* _E_ _W1_ */
    const char * const  unterm_if_format
= "End of %s within #if (#ifdef) section started at line %ld";  /* _E_ _W1_ */
    const char * const  unterm_macro_format
            = "End of %s within macro call started at line %ld";/* _E_ _W1_ */
    const char * const  input
            = infile->parent ? "file" : "input";        /* _E_ _W1_ */
    const char * const  no_newline
            = "no newline, skipped the line";           /* _E_ _W1_ */
    const char * const  unterm_com
            = "unterminated comment, skipped the line";         /* _E_ _W1_ */
#if MODE == STANDARD
    const char * const  backsl = "\\, skipped the line";        /* _E_ _W1_ */
#else
    const char * const  unterm_asm_format
= "End of %s with unterminated #asm block started at line %ld"; /* _E_ _W1_ */
#endif
    size_t  len;
    char *  cp = infile->buffer;
    IFINFO *    ifp;

    len = strlen( cp);
    if (len && *(cp += (len - 1)) != '\n') {
        line++;
        *++cp = '\n';                   /* For diagnostic message   */
        *++cp = EOS;
#if MODE == STANDARD
        cerror( format, input, 0L, no_newline);
#else
        if (mode != OLD_PREP && (warn_level & 1))
            cwarn( format, input, 0L, no_newline);
#endif
    }

#if MODE == STANDARD
    if (infile->buffer < infile->bptr)
        cerror( format, input, 0L, backsl);
#endif

    if (in_comment)
#if MODE == STANDARD
        cerror( format, input, 0L, unterm_com);
#else
        if (mode != OLD_PREP && (warn_level & 1))
            cwarn( format, input, 0L, no_newline);
#endif

    if (infile->initif < ifptr) {
        ifp = infile->initif + 1;
#if MODE == STANDARD
        cerror( unterm_if_format, input, ifp->ifline, NULLST);
        ifptr = infile->initif;             /* Clear information of */
        compiling = ifptr->stat;            /*   erroneous grouping */
#else   /* MODE == PRE_STANDARD */
        if (mode != OLD_PREP && (warn_level & 1))
            cwarn( unterm_if_format, input, ifp->ifline, NULLST);
#endif
    }

    if (macro_line != 0 && macro_line != MACRO_ERROR
#if MODE == STANDARD
                && mode == STD && in_getarg
#endif
            ) {
#if MODE == STANDARD
        cerror( unterm_macro_format, input, macro_line, NULLST);
        macro_line = MACRO_ERROR;
#else
        if (warn_level & 1)
            cwarn( unterm_macro_format, input, macro_line, NULLST);
#endif
    }

#if MODE == PRE_STANDARD
    if (in_asm) {
        if (mode != OLD_PREP && (warn_level & 1))
            cwarn( unterm_asm_format, input, in_asm, NULLST);
    }
#endif
}

void
#if PROTO
unget( void)
#else
unget()
#endif
/*
 * Backup the pointer to reread the last character.  Fatal error (code bug)
 * if we backup too far.  unget() may be called, without problems, at end of
 * file.  Only one character may be ungotten.  If you need to unget more,
 * call unget_string().
 */
{
    if (infile != NULL) {
#if MODE == STANDARD
        if (mode == POST_STD && infile->fp) {
            switch (insert_sep) {
            case INSERTED_SEP:  /* Have just read an inserted separator */
                insert_sep = INSERT_SEP;
                return;
#if DEBUG
            case INSERT_SEP:
                cfatal( "Bug: unget() just after scan_token()"       /* _F_  */
                        , NULLST, 0L, NULLST);
                break;
#endif
            default:
                break;
            }
        }
#endif  /* MODE == STANDARD */
        --infile->bptr;
#if DEBUG
        if (infile->bptr < infile->buffer)      /* Shouldn't happen */
            cfatal( "Bug: Too much pushback", NULLST, 0L, NULLST);  /* _F_  */
#endif
    }

#if DEBUG
    if (debug & GETC)
        dump_unget( "after unget");
#endif
}

FILEINFO *
#if PROTO
unget_string( const char * text, const char * name)
#else
unget_string( text, name)
    char *      text;                   /* Text to unget            */
    char *      name;                   /* Name of the macro        */
#endif
/*
 * Push a string back on the input stream.  This is done by treating
 * the text as if it were a macro.
 */
{
    register FILEINFO *     file;
    size_t                  size;

    if (text)
        size = strlen( text) + 1;
    else
        size = 1;
    file = get_file( name, size);
    if (text)
        memcpy( file->buffer, text, size);
    else
        *file->buffer = EOS;
    return  file;
}

char *
#if PROTO
save_string( const char * text)
#else
save_string( text)
    char *      text;
#endif
/*
 * Store a string into free memory.
 */
{
    register char *     result;
    size_t              size;

    size = strlen( text) + 1;
    result = xmalloc( size);
    memcpy( result, text, size);
    return  result;
}

FILEINFO *
#if PROTO
get_file( const char * name, size_t bufsize)
#else
get_file( name, bufsize)
    char *      name;               /* File or macro name string    */
    size_t      bufsize;            /* Line buffer size             */
#endif
/*
 * Common FILEINFO buffer initialization for a new file or macro.
 */
{
    register FILEINFO *     file;

    file = (FILEINFO *) xmalloc( sizeof (FILEINFO));
    file->buffer = xmalloc( bufsize);
    file->bptr = file->buffer;              /* Initialize line ptr  */
    file->buffer[ 0] = EOS;                 /* Force first read     */
    file->line = 0L;                        /* (Not used just yet)  */
    file->fp = NULL;                        /* No file yet          */
    file->pos = 0L;                         /* No pos to remember   */
    file->parent = infile;                  /* Chain files together */
    file->initif = ifptr;                   /* Initial ifstack      */
    file->dirp = NULL;                      /* No sys-header yet    */
    file->filename = name;                  /* Save file/macro name */

    if (infile != NULL)                     /* If #include file     */
        infile->line = line;                /* Save current line    */
    infile = file;                          /* New current file     */
    return  file;                           /* All done.            */
}

static const char * const   out_of_memory
    = "Out of memory (required size is %.0s0x%lx bytes)";   /* _F_  */

char *
#if PROTO
(xmalloc)( size_t size)     /* Parenthesis to avoid macro expansion */
#else
(xmalloc)( size)
    size_t      size;
#endif
/*
 * Get a block of free memory.
 */
{
    char *      result;

    if ((result = (char *) malloc( size)) == NULL) {
#if DEBUG
        if (debug & MEMORY)
            print_heap();
#endif
       cfatal( out_of_memory, NULLST, (long) size, NULLST);
    }
    return  result;
}

char *
#if PROTO
(xrealloc)( char * ptr, size_t size)
#else
(xrealloc)( ptr, size)      /* Parenthesis to avoid macro expansion */
    char *      ptr;
    size_t      size;
#endif
/*
 * Reallocate malloc()ed memory.
 */
{
    char *      result;

    if ((result = (char *) realloc( ptr, size)) == NULL && size != 0) {
        /* 'size != 0' is necessary to cope with some               */
        /*   implementation of realloc( ptr, 0) which returns NULL. */
#if DEBUG
        if (debug & MEMORY)
            print_heap();
#endif
        cfatal( out_of_memory, NULLST, (long) size, NULLST);
    }
    return  result;
}

#if MODE == PRE_STANDARD

static void
#if PROTO
put_line( char * out, FILE * fp)
#else
put_line( out, fp)
    char *  out;
    FILE *  fp;
#endif
/*
 * Put out a logical source line.
 * This routine is called only in OLD_PREP mode.
 */
{
    register int    c;

    while ((c = *out++) != EOS) {
        if (c != COM_SEP)           /* Skip 0-length comment        */
            fputc( c, fp);
    }
}

#endif  /* MODE == PRE_STANDARD */

static void
#if PROTO
do_msg( const char * severity, const char * format, const char * arg1
        , long arg2, const char * arg3)
#else
do_msg( severity, format, arg1, arg2, arg3)
    char *  severity;               /* "fatal", "error", "warning"  */
    char *  format;                 /* Format for the error message */
    char *  arg1;                   /* String arg. for the message  */
    long    arg2;                   /* Integer argument             */
    char *  arg3;                   /* Second string argument       */
#endif
/*
 * Print filenames, macro names, line numbers and error messages.
 */
{
    FILEINFO *  file;
    int         i;
    size_t      slen;
    const char *    arg_s[ 2];
    char *      arg_t[ 2];
    char *      tp;
    const char *    sp;
    register int    c;

    fflush( fp_out);                /* Synchronize output and diagnostics   */
    arg_s[ 0] = arg1;  arg_s[ 1] = arg3;

    for (i = 0; i < 2; i++) {   /* Convert special characters to visible    */
        sp = arg_s[ i];
        if (sp != NULL)
            slen = strlen( sp) + 1;
        else
            slen = 1;
        tp = arg_t[ i] = (char *) malloc( slen);
            /* Don't use xmalloc() so as not to cause infinite recursion    */
        if (sp == NULL || *sp == EOS) {
            *tp = EOS;
            continue;
        }

        while ((c = *sp++) != EOS) {
            switch (c) {
#if MODE == STANDARD
            case RT_END:
            case TOK_SEP:
            case IN_SRC:
                if (mode == POST_STD) {
                    *tp++ = c;
                    break;
                }                       /* Else fall through        */
            case CAT:
            case ST_QUOTE:
            case DEF_MAGIC:
                break;
#else
            case COM_SEP:
                if (mode == OLD_PREP)
                    break;              /* Skip magic characters    */
                /* Else illegal control character, convert to a space       */
#endif
            case '\n':
                *tp++ = ' ';            /* Convert '\n' to a space  */
                break;
            default:
                *tp++ = c;
                break;
            }
        }

        if (*(sp - 2) == '\n')
            tp--;
        *tp = EOS;
    }

    file = infile;
    while (file != NULL && (file->fp == NULL || file->fp == (FILE *)-1))
        file = file->parent;                        /* Skip macro   */
    if (file != NULL) {
        file->line = line;
        fprintf( fp_err, "%s:%ld: %s: ", cur_fullname, line, severity);
    }
    fprintf( fp_err, format, arg_t[ 0], arg2, arg_t[ 1]);
    fputc( '\n', fp_err);
    if (no_source_line)
        goto  free_arg;

    file = infile;
    if (file != NULL && file->fp != NULL) {
#if MODE == STANDARD
        fprintf( fp_err, "    %s", file->buffer);   /* Current source line  */
#else
        if (mode == OLD_PREP) {
            fputs( "    ", fp_err);
            put_line( file->buffer, fp_err);
        } else {
            fprintf( fp_err, "    %s", file->buffer);
        }
#endif
        file = file->parent;
    }

    while (file != NULL) {                  /* Print #includes, too */
        if (file->fp == NULL) {             /* Macro                */
            if (file->filename) {
                DEFBUF *    defp;
                defp = look_id( file->filename);
                dump_a_def( "    in macro", defp, FALSE, FALSE, TRUE, fp_err);
            }
        } else {                            /* Source file          */
            if (file->buffer[ 0] == '\0')
                strcpy( file->buffer, "\n");
#if MODE == STANDARD
            fprintf( fp_err, "    from %s%s: %ld:    %s",
                *(file->dirp),              /* Include directory    */
                file->filename,             /* Current file name    */
                file->line,                 /* Current line number  */
                file->buffer);              /* The source line      */
#else
            if (mode == OLD_PREP) {
                fprintf( fp_err, "    from %s%s: %ld:    ",
                    *(file->dirp), file->filename, file->line);
                put_line( file->buffer, fp_err);
            } else {
                fprintf( fp_err, "    from %s%s: %ld:    %s",
                    *(file->dirp), file->filename, file->line, file->buffer);
            }
#endif
        }
        file = file->parent;
    }
free_arg:
    for (i = 0; i < 2; i++)
        free( arg_t[ i]);
}

void
#if PROTO
cfatal( const char * format, const char * arg1, long arg2, const char * arg3)
#else
cfatal( format, arg1, arg2, arg3)
    char *  format;
    char *  arg1;
    long    arg2;
    char *  arg3;
#endif
/*
 * A real disaster.
 */
{
    do_msg( "fatal error", format, arg1, arg2, arg3);
    exit( IO_ERROR);
}

void
#if PROTO
cerror( const char * format, const char * arg1, long arg2, const char * arg3)
#else
cerror( format, arg1, arg2, arg3)
    char *  format;
    char *  arg1;
    long    arg2;
    char *  arg3;
#endif
/*
 * Print a error message.
 */
{
    do_msg( "error", format, arg1, arg2, arg3);
    errors++;
}

void
#if PROTO
cwarn( const char * format, const char * arg1, long arg2, const char * arg3)
#else
cwarn( format, arg1, arg2, arg3)
    char *      format;
    char *      arg1;
    long        arg2;
    char *      arg3;
#endif
/*
 * Maybe an error.
 */
{
    do_msg( "warning", format, arg1, arg2, arg3);
}

#if DEBUG

void
#if PROTO
dump_string( const char * why, const char * text)
#else
dump_string( why, text)
    char *          why;
    char *          text;
#endif
/*
 * Dump text readably.
 * Bug: macro argument number may be putout as a control character or any
 * other character, just after MAC_PARM has been read away.
 */
{
    register const char *   cp;
    const char *    chr;
    register int    c;

    if (why != NULL)
        fprintf( fp_debug, " (%s)", why);
    fputs( " => ", fp_debug);

    if (text == NULL) {
        fputs( "NULL", fp_debug);
        return;
    }

    for (cp = text; (c = *cp++ & UCHARMAX) != EOS; ) {
        chr = NULL;

        switch (c) {
        case MAC_PARM:
            c = *cp++ & UCHARMAX;
            fprintf( fp_debug, "<%d>", c);
            break;
#if MODE == STANDARD
        case DEF_MAGIC:
            chr = "<MAGIC>";
            break;
        case CAT:
            chr = "##";
            break;
        case ST_QUOTE:
            chr = "#";
            break;
        default:
            if (mode == STD) {
                switch( c) {
                case TOK_SEP:
                    chr = "<TSEP>";
                    break;
                case RT_END:
                    chr = "<RT_END>";
                    break;
                case IN_SRC:
                    chr = "<SRC>";
                    break;
                }
            }
            if (chr)
                break;
            if (c < ' ')
                fprintf( fp_debug, "<^%c>", c + '@');
            else
                fputc( c, fp_debug);
            break;
        }

        if (chr)
            fputs( chr, fp_debug);
#else
        case COM_SEP:
            if (mode == OLD_PREP) {
                chr = "<CSEP>";
                break;
            }       /* Else fall through    */
        default:
            if (c < ' ')
                fprintf( fp_debug, "<^%c>", c + '@');
            else
                fputc( c, fp_debug);
            break;
        }

        if (chr)
            fputs( chr, fp_debug);
#endif
    }

    fputc( '\n', fp_debug);
}

void
#if PROTO
dump_unget( const char * why)
#else
dump_unget( why)
    char *      why;
#endif
/*
 * Dump all ungotten junk (pending macros and current input lines).
 */
{
    register const FILEINFO *   file;

    fputs( "dump of pending input text", fp_debug);
    if (why != NULL) {
        fputs( "-- ", fp_debug);
        fputs( why, fp_debug);
    }
    fputc( '\n', fp_debug);

    for (file = infile; file != NULL; file = file->parent)
        dump_string( file->filename ? file->filename : "NULL", file->bptr);
}

static void
#if PROTO
dump_token( int token_type, const char * cp)
#else
dump_token( token_type, cp)
    int     token_type;
    char *  cp;                                     /* Token        */
#endif
/*
 * Dump a token.
 */
{
    static const char * const   t_type[]
            = { "NAM", "NUM", "STR", "WSTR", "CHR", "WCHR", "OPE", "SPE"
            , "SEP", };

    fputs( "token", fp_debug);
    dump_string( t_type[ token_type - NAM], cp);
}

#endif  /* DEBUG    */

