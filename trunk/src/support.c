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
 *                          S U P P O R T . C
 *                  S u p p o r t   R o u t i n e s
 *
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

static void     scan_id( int c);
                /* Scan an identifier           */
static char *   scan_number( int c, char * out, char * out_end);
                /* Scan a preprocessing number  */
static char *   scan_number_prestd( int c, char * out, char * out_end);
                /* scan_number() for pre-Standard       */
#if OK_UCN
static char *   scan_ucn( int cnt, char * out);
                /* Scan an UCN sequence         */
#endif
static char *   scan_op( int c, char * out);
                /* Scan an operator or punctuat.*/
static char *   parse_line( void);
                /* Parse a logical line         */
static char *   read_a_comment( char * sp);
                /* Read over a comment          */
static char *   get_line( int in_comment);
                /* Get a logical line from file */
static void     at_eof( int in_comment);
                /* Check erroneous end of file  */
static void     do_msg( const char * severity, const char * format
        , const char * arg1, long arg2, const char * arg3);
                /* Putout diagnostic message    */
static char *   cat_line( int del_bsl);
                /* Splice the line              */
static int      last_is_mbchar( const char * in, int len);
                /* The line ends with MBCHAR ?  */
static void     put_line( char * out, FILE * fp);
                /* Put out a logical line       */
static void     dump_token( int token_type, const char * cp);
                /* Dump a token and its type    */

#define EXP_MAC_IND_MAX     8
/* Information of current expanding macros for diagnostic   */
static const char * expanding_macro[ EXP_MAC_IND_MAX];

static int  in_token = FALSE;       /* For token scanning functions */
static int  in_string = FALSE;      /* For get() and parse_line()   */


#if MCPP_LIB
void    init_support( void)
{
    in_token = FALSE;
    in_string = FALSE;
}
#endif

int     get_unexpandable(
    int     c,                              /* First of token       */
    int     diag                            /* Flag of diagnosis    */
)
/*
 * Get the next unexpandable token in the line, expanding macros.
 * Return the token type.  The token is written in work[].
 * The once expanded macro is never expanded again.
 * Called only from the routines processing #if (#elif, #assert), #line and
 * #include directives in order to diagnose some subtle macro expansions.
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
            && fp != NULL                       /* In source !      */
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
        if (standard && str_eq( identifier, "defined") && (warn_level & 1))
            cwarn( "Macro \"%s\" is expanded to \"defined\""        /* _W1_ */
                    , defp->name, 0L, NULLST);
        if (! standard && str_eq( identifier, "sizeof") && (warn_level & 1))
            cwarn( "Macro \"%s\" is expanded to \"sizeof\""         /* _W1_ */
                    , defp->name, 0L, NULLST);
    }
    return  token_type;
}

void    skip_nl( void)
/*
 * Skip to the end of the current input line.
 */
{
    insert_sep = NO_SEP;
    while (infile && infile->fp == NULL) {  /* Stacked text         */
        infile->bptr += strlen( infile->bptr);
        get();                              /* To the parent        */
    }
    if (infile)
        infile->bptr += strlen( infile->bptr);  /* Source line      */
}

int     skip_ws( void)
/*
 * Skip over whitespaces other than <newline>.
 * Note: POST_STD mode does not use TOK_SEP, and KR mode does not use COM_SEP.
 */
{
    int     c;

    do {
        c = get();
    }
    while (c == ' ' || c == TOK_SEP);
                                /* COM_SEP is an alias of TOK_SEP   */
    return  c;
}

int     scan_token(
    int     c,                  /* The first character of the token */
    char ** out_pp,             /* Pointer to pointer to output buf */
    char *  out_end             /* End of output buffer             */
)
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
    char *  out = *out_pp;              /* Output pointer           */
    int     ch_type;                    /* Type of character        */
    int     token_type = 0;             /* Type of token            */
    int     ch;

    if (standard)
        in_token = TRUE;                /* While a token is scanned */
    ch_type = type[ c & UCHARMAX] & mbmask;
    c = c & UCHARMAX;

    switch (ch_type) {
    case LET:                           /* An identifier            */
        switch (c) {
        case 'L':
            if (! standard)
                goto  ident;
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
        default:
ident:
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
        out = (standard ? scan_number( c, out, out_end)
                : scan_number_prestd( c, out, out_end));
        token_type = NUM;
        break;
    case PUNC:
operat: out = scan_op( c, out);         /* Operator or punctuator   */
        token_type = OPE;       /* Number is set in global "openum" */
        break;
    default:
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
        if ((standard && (c == CAT || c == ST_QUOTE)) || (type[ c] & SPA))
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
    if (debug & TOKEN)
        dump_token( token_type, *out_pp);
    if (mode == POST_STD && token_type != SEP && infile->fp != NULL
            && (type[ *infile->bptr & UCHARMAX] & SPA) == 0)
        insert_sep = INSERT_SEP;    /* Insert token separator       */
    *out_pp = out;

    in_token = FALSE;               /* Token scanning has been done */
    return  token_type;
}

static void scan_id(
    int     c                               /* First char of id     */
)
/*
 * Reads the next identifier and put it into identifier[].
 * The caller has already read the first character of the identifier.
 */
{
#if DOLLAR_IN_NAME
    static int      diagnosed = FALSE;  /* Flag of diagnosing '$'   */
#endif
    static char * const     limit = &identifier[ IDMAX];
#if OK_UCN
    int     uc2 = 0, uc4 = 0;           /* Count of UCN16, UCN32    */
#endif
#if OK_MBIDENT
    int     mb = 0;                     /* Count of MBCHAR  */
#endif
    size_t  len;                        /* Length of identifier     */
    char *  bp = identifier;

    do {
        if (bp < limit)
            *bp++ = c;
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
        c = get();
    } while ((type[ c] & (LET | DIG))           /* Letter or digit  */
#if OK_UCN
            || (mode == STD && c == '\\' && stdc2)
#endif
#if OK_MBIDENT
            || (mode == STD && (type[ c] & mbstart) && stdc3)
#endif
        );

    unget();
    *bp = EOS;

    if (bp >= limit && (warn_level & 1))        /* Limit of token   */
        cwarn( "Too long identifier truncated to \"%s\""    /* _W1_ */
                , identifier, 0L, NULLST);

    len = bp - identifier;
#if IDMAX > IDLEN90MIN
    /* UCN16, UCN32, MBCHAR are counted as one character for each.  */
#if OK_UCN
    if (mode == STD)
        len -= (uc2 * 5) - (uc4 * 9);
#endif
#if OK_MBIDENT
    if (mode == STD)
        len -= mb;
#endif
    if (standard && infile->fp && len > id_len_min && (warn_level & 4))
        cwarn( "Identifier longer than %.0s%ld characters \"%s\""   /* _W4_ */
                , NULLST, (long) id_len_min, identifier);
#endif  /* IDMAX > IDLEN90MIN   */

#if DOLLAR_IN_NAME
    if (diagnosed == FALSE && (warn_level & 2)
            && strchr( identifier, '$') != NULL) {
        cwarn( "'$' in identifier \"%s\"", identifier, 0L, NULLST); /* _W2_ */
        diagnosed = TRUE;                   /* Diagnose only once   */
    }
#endif
}

char *  scan_quote(
    int         delim,              /* ', " or < (header-name)      */
    char *      out,                /* Output buffer                */
    char *      out_end,            /* End of output buffer         */
    int         diag                /* Diagnostic should be output  */
)
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
    int         c;
    char *      out_p = out;

    /* Set again in case of called from routines other than scan_token().   */
    if (standard)
        in_token = TRUE;
    *out_p++ = delim;
    if (delim == '<')
        delim = '>';

scan:
    while ((c = get()) != EOS) {

#if MBCHAR
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
        } else if (c == '\\' && delim != '>') { /* In string literal    */
#if OK_UCN
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
#endif  /* OK_UCN   */
            *out_p++ = c;                   /* Escape sequence      */
            c = get();
escape:
#if MBCHAR
            if (type[ c] & mbstart) {   /* '\\' followed by multi-byte char */
                unget();
                continue;
            }
#endif
            if (! standard && c == '\n') {  /* <backslash><newline> */
                out_p--;                    /* Splice the lines     */
                if (cat_line( TRUE) == NULL)        /* End of file  */
                    break;
                c = get();
            }
        } else if (mode == POST_STD && c == ' ' && delim == '>'
                && infile->fp == NULL) {
            continue;   /* Skip space possibly inserted by macro expansion  */
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
            if (mode == OLD_PREP        /* Implicit closing of quote*/
                    && (delim == '"' || delim == '\''))
                return  out_p;
            if (delim == '"') {
                if (mode != POST_STD && lang_asm) { /* STD, KR      */
                    /* Concatenate the unterminated string to the next line */
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
            } else {
                cerror( "Unterminated header name %s%.0ld%s"        /* _E_  */
                        , out, 0L, skip);
            }
            out_p = NULL;
        } else if (delim == '\'' && out_p - out <= 2) {
            cerror( "Empty character constant %s%.0ld%s"    /* _E_  */
                    , out, 0L, skip);
            out_p = NULL;
        }
        else if (mode == POST_STD && delim == '>' && (warn_level & 2))
            cwarn(
        "Header-name enclosed by <, > is an obsolescent feature %s" /* _W2_ */
                    , out, 0L, skip);
#if NWORK-2 > SLEN90MIN
        if (standard && out_p - out > str_len_min && (warn_level & 4))
            cwarn( "Quotation longer than %.0s%ld bytes"    /* _W4_ */
                    , NULLST, str_len_min, NULLST);
#endif
    }

    in_token = FALSE;
    return  out_p;
}

static char *   cat_line(
    int     del_bsl         /* Delete the <backslash><newline> ?    */
)
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

static char *   scan_number(
    int     c,                              /* First char of number */
    char *  out,                            /* Output buffer        */
    char *  out_end                 /* Limit of output buffer       */
)
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
#if OK_UCN
            || (mode == STD && c == '\\' && stdc3)
#endif
#if OK_MBIDENT
            || (mode == STD && (type[ c] & mbstart) && stdc3)
#endif
        );

    *out_p = EOS;
    if (out_end < out_p)
        cfatal( "Too long pp-number token \"%s\""           /* _F_  */
                , out, 0L, NULLST);
    unget();
    return  out_p;
}

/* Original version of DECUS CPP, too exact for Standard preprocessing.     */
static char *   scan_number_prestd(
    int         c,                          /* First char of number */
    char *      out,                        /* Output buffer        */
    char *      out_end             /* Limit of output buffer       */
)
/*
 * Process a number.  We know that c is from 0 to 9 or dot.
 * Algorithm from Dave Conroy's Decus C.
 * Returns the advanced output pointer.
 */
{
    char * const    out_s = out;            /* For diagnostics      */
    int             radix;                  /* 8, 10, or 16         */
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

#if OK_UCN
static char *   scan_ucn(
    int     cnt,                            /* Bytes of sequence    */
    char *  out                             /* Output buffer        */
)
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
#endif  /* OK_UCN   */

static char *   scan_op(
    int     c,                          /* First char of the token  */
    char *  out                         /* Output buffer            */
)
/*
 * Scan C operator or punctuator into the specified buffer.
 * Return the advanced output pointer.
 * The code-number of the operator is stored to global variable 'openum'.
 * Note: '#' is not an operator nor a punctuator in other than control line,
 *   nevertheless is handled as a punctuator in this cpp for convenience.
 */
{
    int     c2, c3, c4;

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
        case ':':                                   /* <: i.e. [    */
            if (mode == STD && dig_flag)
                openum = OP_LBRCK_D;
            else
                openum = OP_LT;
            break;
        case '%':                                   /* <% i.e. {    */
            if (mode == STD && dig_flag)
                openum = OP_LBRACE_D;
            else
                openum = OP_LT;
            break;
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
        if (standard && (in_define || macro_line))  /* in #define or macro  */
            openum = ((c2 == '#') ? OP_CAT : OP_STR);   /* ##, #    */
        else
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
            if (cplus) {
                if ((c3 = get()) == '*') {              /* ->*      */
                    openum = OP_3;
                    *out++ = c3;
                } else {
                    /* openum = OP_2;   */
                    unget();
                }
            }   /* else openum = OP_2;  */              /* ->       */
            /* else openum = OP_2;      */
            break;
        default :   openum = OP_SUB;        break;      /* -        */
        }
        break;
    case '%':
        switch (c2) {
        case '=':                           break;      /* %=       */
        case '>':                                   /* %> i.e. }    */
            if (mode == STD && dig_flag)
                openum = OP_RBRACE_D;
            else
                openum = OP_MOD;
            break;
        case ':':
            if (mode == STD && dig_flag) {
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
        if (standard) {
            if (c2 == '.') {
                c3 = get();
                if (c3 == '.') {
                    openum = OP_ELL;                    /* ...      */
                    *out++ = c3;
                    break;
                } else {
                    unget();
                    openum = OP_1;
                }
            } else if (cplus && c2 == '*') {            /* .*       */
                /* openum = OP_2    */  ;
            } else {                                    /* .        */
                openum = OP_1;
            }
        } else {    
            openum = OP_1;
        }
        break;
    case ':':
        if (cplus && c2 == ':')                         /* ::       */
            /* openum = OP_2    */  ;
        else if (mode == STD && c2 == '>' && dig_flag)
            openum = OP_RBRCK_D;                    /* :> i.e. ]    */
        else                                            /* :        */
            openum = OP_COL;
        break;
    default:                                        /* Who knows ?  */
        cfatal( "Bug: Punctuator is mis-implemented %.0s0lx%x"      /* _F_  */
                , NULLST, (long) c, NULLST);
        openum = OP_1;
        break;
    }

    switch (openum) {
    case OP_STR:
        if (mode == STD && c == '%')    break;              /* %:   */
    case OP_1:
    case OP_NOT:    case OP_AND:    case OP_OR:     case OP_LT:
    case OP_GT:     case OP_ADD:    case OP_SUB:    case OP_MOD:
    case OP_MUL:    case OP_DIV:    case OP_XOR:    case OP_COM:
    case OP_COL:    /* Any single byte operator or punctuator       */
        unget();
        out--;
        break;
    default:        /* Two or more bytes operators or punctuators   */
        break;
    }

    *out = EOS;
    return  out;
}

int     id_operator(
    const char *    name
)
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

int     get( void)
/*
 * Return the next character from a macro or the current file.
 * Always return the value representable by unsigned char.
 */
{
    static int      squeezews = FALSE;
    int             len;
    int             c;
    FILEINFO *      file;

    /*
     * 'in_token' is set to TRUE while scan_token() is executed (and
     * scan_id(), scan_quote(), scan_number(), scan_ucn() and scan_op()
     * via scan_token()) in Standard mode to simplify tokenization.
     * Any token cannot cross "file"s.
     */
    if (in_token)
        return (*infile->bptr++ & UCHARMAX);

    if ((file = infile) == NULL)
        return  CHAR_EOF;                   /* End of all input     */

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
    if (! standard && squeezews) {
        if (*file->bptr == ' ')
            file->bptr++;                   /* Squeeze white spaces */
        squeezews = FALSE;
    }

    if (debug & GETC) {
        fprintf( fp_debug, "get(%s), line %ld, bptr = %d, buffer"
        , file->fp ? cur_fullname : file->filename ? file->filename : "NULL"
            , line, (int) (file->bptr - file->buffer));
        dump_string( NULLST, file->buffer);
        dump_unget( "get entrance");
    }

    /*
     * Read a character from the current input logical line or macro.
     * At EOS, either finish the current macro (freeing temporary storage)
     * or get another logical line by parse_line().
     * At EOF, exit the current file (#included) or, at EOF from the MCPP input
     * file, return CHAR_EOF to finish processing.
     * The character is converted to int with no sign-extension.
     */
    if ((c = (*file->bptr++ & UCHARMAX)) != EOS) {
        if (standard)
            return  c;                      /* Just a character     */
        if (! in_string && c == '\\' && *file->bptr == '\n'
                && in_define        /* '\\''\n' is deleted in #define line, */
                    /*   provided the '\\' is not the 2nd byte of mbchar.   */
                && ! last_is_mbchar( file->buffer, strlen( file->buffer) - 2)
            ) {
            if (*(file->bptr - 2) == ' ')
                squeezews = TRUE;
        } else {
            return  c;
        }
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
    free( file->buffer);                    /* Free buffer          */
    if (infile == NULL) {                   /* If at end of input,  */
        free( file);
        return  CHAR_EOF;                   /*   return end of file.*/
    }
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
        line++;                             /* Next line to #include*/
        sharp();                            /* Need a #line now     */
        line--;
        newlines = 0;                       /* Clear the blank lines*/
    } else if (file->filename && macro_name) {  /* Expanding macro  */
        if (exp_mac_ind < EXP_MAC_IND_MAX - 1)
            exp_mac_ind++;
        else
            exp_mac_ind = 1;
        /* Remember used macro name for diagnostic  */
        expanding_macro[ exp_mac_ind] = file->filename;
    }
    free( file);                            /* Free file space      */
    return  get();                          /* Get from the parent  */
}

static char *   parse_line( void)
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
    int         c;

    if ((sp = get_line( FALSE)) == NULL)    /* Next logical line    */
        return  NULL;                       /* End of a file        */
    if (in_asm) {                           /* In #asm block        */
        while (type[ *sp++ & UCHARMAX] & SPA)
            ;
        if (*--sp == '#')                   /* Directive line       */
            infile->bptr = sp;
        return  infile->bptr;               /* Don't tokenize       */
    }
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
                if (mode == POST_STD && (temp < tp && *(tp - 1) != ' '))
                    *tp++ = ' ';            /* Skip line top spaces */
                else if (mode == OLD_PREP && (temp == tp
                        || (*(tp - 1) != ' ' && *(tp - 1) != COM_SEP)))
                    *tp++ = COM_SEP;        /* Convert to magic character   */
                else if (temp == tp || *(tp - 1) != ' ')
                    *tp++ = ' ';            /* Squeeze white spaces */
                break;
            case '/':
                if (! standard)
                    goto  not_comment;
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
            default:                        /* Not a comment        */
not_comment:
                *tp++ = '/';
                sp--;                       /* To re-read           */
                break;
            }
            break;
        case '\r':                          /* Vertical white spaces*/
                /* Note that [CR+LF] is already converted to [LF].  */
        case '\f':
        case '\v':
            if (warn_level & 4)
                cwarn( "Converted %.0s0x%02lx to a space"   /* _W4_ */
                    , NULLST, (long) c, NULLST);
        case '\t':                          /* Horizontal space     */
        case ' ':
            if (mode == POST_STD && temp < tp && *(tp - 1) != ' ')
                *tp++ = ' ';                /* Skip line top spaces */
            else if (mode == OLD_PREP && temp < tp && *(tp - 1) == COM_SEP)
                *(tp - 1) = ' ';    /* Squeeze COM_SEP with spaces  */
            else if (temp == tp || *(tp - 1) != ' ')
                *tp++ = ' ';                /* Squeeze white spaces */
            break;
        case '"':                           /* String literal       */
        case '\'':                          /* Character constant   */
            infile->bptr = sp;
            if (standard) {
                tp = scan_quote( c, tp, limit, TRUE);
            } else {
                in_string = TRUE;   /* Enable line splicing by scan_quote() */
                tp = scan_quote( c, tp, limit, TRUE);   /*   (not by get()).*/
                in_string = FALSE;
            }
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

end_line:
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
                    || (mode == STD && *temp == '%' && *(temp + 1) == ':'))
            if (warn_level & 1)
                cwarn(
    "Macro started at line %.0s%ld swallowed directive-like line"   /* _W1_ */
                    , NULLST, macro_line, NULLST);
    }
    return  infile->buffer;
}

static char *   read_a_comment(
    char *      sp
)
/*
 * Read over a comment (which may cross the lines).
 */
{
    int         c;

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

static char *   get_line(
    int     in_comment
)
/*
 * ANSI (ISO) C: translation phase 1, 2.
 * Get the next logical line from source file.
 * Convert [CR+LF] to [LF]. 
 */
{
#if COMPILER == INDEPENDENT
#define cr_warn_level 1
#else
#define cr_warn_level 2
#endif
    static int  cr_converted;
    int     converted = FALSE;
    int     len;                            /* Line length - alpha  */
    char *  ptr;

    if (infile == NULL)                     /* End of a source file */
        return  NULL;
    ptr = infile->bptr = infile->buffer;

    while (fgets( ptr, (int) (infile->buffer + NBUFF - ptr), infile->fp)
            != NULL) {
        /* Translation phase 1  */
        line++;                     /* Gotten next physical line    */
        if (standard && line == line_limit + 1 && (warn_level & 1))
            cwarn( "Line number %.0s\"%ld\" got beyond range"       /* _W1_ */
                    , NULLST, line, NULLST);
        if (debug & (TOKEN | GETC)) {       /* Dump it to fp_debug  */
            fprintf( fp_debug, "\n#line %ld (%s)", line, cur_fullname);
            dump_string( NULLST, ptr);
        }
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
        if (*(ptr + len - 2) == '\r') {         /* [CR+LF]          */
            *(ptr + len - 2) = '\n';
            *(ptr + --len) = EOS;
            if (! cr_converted && (warn_level & cr_warn_level)) {
                cwarn( "Converted [CR+LF] to [LF]"  /* _W1_ _W2_    */
                        , NULLST, 0L, NULLST);
                cr_converted = TRUE;
            }
        }
        if (standard) {
            if (trig_flag)
                converted = cnv_trigraph( ptr);
            if (mode == POST_STD && dig_flag)
                converted += cnv_digraph( ptr);
            if (converted)
                len = strlen( ptr);
            /* Translation phase 2  */
            len -= 2;
            if (len >= 0) {
                if ((*(ptr + len) == '\\') && ! last_is_mbchar( ptr, len)) {
                            /* <backslash><newline> (not MBCHAR)    */
                    ptr = infile->bptr += len;  /* Splice the lines */
                    wrong_line = TRUE;
                    continue;
                }
            }
#if NBUFF-2 > SLEN90MIN
            if (ptr - infile->buffer + len + 2 > str_len_min + 1
                    && (warn_level & 4))    /* +1 for '\n'          */
            cwarn( "Logical source line longer than %.0s%ld bytes"  /* _W4_ */
                        , NULLST, str_len_min, NULLST);
#endif
        }
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

#define TRIOFFSET       10

int     cnv_trigraph(
    char *      in
)
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
    const char *    tp;

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

int     cnv_digraph(
    char *      in
)
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

static int  last_is_mbchar(
    const char *  in,               /* Input physical line          */
    int     len                     /* Length of the line minus 2   */
)
/*
 * Return 2, if the last char of the line is second byte of SJIS or BIGFIVE,
 * else return 0.
 */
{
    const char *    cp = in + len;
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

static void at_eof(
    int     in_comment
)
/*
 * Check the partial line, unterminated comment, unbalanced #if block,
 * uncompleted macro call at end of file or at end of input.
 */
{
    const char * const  format
            = "End of %s with %.0ld%s";                 /* _E_ _W1_ */
    const char * const  unterm_if_format
= "End of %s within #if (#ifdef) section started at line %ld";  /* _E_ _W1_ */
    const char * const  unterm_macro_format
            = "End of %s within macro call started at line %ld";/* _E_ _W1_ */
    const char * const  input
            = infile->parent ? "file" : "input";        /* _E_ _W1_ */
    const char * const  no_newline
            = "no newline, supplemented newline";       /* _W1_     */
    const char * const  unterm_com
            = "unterminated comment, terminated the comment";   /* _W1_     */
    const char * const  backsl = "\\, deleted the \\";  /* _W1_     */
    const char * const  unterm_asm_format
= "End of %s with unterminated #asm block started at line %ld"; /* _E_ _W1_ */
    size_t  len;
    char *  cp = infile->buffer;
    IFINFO *    ifp;

    len = strlen( cp);
    if (len && *(cp += (len - 1)) != '\n') {
        *++cp = '\n';                       /* Supplement <newline> */
        *++cp = EOS;
        if (standard && (warn_level & 1))
            cwarn( format, input, 0L, no_newline);
        else if (mode == KR && (warn_level & 1))
            cwarn( format, input, 0L, no_newline);
    }
    if (standard && infile->buffer < infile->bptr) {
        cp += len - 2;
        *cp++ = '\n';                       /* Delete the \\        */
        *cp = EOS;
        if (warn_level & 1)
            cwarn( format, input, 0L, backsl);
    }
    if (in_comment) {
        if ((standard || mode == KR) && (warn_level & 1))
            cwarn( format, input, 0L, unterm_com);
    }

    if (infile->initif < ifptr) {
        ifp = infile->initif + 1;
        if (standard) {
            cerror( unterm_if_format, input, ifp->ifline, NULLST);
            ifptr = infile->initif;         /* Clear information of */
            compiling = ifptr->stat;        /*   erroneous grouping */
        } else if (mode != OLD_PREP && (warn_level & 1)) {
            cwarn( unterm_if_format, input, ifp->ifline, NULLST);
        }
    }

    if (macro_line != 0 && macro_line != MACRO_ERROR
            && ((mode == STD && in_getarg) || ! standard)) {
        if (standard) {
            cerror( unterm_macro_format, input, macro_line, NULLST);
            macro_line = MACRO_ERROR;
        } else if (warn_level & 1) {
            cwarn( unterm_macro_format, input, macro_line, NULLST);
        }
    }

    if (in_asm && mode == KR && (warn_level & 1))
        cwarn( unterm_asm_format, input, in_asm, NULLST);
}

void    unget( void)
/*
 * Back the pointer to reread the last character.  Fatal error (code bug)
 * if we back too far.  unget() may be called, without problems, at end of
 * file.  Only one character may be ungotten.  If you need to unget more,
 * call unget_string().
 */
{
    if (in_token) {
        infile->bptr--;
        return;
    }

    if (infile != NULL) {
        if (mode == POST_STD && infile->fp) {
            switch (insert_sep) {
            case INSERTED_SEP:  /* Have just read an inserted separator */
                insert_sep = INSERT_SEP;
                return;
            case INSERT_SEP:
                cfatal( "Bug: unget() just after scan_token()"       /* _F_  */
                        , NULLST, 0L, NULLST);
                break;
            default:
                break;
            }
        }
        --infile->bptr;
        if (infile->bptr < infile->buffer)      /* Shouldn't happen */
            cfatal( "Bug: Too much pushback", NULLST, 0L, NULLST);  /* _F_  */
    }

    if (debug & GETC)
        dump_unget( "after unget");
}

FILEINFO *  unget_string(
    const char *    text,               /* Text to unget            */
    const char *    name                /* Name of the macro        */
)
/*
 * Push a string back on the input stream.  This is done by treating
 * the text as if it were a macro.
 */
{
    FILEINFO *      file;
    size_t          size;

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

char *  save_string(
    const char *      text
)
/*
 * Store a string into free memory.
 */
{
    char *      result;
    size_t      size;

    size = strlen( text) + 1;
    result = xmalloc( size);
    memcpy( result, text, size);
    return  result;
}

FILEINFO *  get_file(
    const char *    name,           /* File or macro name string    */
    size_t      bufsize             /* Line buffer size             */
)
/*
 * Common FILEINFO buffer initialization for a new file or macro.
 */
{
    FILEINFO *  file;

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
(xmalloc)(
    size_t      size
)
/*
 * Get a block of free memory.
 */
{
    char *      result;

    if ((result = (char *) malloc( size)) == NULL) {
        if (debug & MEMORY)
            print_heap();
       cfatal( out_of_memory, NULLST, (long) size, NULLST);
    }
    return  result;
}

char *  (xrealloc)(
    char *      ptr,
    size_t      size
)
/*
 * Reallocate malloc()ed memory.
 */
{
    char *      result;

    if ((result = (char *) realloc( ptr, size)) == NULL && size != 0) {
        /* 'size != 0' is necessary to cope with some               */
        /*   implementation of realloc( ptr, 0) which returns NULL. */
        if (debug & MEMORY)
            print_heap();
        cfatal( out_of_memory, NULLST, (long) size, NULLST);
    }
    return  result;
}

static void put_line(
    char *  out,
    FILE *  fp
)
/*
 * Put out a logical source line.
 * This routine is called only in OLD_PREP mode.
 */
{
    int     c;

    while ((c = *out++) != EOS) {
        if (c != COM_SEP)           /* Skip 0-length comment        */
            fputc( c, fp);
    }
}

static void do_msg(
    const char *    severity,       /* "fatal", "error", "warning"  */
    const char *    format,         /* Format for the error message */
    const char *    arg1,           /* String arg. for the message  */
    long            arg2,           /* Integer argument             */
    const char *    arg3            /* Second string argument       */
)
/*
 * Print filenames, macro names, line numbers and error messages.
 */
{
    FILEINFO *  file;
    DEFBUF *    defp;
    int         i;
    size_t      slen;
    const char *    arg_s[ 2];
    char *      arg_t[ 2];
    char *      tp;
    const char *    sp;
    int         c;
    int         ind;

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
            case TOK_SEP:
                if (mode == OLD_PREP)   /* COM_SEP                  */
                    break;              /* Skip magic characters    */
                /* Else fall through    */
            case RT_END:
            case IN_SRC:
                if (mode != STD) {
                    *tp++ = ' ';
                    /* Illegal control character, convert to a space*/
                    break;
                }                       /* Else fall through        */
            case CAT:
            case ST_QUOTE:
            case DEF_MAGIC:
                if (! standard)
                    *tp++ = ' ';
                break;                  /* Skip magic characters    */
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

    /* Print diagnostic */
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

    /* Print source line, includers and expanding macros    */
    file = infile;
    if (file != NULL && file->fp != NULL) {
        if (mode == OLD_PREP) {
            fputs( "    ", fp_err);
            put_line( file->buffer, fp_err);
        } else {
            fprintf( fp_err, "    %s", file->buffer);
                                            /* Current source line  */
        }
        file = file->parent;
    }
    while (file != NULL) {                  /* Print #includes, too */
        if (file->fp == NULL) {             /* Macro                */
            if (file->filename) {
                defp = look_id( file->filename);
                dump_a_def( "    macro", defp, FALSE, FALSE, TRUE, fp_err);
            }
        } else {                            /* Source file          */
            if (file->buffer[ 0] == '\0')
                strcpy( file->buffer, "\n");
            if (mode != OLD_PREP) {
                fprintf( fp_err, "    from %s%s: %ld:    %s",
                    *(file->dirp),          /* Include directory    */
                    file->filename,         /* Current file name    */
                    file->line,             /* Current line number  */
                    file->buffer);          /* The source line      */
            } else {
                fprintf( fp_err, "    from %s%s: %ld:    ",
                    *(file->dirp), file->filename, file->line);
                put_line( file->buffer, fp_err);
            }
        }
        file = file->parent;
    }

    /* Additional information of macro definitions  */
    for (ind = 1; ind <= exp_mac_ind; ind++) {
        int         ind_done;
        FILEINFO    *file;

        for (ind_done = 1; ind_done < ind; ind_done++)
            if (str_eq( expanding_macro[ ind], expanding_macro[ ind_done]))
                break;                      /* Already reported     */
        if (ind_done < ind)
            continue;
        for (file = infile; file; file = file->parent)
            if (file->fp == NULL && file->filename
                    && str_eq( expanding_macro[ ind], file->filename))
                break;                      /* Already reported     */
        if (file)
            continue;
        if ((defp = look_id( expanding_macro[ ind])) != NULL)
            dump_a_def( "    macro", defp, FALSE, FALSE, TRUE, fp_err);
            /* Macro already read over  */
    }

free_arg:
    for (i = 0; i < 2; i++)
        free( arg_t[ i]);
}

void    cfatal(
    const char *    format,
    const char *    arg1,
    long    arg2,
    const char *    arg3
)
/*
 * A real disaster.
 */
{
    do_msg( "fatal error", format, arg1, arg2, arg3);
    longjmp( error_exit, -1);
}

void    cerror(
    const char *    format,
    const char *    arg1,
    long    arg2,
    const char *    arg3
)
/*
 * Print a error message.
 */
{
    do_msg( "error", format, arg1, arg2, arg3);
    errors++;
}

void    cwarn(
    const char *    format,
    const char *    arg1,
    long    arg2,
    const char *    arg3
)
/*
 * Maybe an error.
 */
{
    do_msg( "warning", format, arg1, arg2, arg3);
}

void    dump_string(
    const char *    why,
    const char *    text
)
/*
 * Dump text readably.
 * Bug: macro argument number may be putout as a control character or any
 * other character, just after MAC_PARM has been read away.
 */
{
    const char *    cp;
    const char *    chr;
    int     c;

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
        case DEF_MAGIC:
            if (standard) {
                chr = "<MAGIC>";
                break;
            }       /* Else fall through    */
        case CAT:
            if (standard) {
                chr = "##";
                break;
            }       /* Else fall through    */
        case ST_QUOTE:
            if (standard) {
                chr = "#";
                break;
            }       /* Else fall through    */
        case RT_END:
            if (standard) {
                chr = "<RT_END>";
                break;
            }       /* Else fall through    */
        case IN_SRC:
            if (standard) {
                chr = "<SRC>";
                break;
            } else {                        /* Control character    */
                fprintf( fp_debug, "<^%c>", c + '@');
            }
        case TOK_SEP:
            if (mode == STD) {
                chr = "<TSEP>";
                break;
            } else if (mode == OLD_PREP) {          /* COM_SEP      */
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
    }

    fputc( '\n', fp_debug);
}

void    dump_unget(
    const char *    why
)
/*
 * Dump all ungotten junk (pending macros and current input lines).
 */
{
    const FILEINFO *    file;

    fputs( "dump of pending input text", fp_debug);
    if (why != NULL) {
        fputs( "-- ", fp_debug);
        fputs( why, fp_debug);
    }
    fputc( '\n', fp_debug);

    for (file = infile; file != NULL; file = file->parent)
        dump_string( file->filename ? file->filename : "NULL", file->bptr);
}

static void dump_token(
    int     token_type,
    const char *    cp                              /* Token        */
)
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

