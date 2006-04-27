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
 *                              E V A L . C
 *                  E x p r e s s i o n   E v a l u a t i o n
 *
 * Edit History of DECUS CPP / cpp5.c
 * 31-Aug-84    MM      USENET net.sources release
 * 29-Apr-85            Latest revision
 */

/*
 * CPP Version 2.0 / eval.c
 * 1998/08      kmatsui
 *      Renamed cpp5.c eval.c.
 *      Created overflow(), dumpval().
 *      Split evalsval(), evaluval() from evaleval().
 *      Made #if error returns 0 (rather than 1).
 *      Changed the type of #if evaluation from int to long / unsigned long
 *          (unsigned long is only for the compiler which has that type)
 *          or long long / unsigned long long for C99.
 *      Reinforced expression evaluation (eval(), opdope[]).
 *      Implemented evaluation of multi-character character constant, wide
 *          character constant and revised evaluation of multi-byte
 *          character constant.
 *      Revised most of the functions.
 */

/*
 * CPP Version 2.1 / eval.c
 * 1998/09      kmatsui
 *      Changed the type of #if expression from fixed size to variable size,
 *          which is determined by the value of __STDC_VERSION__.
 *      Implemented the evaluation of UCN sequence in character constant.
 */

/*
 * CPP Version 2.2 / eval.c
 * 1998/11      kmatsui
 *      Changed to evaluate "true" as 1 and "false" as 0 on C++, according
 *          to C++ Standard.
 */

/*
 * CPP Version 2.3 pre-release 1 / eval.c
 * 2002/08      kmatsui
 *      Implemented C99-compatible mode of C++ (evaluating #if expression
 *          as long long).
 *      Renamed the several functions using underscore.
 * CPP Version 2.3 release / eval.c
 * 2003/02      kmatsui
 *      Implemented identifier-like operators in C++98.
 *      Created chk_ops().
 */

/*
 * MCPP Version 2.4 prerelease
 * 2003/11      kmatsui
 *      Changed some macro names according to config.h.
 *
 * MCPP Version 2.4 release
 * 2004/02      kmatsui
 *      Updated eval_char() according to the extension of multi-byte
 *          character handling.
 */

/*
 * MCPP Version 2.5
 * 2005/03      kmatsui
 *      Absorbed POST_STANDARD into STANDARD and OLD_PREPROCESSOR into
 *          PRE_STANDARD.
 *      Changed to use only (signed) long in PRE_STANDARD.
 */

/*
 * The routines to evaluate #if expression are placed here.
 * Some routines are used also to evaluate the value of numerical tokens.
 */

#if PREPROCESSED
#include    "mcpp.H"
#else
#include    "system.H"
#include    "internal.H"
#endif

typedef struct optab {
    char    op;                     /* Operator                     */
    char    prec;                   /* Its precedence               */
    char    skip;                   /* Short-circuit: non-0 to skip */
} OPTAB;

#if PROTO

static int      eval_lex( void);
static int      chk_ops( void);
static VAL_SIGN *   eval_char( char * const token);
static expr_t   eval_one( char ** seq_pp, int wide, int mbits, int * ucn8);
static VAL_SIGN *   eval_eval( VAL_SIGN * valp, int op);
static expr_t   eval_signed( VAL_SIGN ** valpp, expr_t v1, expr_t v2, int op);
#if HAVE_UNSIGNED_LONG && MODE == STANDARD
static expr_t   eval_unsigned( VAL_SIGN ** valpp, uexpr_t v1u, uexpr_t v2u
        , int op);
#endif
static void     overflow( const char * op_name, VAL_SIGN ** valpp);
#if OK_SIZE
static int      do_sizeof( void);
static int      look_type( int typecode);
#endif
#if DEBUG_EVAL
static void     dump_val( const char * msg, const VAL_SIGN * valp);
static void     dump_stack( const OPTAB * opstack, const OPTAB * opp
        , const VAL_SIGN * value, const VAL_SIGN * valp);
#endif

#else   /* ! PROTO  */

static int      eval_lex();         /* Get type and value of token  */
static int      chk_ops();          /* Check identifier-like ops    */
static VAL_SIGN *   eval_char();    /* Evaluate character constant  */
static expr_t   eval_one();         /* Evaluate a character         */
static VAL_SIGN *   eval_eval();    /* Entry to #if arithmetic      */
static expr_t   eval_signed();      /* Do signed arithmetic of expr.*/
#if HAVE_UNSIGNED_LONG && MODE == STANDARD
static expr_t   eval_unsigned();    /* Do unsigned arithmetic       */
#endif
static void     overflow();         /* Diagnose overflow of expr.   */
#if OK_SIZE
static int      do_sizeof();        /* Evaluate sizeof (type)       */
static int      look_type();        /* Look for type of the name    */
#endif
#if DEBUG_EVAL
static void     dump_val();         /* Print value of an operand    */
static void     dump_stack();       /* Print stacked operators      */
#endif

#endif

/* For debug and error messages.    */
static const char * const   opname[ OP_END + 1] = {
    "end of expression",    "val",  "(",
    "unary +",      "unary -",      "~",    "!",
    "*",    "/",    "%",
    "+",    "-",    "<<",   ">>",
    "<",    "<=",   ">",    ">=",   "==",   "!=",
    "&",    "^",    "|",    "&&",   "||",
    "?",    ":",
    ")",    "(none)"
};

/*
 * opdope[] has the operator (and operand) precedence:
 *     Bits
 *        7     Unused (so the value is always positive)
 *      6-2     Precedence (0000 .. 0174)
 *      1-0     Binary op. flags:
 *          10  The next binop flag (binop should/not follow).
 *          01  The binop flag (should be set/cleared when this op is seen).
 * Note:   next binop
 * value    1   0   Value doesn't follow value.
 *                  Binop, ), END should follow value, value or unop doesn't.
 *  (       0   0   ( doesn't follow value.  Value follows.
 * unary    0   0   Unop doesn't follow value.  Value follows.
 * binary   0   1   Binary op follows value.  Value follows.
 *  )       1   1   ) follows value.  Binop, ), END follows.
 * END      0   1   END follows value, doesn't follow ops.
 */

static const char   opdope[ OP_END + 1] = {
    0001,                               /* End of expression        */
    0002, 0170,                         /* VAL (constant), LPA      */
/* Unary op's   */
    0160, 0160, 0160, 0160,             /* PLU, NEG, COM, NOT       */
/* Binary op's  */
    0151, 0151, 0151,                   /* MUL, DIV, MOD,           */
    0141, 0141, 0131, 0131,             /* ADD, SUB, SL, SR         */
    0121, 0121, 0121, 0121, 0111, 0111, /* LT, LE, GT, GE, EQ, NE   */
    0101, 0071, 0061, 0051, 0041,       /* AND, XOR, OR, ANA, ORO   */
    0031, 0031,                         /* QUE, COL                 */
/* Parens       */
    0013, 0023                          /* RPA, END                 */
};
/*
 * OP_QUE, OP_RPA and unary operators have alternate precedences:
 */
#define OP_RPA_PREC     0013
#define OP_QUE_PREC     0024    /* From right to left grouping      */
#define OP_UNOP_PREC    0154            /*      ditto               */

/*
 * S_ANDOR and S_QUEST signal "short-circuit" boolean evaluation, so that
 *      #if FOO != 0 && 10 / FOO ...
 * doesn't generate an error message.  They are stored in optab.skip.
 */
#define S_ANDOR         2
#define S_QUEST         1

static VAL_SIGN     ev;     /* Current value and signedness     */
static int          skip = 0;   /* 3-way signal of skipping expr*/
static const char * const   non_eval
        = " (in non-evaluated sub-expression)";             /* _W8_ */

#if OK_SIZE

/*
 * S_CHAR etc.  define the sizeof the basic TARGET machine word types.
 *      By default, sizes are set to the values for the HOST computer.  If
 *      this is inappropriate, see those tables for details on what to change.
 *      Also, if you have a machine where sizeof (signed int) differs from
 *      sizeof (unsigned int), you will have to edit those tables and code in
 *      eval.c.
 * Note: sizeof in #if expression is disallowed by Standard.
 */

#define S_CHAR      (sizeof (char))
#define S_SINT      (sizeof (short int))
#define S_INT       (sizeof (int))
#define S_LINT      (sizeof (long int))
#define S_FLOAT     (sizeof (float))
#define S_DOUBLE    (sizeof (double))
#define S_PCHAR     (sizeof (char *))
#define S_PSINT     (sizeof (short int *))
#define S_PINT      (sizeof (int *))
#define S_PLINT     (sizeof (long int *))
#define S_PFLOAT    (sizeof (float *))
#define S_PDOUBLE   (sizeof (double *))
#define S_PFPTR     (sizeof (int (*)()))
#if HAVE_LONG_LONG
#if COMPILER == BORLANDC
#define S_LLINT     (sizeof (__int64))
#define S_PLLINT    (sizeof (__int64 *))
#else
#define S_LLINT     (sizeof (long long int))
#define S_PLLINT    (sizeof (long long int *))
#endif
#endif
#if HAVE_LONG_DOUBLE
#define S_LDOUBLE   (sizeof (long double))
#define S_PLDOUBLE  (sizeof (long double *))
#endif

typedef struct types {
    int         type;               /* This is the bit if           */
    char *      token_name;         /* this is the token word       */
    int         excluded;           /* but these aren't legal here. */
} TYPES;

#define ANYSIGN     (T_SIGNED | T_UNSIGNED)
#if HAVE_LONG_DOUBLE
#define ANYFLOAT    (T_FLOAT | T_DOUBLE | T_LONGDOUBLE)
#else
#define ANYFLOAT    (T_FLOAT | T_DOUBLE)
#endif
#if HAVE_LONG_LONG
#define ANYINT      (T_CHAR | T_SHORT | T_INT | T_LONG | T_LONGLONG)
#else
#define ANYINT      (T_CHAR | T_SHORT | T_INT | T_LONG)
#endif

static const TYPES basic_types[] = {
    { T_CHAR,       "char",         ANYFLOAT | ANYINT },
    { T_SHORT,      "short",        ANYFLOAT | ANYINT },
    { T_INT,        "int",          ANYFLOAT | T_CHAR | T_INT },
    { T_LONG,       "long",         ANYFLOAT | ANYINT },
#if HAVE_LONG_LONG
#if COMPILER == BORLANDC
    { T_LONGLONG,   "__int64",      ANYFLOAT | ANYINT },
#else
    { T_LONGLONG,   "long long",    ANYFLOAT | ANYINT },
#endif
#endif
    { T_FLOAT,      "float",        ANYFLOAT | ANYINT | ANYSIGN },
    { T_DOUBLE,     "double",       ANYFLOAT | ANYINT | ANYSIGN },
#if HAVE_LONG_DOUBLE
    { T_LONGDOUBLE, "long double",  ANYFLOAT | ANYINT | ANYSIGN },
#endif
    { T_SIGNED,     "signed",       ANYFLOAT | ANYINT | ANYSIGN },
    { T_UNSIGNED,   "unsigned",     ANYFLOAT | ANYINT | ANYSIGN },
    { 0,            NULL,           0 }     /* Signal end           */
};

/*
 * In this table, T_FPTR (pointer to function) should be placed last.
 */
SIZES     size_table[] = {
    { T_CHAR,   S_CHAR,     S_PCHAR     },          /* char         */
    { T_SHORT,  S_SINT,     S_PSINT     },          /* short int    */
    { T_INT,    S_INT,      S_PINT      },          /* int          */
    { T_LONG,   S_LINT,     S_PLINT     },          /* long         */
#if HAVE_LONG_LONG
    { T_LONGLONG, S_LLINT,  S_PLLINT    },          /* long long    */
#endif
    { T_FLOAT,  S_FLOAT,    S_PFLOAT    },          /* float        */
    { T_DOUBLE, S_DOUBLE,   S_PDOUBLE   },          /* double       */
#if HAVE_LONG_DOUBLE
    { T_LONGDOUBLE, S_LDOUBLE, S_PLDOUBLE },        /* long double  */
#endif
    { T_FPTR,   0,          S_PFPTR     },          /* int (*())    */
    { 0,        0,          0           }           /* End of table */
};

#endif  /* OK_SIZE  */

#define is_binary(op)   (FIRST_BINOP <= op && op <= LAST_BINOP)
#define is_unary(op)    (FIRST_UNOP  <= op && op <= LAST_UNOP)


expr_t
#if PROTO
eval( void)
#else
eval()
#endif
/*
 * Evaluate a #if expression.  Straight-forward operator precedence.
 * This is called from control() on encountering an #if statement.
 * It calls the following routines:
 * eval_lex()   Lexical analyser -- returns the type and value of
 *              the next input token.
 * eval_eval()  Evaluates the current operator, given the values on the
 *              value stack.  Returns a pointer to the (new) value stack.
 */
{
    VAL_SIGN        value[ NEXP * 2 + 1];   /* Value stack          */
    OPTAB           opstack[ NEXP * 3 + 1]; /* Operator stack       */
#if MODE == STANDARD
    int             parens = 0;     /* Nesting levels of (, )       */
#endif
    int             prec;           /* Operator precedence          */
    int             binop = 0;      /* Set if binary op. needed     */
    int             op1;            /* Operator from stack          */
    int             skip_cur;       /* For short-circuit testing    */
    VAL_SIGN *      valp = value;   /* -> Value and signedness      */
    register OPTAB *    opp = opstack;      /* -> Operator stack    */
    register int    op;             /* Current operator             */

    opp->op = OP_END;               /* Mark bottom of stack         */
    opp->prec = opdope[ OP_END];    /* And its precedence           */
    skip = skip_cur = opp->skip = 0;        /* Not skipping now     */

    while (1) {
#if DEBUG_EVAL
        if (debug & EXPRESSION)
            fprintf( fp_debug
                    , "In eval loop skip = %d, binop = %d, line is: %s\n"
                    , opp->skip, binop, infile->bptr);
#endif
        skip = opp->skip;
        op = eval_lex();
        skip = 0;                   /* Reset to be ready to return  */
        switch (op) {
        case OP_SUB :
            if (binop == 0)
                op = OP_NEG;                /* Unary minus          */
            break;
        case OP_ADD :
            if (binop == 0)
                op = OP_PLU;                /* Unary plus           */
            break;
        case OP_FAIL:
            return  0L;                     /* Token error          */
        }
#if DEBUG_EVAL
        if (debug & EXPRESSION)
            fprintf( fp_debug
                    , "op = %s, opdope = %04o, binop = %d, skip = %d\n"
                    , opname[ op], opdope[ op], binop, opp->skip);
#endif
        if (op == VAL) {                    /* Value?               */
            if (binop != 0) {               /* Binop is needed      */
                cerror( "Misplaced constant \"%s\""         /* _E_  */
                        , work, 0L, NULLST);
                return  0L;
            } else if (& value[ NEXP * 2] <= valp) {
                cerror( "More than %.0s%ld constants stacked at %s" /* _E_  */
                        , NULLST, (long) (NEXP * 2 - 1), work);
                return  0L;
            } else {
#if DEBUG_EVAL
                if (debug & EXPRESSION) {
                    dump_val( "pushing ", &ev);
                    fprintf( fp_debug, " onto value stack[%d]\n"
                            , (int)(valp - value));
                }
#endif
                valp->val = ev.val;
                (valp++)->sign = ev.sign;
                binop = 1;  /* Binary operator or so should follow  */
            }
            continue;
        }                                   /* Else operators       */
        prec = opdope[ op];
        if (binop != (prec & 1)) {
            if (op == OP_EOE)
                cerror( "Unterminated expression"           /* _E_  */
                        , NULLST, 0L, NULLST);
            else
                cerror( "Operator \"%s\" in incorrect context"      /* _E_  */
                        , opname[ op], 0L, NULLST);
            return  0L;
        }
        binop = (prec & 2) >> 1;            /* Binop should follow? */

        while (1) {
#if DEBUG_EVAL
            if (debug & EXPRESSION)
                fprintf( fp_debug
                        , "op %s, prec %d, stacked op %s, prec %d, skip %d\n"
                , opname[ op], prec, opname[ opp->op], opp->prec, opp->skip);
#endif

            /* Stack coming sub-expression of higher precedence.    */
            if (opp->prec < prec) {
                if (op == OP_LPA) {
                    prec = OP_RPA_PREC;
#if MODE == STANDARD
                    if ((warn_level & 4) && ++parens == exp_nest_min + 1)
                        cwarn(
                    "More than %.0s%ld nesting of parens"   /* _W4_ */
                            , NULLST, (long) exp_nest_min, NULLST);
#endif
                } else if (op == OP_QUE) {
                    prec = OP_QUE_PREC;
                } else if (is_unary( op)) {
                    prec = OP_UNOP_PREC;
                }
                op1 = opp->skip;            /* Save skip for test   */
                /*
                 * Push operator onto operator stack.
                 */
                opp++;
                if (& opstack[ NEXP * 3] <= opp) {
                    cerror(
            "More than %.0s%ld operators and parens stacked at %s"  /* _E_  */
                            , NULLST, (long) (NEXP * 3 - 1), opname[ op]);
                    return  0L;
                }
                opp->op = op;
                opp->prec = prec;
                if (&value[0] < valp)
                    skip_cur = (valp[-1].val != 0L);
                                            /* Short-circuit tester */
                /*
                 * Do the short-circuit stuff here.  Short-circuiting
                 * stops automagically when operators are evaluated.
                 */
                if ((op == OP_ANA && ! skip_cur)
                        || (op == OP_ORO && skip_cur)) {
                    opp->skip = S_ANDOR;    /* And/or skip starts   */
                    if (skip_cur)           /* Evaluate non-zero    */
                        valp[-1].val = 1L;  /*   value to 1         */
                } else if (op == OP_QUE) {  /* Start of ?: operator */
                    opp->skip = (op1 & S_ANDOR) | (!skip_cur ? S_QUEST : 0);
                } else if (op == OP_COL) {  /* : inverts S_QUEST    */
                    opp->skip = (op1 & S_ANDOR)
                              | (((op1 & S_QUEST) != 0) ? 0 : S_QUEST);
                } else {                    /* Other operators leave*/
                    opp->skip = op1;        /*  skipping unchanged. */
                }
#if DEBUG_EVAL
                if (debug & EXPRESSION) {
                    fprintf( fp_debug, "stacking %s, ", opname[ op]);
                    if (&value[0] < valp)
                        dump_val( "valp[-1].val == ", valp - 1);
                    fprintf( fp_debug, " at %s\n", infile->bptr);
                    dump_stack( opstack, opp, value, valp);
                }
#endif
                break;
            }

            /*
             * Coming sub-expression is of lower precedence.
             * Evaluate stacked sub-expression.
             * Pop operator from operator stack and evaluate it.
             * End of stack and '(', ')' are specials.
             */
            skip_cur = opp->skip;           /* Remember skip value  */
            switch ((op1 = opp->op)) {      /* Look at stacked op   */
            case OP_END:                    /* Stack end marker     */
                if (op == OP_RPA) {         /* No corresponding (   */
                    cerror( "Excessive \")\"", NULLST, 0L, NULLST); /* _E_  */
                    return  0L;
                }
                if (op == OP_EOE)
                    return  valp[-1].val;   /* Finished ok.         */
                break;
            case OP_LPA:                    /* ( on stack           */
                if (op != OP_RPA) {         /* Matches ) on input   */
                    cerror( "Missing \")\"", NULLST, 0L, NULLST);   /* _E_  */
                    return  0L;
                }
                opp--;                      /* Unstack it           */
#if MODE == STANDARD
                parens--;                   /* Count down nest level*/
#endif
                break;
            case OP_QUE:                    /* Evaluate true expr.  */
                break;
            case OP_COL:                    /* : on stack           */
                opp--;                      /* Unstack :            */
                if (opp->op != OP_QUE) {    /* Matches ? on stack?  */
                    cerror(
                    "Misplaced \":\", previous operator is \"%s\""  /* _E_  */
                            , opname[opp->op], 0L, NULLST);
                    return  0L;
                }
                /* Evaluate op1.            Fall through            */
            default:                        /* Others:              */
                opp--;                      /* Unstack the operator */
#if DEBUG_EVAL
                if (debug & EXPRESSION) {
                    fprintf( fp_debug, "Stack before evaluation of %s\n"
                            , opname[ op1]);
                    dump_stack( opstack, opp, value, valp);
                }
#endif
                if (op1 == OP_COL)
                    skip = 0;
                else
                    skip = skip_cur;
                valp = eval_eval( valp, op1);
                if (valp->sign == VAL_ERROR)
                    return  0L;     /* Out of range or divide by 0  */
                valp++;
                skip = 0;
#if DEBUG_EVAL
                if (debug & EXPRESSION) {
                    fprintf( fp_debug, "Stack after evaluation\n");
                    dump_stack( opstack, opp, value, valp);
                }
#endif
            }                               /* op1 switch end       */

            if (op1 == OP_END || op1 == OP_LPA || op1 == OP_QUE)
                break;                      /* Read another op.     */
        }                                   /* Stack unwind loop    */

    }

    return  0L;                             /* Never reach here     */
}

static int
#if PROTO
eval_lex( void)
#else
eval_lex()
#endif
/*
 * Return next operator or value to evaluate.  Called from eval().  It calls
 * a special-purpose routines for character constants and numeric values:
 *      eval_char()     called to evaluate 'x'
 *      eval_num()      called to evaluate numbers
 * C++98 treats 11 identifier-like tokens as operators.
 * POST_STD forbids character constants in #if expression.
 */
{
#if MODE == STANDARD
    int     c1;
#endif
    VAL_SIGN *  valp;
    int     warn = ! skip || (warn_level & 8);
    int     token_type;
    register int    c;

    ev.sign = SIGNED;                       /* Default signedness   */
    ev.val = 0L;            /* Default value (on error or 0 value)  */
    c = skip_ws();
    if (c == '\n') {
        unget();
        return  OP_EOE;                     /* End of expression    */
    }
    token_type = get_unexpandable( c, warn);
#if MODE == STANDARD
    if (macro_line == MACRO_ERROR)      /* Unterminated macro call  */
        return  OP_FAIL;
#endif
    if (token_type == NO_TOKEN)
        return  OP_EOE;     /* Only macro(s) expanding to 0-token   */

    switch (token_type) {
    case NAM:
#if MODE == STANDARD
        if (str_eq( identifier, "defined")) {   /* defined name     */
            c1 = c = skip_ws();
            if (c == '(')                   /* Allow defined (name) */
                c = skip_ws();
            if (scan_token( c, (workp = work, &workp), work_end) == NAM) {
                if (warn)
                    ev.val = (look_id( identifier) != NULL);
                if (c1 != '(' || skip_ws() == ')')  /* Balanced ?   */
                    return  VAL;            /* Parsed ok            */
            }
            cerror( "Bad defined syntax: %s"                /* _E_  */
                    , infile->fp ? "" : infile->buffer, 0L, NULLST);
            break;
        }
        if (cplus) {
            if (str_eq( identifier, "true")) {
                ev.val = 1L;
                return  VAL;
            } else if (str_eq( identifier, "false")) {
                ev.val = 0L;
                return  VAL;
            } else if (mode != POST_STD
                    && (openum = id_operator( identifier)) != 0) {
                /* Identifier-like operator in C++98    */
                strcpy( work, identifier);
                return  chk_ops();
            }
        }
#endif
#if OK_SIZE
        if (mode != POST_STD && str_eq( identifier, "sizeof"))
            /* sizeof hackery       */
            return  do_sizeof();            /* Gets own routine     */
#endif
        /*
         * The ANSI C Standard says that an undefined symbol
         * in an #if has the value zero.  We are a bit pickier,
         * warning except where the programmer was careful to write
         *          #if defined(foo) ? foo : 0
         */
        if ((! skip && (warn_level & 4)) || (skip && (warn_level & 8)))
            cwarn( "Undefined symbol \"%s\"%.0ld%s" /* _W4_ _W8_    */
                    , identifier, 0L, skip ? non_eval : ", evaluated to 0");
        return  VAL;
    case CHR:                               /* Character constant   */
#if MODE == STANDARD
    case WCHR:                              /* Wide char constant   */
        if (mode == POST_STD) {
            cerror( "Can't use a character constant %s"     /* _E_  */
                    , work, 0L, NULLST);
            break;
        }
#endif
        valp = eval_char( work);            /* 'valp' points 'ev'   */
        if (valp->sign == VAL_ERROR)
            break;
#if DEBUG_EVAL
        if (debug & EXPRESSION) {
            dump_val( "eval_char returns ", &ev);
            fputc( '\n', fp_debug);
        }
#endif
        return  VAL;                        /* Return a value       */
    case STR:                               /* String literal       */
#if MODE == STANDARD
    case WSTR:                              /* Wide string literal  */
#endif
        cerror( "Can't use a string literal %s", work, 0L, NULLST); /* _E_  */
        break;
    case NUM:                               /* Numbers are harder   */
        valp = eval_num( work);             /* 'valp' points 'ev'   */
        if (valp->sign == VAL_ERROR)
            break;
#if DEBUG_EVAL
        if (debug & EXPRESSION) {
            dump_val( "eval_num returns ", &ev);
            fputc( '\n', fp_debug);
        }
#endif
        return  VAL;
    case OPE:                           /* Operator or punctuator   */
        return  chk_ops();

    default:                                /* Total nonsense       */
        cerror( "Can't use the character %.0s0x%02lx"       /* _E_  */
                , NULLST, (long) c, NULLST);
        break;
    }

    return  OP_FAIL;                        /* Any errors           */
}

static int
#if PROTO
chk_ops( void)
#else
chk_ops()
#endif
/*
 * Check the operator.
 * If it can't be used in #if expression return OP_FAIL
 * else return openum.
 */
{
    switch (openum) {
#if MODE == STANDARD
    case OP_STR:    case OP_CAT:    case OP_ELL:
#endif
    case OP_1:      case OP_2:      case OP_3:
        cerror( "Can't use the operator \"%s\""             /* _E_  */
                , work, 0L, NULLST);
        return  OP_FAIL;
    default:
        return  openum;
    }
}

#if OK_SIZE

static int
#if PROTO
do_sizeof( void)
#else
do_sizeof()
#endif
/*
 * Process the sizeof (basic type) operation in an #if string.
 * Sets ev.val to the size and returns
 *      VAL             success
 *      OP_FAIL         bad parse or something.
 * This routine is never called in POST_STD mode.
 */
{
    const char * const  no_type = "sizeof: No type specified";      /* _E_  */
    int     warn = ! skip || (warn_level & 8);
    int     type_end = FALSE;
    int     typecode = 0;
    int     token_type;
    register const SIZES *  sizp = NULL;

    if (get_unexpandable( skip_ws(), warn) != OPE || openum != OP_LPA)
        goto  no_good;                      /* Not '('              */

    /*
     * Scan off the tokens.
     */

    while (! type_end) {
        token_type = get_unexpandable( skip_ws(), warn);
                                /* Get next token expanding macros  */
        switch (token_type) {
        case OPE:
            if (openum == OP_LPA) {         /* thing (*)() func ptr */
                if (get_unexpandable( skip_ws(), warn) == OPE
                        && openum == OP_MUL
                        && get_unexpandable( skip_ws(), warn) == OPE
                        && openum == OP_RPA) {      /* (*)          */
                    if (get_unexpandable( skip_ws(), warn) != OPE
                            || openum != OP_LPA
                            || get_unexpandable( skip_ws(), warn) != OPE
                            || openum != OP_RPA)    /* Not ()       */
                        goto  no_good;
                    typecode |= T_FPTR;     /* Function pointer     */
                } else {                    /* Junk is an error     */
                    goto  no_good;
                }
            } else {                        /* '*' or ')'           */
                type_end = TRUE;
            }
            break;
        case NAM:                           /* Look for type comb.  */
            if ((typecode = look_type( typecode)) == 0)
                return  OP_FAIL;            /* Illegal type or comb.*/
            break;
        default:    goto  no_good;          /* Illegal token        */
        }
    }                                       /* End of while         */

    /*
     * We are at the end of the type scan.  Chew off '*' if necessary.
     */
    if (token_type == OPE) {
        if (openum == OP_MUL) {             /* '*'                  */
            typecode |= T_PTR;
            if (get_unexpandable( skip_ws(), warn) != OPE)
                goto  no_good;
        }
        if (openum == OP_RPA) {             /* ')'                  */
            /*
             * Last syntax check
             * We assume that all function pointers are the same size:
             *          sizeof (int (*)()) == sizeof (float (*)())
             * We assume that signed and unsigned don't change the size:
             *          sizeof (signed int) == sizeof (unsigned int)
             */
            if ((typecode & T_FPTR) != 0) { /* Function pointer     */
                typecode = T_FPTR | T_PTR;
            } else {                        /* Var or var * datum   */
                typecode &= ~(T_SIGNED | T_UNSIGNED);
#if HAVE_LONG_LONG
                if ((typecode & (T_SHORT | T_LONG | T_LONGLONG)) != 0)
#else
                if ((typecode & (T_SHORT | T_LONG)) != 0)
#endif
                    typecode &= ~T_INT;
            }
            if ((typecode & ~T_PTR) == 0) {
                cerror( no_type, NULLST, 0L, NULLST);
                return  OP_FAIL;
            } else {
                /*
                 * Exactly one bit (and possibly T_PTR) may be set.
                 */
                for (sizp = size_table; sizp->bits != 0; sizp++) {
                    if ((typecode & ~T_PTR) == sizp->bits) {
                        ev.val = ((typecode & T_PTR) != 0)
                                ? sizp->psize : sizp->size;
                        break;
                    }
                }
            }
        } else {
            goto  no_good;
        }
    } else {
        goto  no_good;
    }

#if DEBUG_EVAL
    if (debug & EXPRESSION) {
        if (sizp)
            fprintf( fp_debug,
            "sizp->bits:0x%x sizp->size:0x%x sizp->psize:0x%x ev.val:0x%lx\n"
                    , sizp->bits, sizp->size, sizp->psize, ev.val);
    }
#endif
#if MODE == STANDARD
    if (warn_level & 8)
        cwarn( "sizeof is disallowed in Standard"           /* _W8_ */
                , NULLST, 0L, NULLST);
#endif
    return  VAL;

no_good:
    unget();
    cerror( "sizeof: Syntax error", NULLST, 0L, NULLST);    /* _E_  */
    return  OP_FAIL;
}

static int
#if PROTO
look_type( int typecode)
#else
look_type( typecode)
    int     typecode;
#endif
{
    const char * const  unknown_type
            = "sizeof: Unknown type \"%s\"%.0ld%s";     /* _E_ _W8_ */
    const char * const  illeg_comb
    = "sizeof: Illegal type combination with \"%s\"%.0ld%s";    /* _E_ _W8_ */
    int     token_type;
    register const TYPES *  tp;

#if HAVE_LONG_LONG || HAVE_LONG_DOUBLE
    if (str_eq( identifier, "long")) {
        if ((token_type
                = get_unexpandable( skip_ws(), !skip || (warn_level & 8)))
                == NO_TOKEN)
            return  typecode;
        if (token_type == NAM) {
#if HAVE_LONG_LONG
            if (str_eq( identifier, "long")) {
                strcpy( work, "long long");
                goto  basic;
            }
#endif
#if HAVE_LONG_DOUBLE
            if (str_eq( identifier, "double")) {
                strcpy( work, "long double");
                goto  basic;
            }
#endif
        }
        unget_string( work, NULLST);        /* Not long long        */
        strcpy( work, "long");              /*   nor long double    */
    }
#endif  /* HAVE_LONG_LONG || HAVE_LONG_DOUBLE   */

    /*
     * Look for this unexpandable token in basic_types.
     */
basic:
    for (tp = basic_types; tp->token_name != NULL; tp++) {
        if (str_eq( work, tp->token_name))
            break;
    }

    if (tp->token_name == NULL) {
        if (! skip) {
            cerror( unknown_type, work, 0L, NULLST);
            return  0;
        } else if (warn_level & 8) {
            cwarn( unknown_type, work, 0L, non_eval);
        }
    }
    if ((typecode & tp->excluded) != 0) {
        if (! skip) {
            cerror( illeg_comb, work, 0L, NULLST);
            return  0;
        } else if (warn_level & 8) {
            cwarn( illeg_comb, work, 0L, non_eval);
        }
    }

#if DEBUG_EVAL
    if (debug & EXPRESSION) {
        if (tp->token_name)
            fprintf( fp_debug,
            "sizeof -- typecode:0x%x tp->token_name:\"%s\" tp->type:0x%x\n"
                    , typecode, tp->token_name, tp->type);
    }
#endif
    return  typecode |= tp->type;           /* Or in the type bit   */
}

#endif  /* OK_SIZE  */

VAL_SIGN *
#if PROTO
eval_num( const char * nump)
#else
eval_num( nump)
    char *  nump;                           /* Preprocessing number */
#endif
/*
 * Evaluate number for #if lexical analysis.  Note: eval_num recognizes
 * the unsigned suffix, but only returns a signed expr_t value, and stores
 * the signedness to ev.sign, which is set UNSIGNED (== unsigned) if the
 * value is not in the range of positive (signed) expr_t (or long).
 */
{
    const char * const  not_integer = "Not an integer \"%s\"";      /* _E_ */
    const char * const  out_of_range
            = "Constant \"%s\"%.0ld%s is out of range"; /* _E_ _W8_ */
    expr_t          value;
#if HAVE_UNSIGNED_LONG && MODE == STANDARD
    uexpr_t         v, v1;  /* unsigned long or unsigned long long  */
#else
    expr_t          v, v1;                  /* signed long          */
#endif
#if MODE == STANDARD
    int             uflag = FALSE;
#endif
#if HAVE_LONG_LONG && MODE == STANDARD
    int             llflag = FALSE;
#endif
    int             lflag = FALSE;
    int             erange = FALSE;
    int             base;
    int             c1;
    register int    c;
    const char *    cp = nump;

    ev.sign = SIGNED;                       /* Default signedness   */
    ev.val = 0L;                            /* Default value        */
    if ((type[ c = *cp++ & UCHARMAX] & DIG) == 0)   /* Dot          */
        goto  num_err;
    if (c != '0') {                         /* Decimal              */
        base = 10;
    } else if ((c = *cp++ & UCHARMAX) == 'x' || c == 'X') {
        base = 16;                          /* Hexadecimal          */
        c = *cp++ & UCHARMAX;
    } else if (c == EOS) {                  /* 0                    */
        return  & ev;
    } else {                                /* Octal or illegal     */
        base = 8;
    }

    v = v1 = 0L;
    for (;;) {
        c1 = c;
        if (isupper( c1))
            c1 = tolower( c1);
        if (c1 >= 'a')
            c1 -= ('a' - 10);
        else
            c1 -= '0';
        if (c1 < 0 || base <= c1)
            break;
        v1 *= base;
        v1 += c1;
#if HAVE_LONG_LONG && MODE == STANDARD
        if (!stdc3) {
            if (v1 > ULONGMAX) {
                if (! skip)
                    goto  range_err;
                else
                    erange = TRUE;
            }
        } else
#endif
        if (v1 / base < v) {                /* Overflow             */
            if (! skip)
                goto  range_err;
            else
                erange = TRUE;
        }
        v = v1;
        c = *cp++ & UCHARMAX;
    }

    value = v;
    while (c == 'u' || c == 'U' || c == 'l' || c == 'L') {
        if (c == 'u' || c == 'U') {
#if MODE == PRE_STANDARD
            goto  num_err;
#else
            if (uflag)
                goto  num_err;
            uflag = TRUE;
#endif
        }
        if (c == 'l' || c == 'L') {
#if HAVE_LONG_LONG && MODE == STANDARD
            if (llflag) {
                goto  num_err;
            } else if (lflag) {
                if (!stdc3) {
                    goto  num_err;
                } else if ((c = *cp++) == 'l' || c == 'L') {
                    llflag = TRUE;
                    continue;
                } else {
                    break;
                }
            } else {
                lflag = TRUE;
            }
#else
            if (lflag)
                goto  num_err;
            else
                lflag = TRUE;
#endif
        }
        c = *cp++;
    }

    if (c != EOS)
        goto  num_err;

#if HAVE_UNSIGNED_LONG && MODE == STANDARD
    if (uflag)  /* If 'U' suffixed, uexpr_t is treated as unsigned  */
        ev.sign = UNSIGNED;
    else
#if HAVE_LONG_LONG
    {
        if (!stdc3)
            ev.sign = (value <= LONGMAX);
        else
            ev.sign = (value >= 0L);
    }
#else   /* ! HAVE_LONG_LONG */
        ev.sign = (value >= 0L);
#endif
#else   /* ! HAVE_UNSIGNED_LONG || MODE == PRE_STANDARD */
    ev.sign = (value >= 0L);
#endif

    ev.val = value;
    if (erange && (warn_level & 8))
        cwarn( out_of_range, nump, 0L, non_eval);
    return  & ev;

range_err:
    cerror( out_of_range, nump, 0L, NULLST);
    ev.sign = VAL_ERROR;
    return  & ev;
num_err:
    cerror( not_integer, nump, 0L, NULLST);
    ev.sign = VAL_ERROR;
    return  & ev;
}

static VAL_SIGN *
#if PROTO
eval_char( char * const token)
#else
eval_char( token)
    char *  token;
#endif
/*
 * Evaluate a character constant.
 * This routine is never called in POST_STD mode.
 */
{
#if MODE == STANDARD
    const char * const  w_out_of_range
        = "Wide character constant %s%.0ld%s is out of range";  /* _E_ _W8_ */
#endif
    const char * const  c_out_of_range
    = "Integer character constant %s%.0ld%s is out of range";   /* _E_ _W8_ */
#if HAVE_UNSIGNED_LONG && MODE == STANDARD
    register uexpr_t    value;
    uexpr_t         tmp;
#else
    register expr_t     value;
    expr_t          tmp;
#endif
    int             erange = FALSE;
    expr_t          cl;
    int             wide = (*token == 'L');
    int             ucn8;
    int             i;
    int             bits, mbits, u8bits, bits_save;
    char *          cp = token + 1;     /* Character content        */

    bits = CHARBIT;
    u8bits = CHARBIT * 4;
    if (mbchar & UTF8)
        mbits = CHARBIT * 4;
    else
        mbits = CHARBIT * 2;
#if MODE == STANDARD
    if (wide) {                         /* Wide character constant  */
        cp++;                           /* Skip 'L'                 */
        bits = mbits;
    }
#endif
    if (type[ *cp & UCHARMAX] & mbstart) {
        cl = mb_eval( &cp);
        bits = mbits;
    } else if ((cl = eval_one( &cp, wide, mbits, (ucn8 = FALSE, &ucn8)))
                == -1L) {
        ev.sign = VAL_ERROR;
        return  & ev;
    }
    bits_save = bits;
    value = cl;

    for (i = 0; *cp != '\'' && *cp != EOS; i++) {
        if (type[ *cp & UCHARMAX] & mbstart) {
            cl = mb_eval( &cp);
            if (cl == 0)
                /* Shift-out sequence of multi-byte or wide character   */
                continue;
            bits = mbits;
        } else {
            cl = eval_one( &cp, wide, mbits, (ucn8 = FALSE, &ucn8));
            if (cl == -1L) {
                ev.sign = VAL_ERROR;
                return  & ev;
            }
#if MODE == STANDARD && OK_UCN
            if (ucn8 == TRUE)
                bits = u8bits;
            else
                bits = bits_save;
#endif
        }
        tmp = value;
        value = (value << bits) | cl;   /* Multi-char or multi-byte char    */
#if HAVE_LONG_LONG && MODE == STANDARD
        if ((!stdc3 && value > ULONGMAX)
                || ((value >> bits) < tmp))
#else
        if ((value >> bits) < tmp)
#endif
        {                               /* Overflow                 */
            if (! skip)
                goto  range_err;
            else
                erange = TRUE;
        }
    }

#if HAVE_UNSIGNED_LONG && MODE == STANDARD
#if HAVE_LONG_LONG
    if (!stdc3)
        ev.sign = (value <= LONGMAX);
    else
        ev.sign = ((expr_t) value >= 0L);
#else
    ev.sign = ((expr_t) value >= 0L);
#endif
#else   /* ! HAVE_UNSIGNED_LONG || MODE == PRE_STANDARD */
    ev.sign = ((expr_t) value >= 0L);
#endif
    ev.val = value;

    if (erange && (warn_level & 8)) {
#if MODE == STANDARD
        if (wide)
            cwarn( w_out_of_range, token, 0L, non_eval);
        else
            cwarn( c_out_of_range, token, 0L, non_eval);
#else
        cwarn( c_out_of_range, token, 0L, non_eval);
#endif
    }

    if (i == 0)         /* Constant of single (character or wide-character) */
        return  & ev;

    if ((! skip && (warn_level & 4)) || (skip && (warn_level & 8))) {
#if MODE == STANDARD
        if (wide)
            cwarn(
"Multi-character wide character constant %s%.0ld%s isn't portable"  /* _W4_ _W8_    */
                    , token, 0L, skip ? non_eval : NULLST);
        else
            cwarn(
"Multi-character or multi-byte character constant %s%.0ld%s isn't portable" /* _W4_ _W8_    */
                    , token, 0L, skip ? non_eval : NULLST);
#else
        cwarn(
"Multi-character or multi-byte character constant %s%.0ld%s isn't portable" /* _W4_ _W8_    */
                    , token, 0L, skip ? non_eval : NULLST);
#endif
    }
    return  & ev;

range_err:
#if MODE == STANDARD
    if (wide)
        cerror( w_out_of_range, token, 0L, NULLST);
    else
        cerror( c_out_of_range, token, 0L, NULLST);
    ev.sign = VAL_ERROR;
#else
    cerror( c_out_of_range, token, 0L, NULLST);
    ev.sign = VAL_ERROR;
#endif
    return  & ev;
}

static expr_t
#if PROTO
eval_one( char ** seq_pp, int wide, int mbits, int * ucn8)
#else
eval_one( seq_pp, wide, mbits, ucn8)
    char **     seq_pp;         /* Address of pointer to sequence   */
                    /* eval_one() advances the pointer to sequence  */
    int     wide;                       /* Flag of wide-character   */
    int     mbits;              /* Number of bits of a wide-char    */
    int *   ucn8;                           /* Flag of UCN-32 bits  */
#endif
/*
 * Called from eval_char() above to get a single character, single multi-
 * byte character or wide character (with or without \ escapes).
 * Returns the value of the character or -1L on error.
 */
{
#if MODE == STANDARD && OK_UCN
    const char * const  ucn_malval
        = "UCN cannot specify the value %.0s\"%08lx\""; /* _E_ _W8_ */
#endif
    const char * const  out_of_range
        = "%s%ld bits can't represent escape sequence '%s'";    /* _E_ _W8_ */
#if HAVE_UNSIGNED_LONG && MODE == STANDARD
    uexpr_t         value;
#else
    expr_t          value;
#endif
    int             erange = FALSE;
    char *          seq = *seq_pp;  /* Initial seq_pp for diagnostic*/
    const char *    cp;
    const char *    digits;
    register unsigned       uc;
    unsigned        uc1;
    int             count;
    int             bits;
    size_t          wchar_max;

    uc = *(*seq_pp)++ & UCHARMAX;

    if (uc != '\\')                 /* Other than escape sequence   */
        return  (expr_t) uc;

    /* escape sequence  */
    uc1 = uc = *(*seq_pp)++ & UCHARMAX;
    switch (uc) {
    case 'a':                               /* New in Standard      */
#ifdef ALERT
        return  ALERT;                      /* Use predefined value */
#else
        return  '\a';                       /* Use compiler's value */
#endif
    case 'b':
        return  '\b';
    case 'f':
        return  '\f';
    case 'n':
        return  '\n';
    case 'r':
        return  '\r';
    case 't':
        return  '\t';
    case 'v':                               /* New in Standard      */
#ifdef  VT
        return  VT;                         /* Use predefined value */
#else
        return  '\v';                       /* Use compiler's value */
#endif
#if MODE == STANDARD
#if OK_UCN
    case 'u':   case 'U':
        if (!stdc2)
            goto  undefined;
        /* Else Universal character name    */
        /* Fall through */
#endif
    case 'x':                               /* '\xFF'               */
        digits = "0123456789abcdef";
        bits = 4;
        uc = *(*seq_pp)++ & UCHARMAX;
        break;
#endif  /* MODE == STANDARD    */
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
        digits = "01234567";
        bits = 3;
        break;
    case '\'':  case '"':   case '?':   case '\\':
        return  (expr_t) uc;
    default:
        goto  undefined;
    }

    wchar_max = (UCHARMAX << CHARBIT) | UCHARMAX;
    if (mbits == CHARBIT * 4)
        wchar_max = (wchar_max << CHARBIT * 2) | wchar_max;

    value = 0L;
    for (count = 0; ; ++count) {
        if (isupper( uc))
            uc = tolower( uc);
        if ((cp = strchr( digits, uc)) == NULL)
            break;
        if (count >= 3 && bits == 3)
            break;      /* Octal escape sequence at most 3 digits   */
#if MODE == STANDARD && OK_UCN
        if ((count >= 4 && uc1 == 'u') || (count >= 8 && uc1 == 'U'))
            break;
#endif
        value = (value << bits) | (cp - digits);
#if MODE == STANDARD && OK_UCN
        if (wchar_max < value && uc1 != 'u' && uc1 != 'U')
#else
        if (wchar_max < value)
#endif
            {
            if (! skip)
                goto  range_err;
            else
                erange = TRUE;
        }
        uc = *(*seq_pp)++ & UCHARMAX;
    }
    (*seq_pp)--;

    if (erange) {
        value &= wchar_max;
        goto  range_err;
    }

#if MODE == STANDARD
    if (count == 0 && bits == 4)            /* '\xnonsense'         */
        goto  undefined;
#if OK_UCN
    if (uc1 == 'u' || uc1 == 'U') {
        if ((count < 4 && uc1 == 'u') || (count < 8 && uc1 == 'U'))
            goto  undefined;
        if ((value >= 0L && value <= 0x9FL
                    && value != 0x24L && value != 0x40L && value != 0x60L)
                || (!stdc3 && value >= 0xD800L && value <= 0xDFFFL)) {
            if (!skip)
                cerror( ucn_malval, NULLST, (long) value, NULLST);
            else if (warn_level & 8)
                cwarn( ucn_malval, NULLST, (long) value, NULLST);
        }
        if (count >= 8 && uc1 == 'U')
            *ucn8 = TRUE;
        return  (expr_t) value;
    }
#endif  /* OK_UCN   */
    if (! wide)
        if (UCHARMAX < value) {
            value &= UCHARMAX;
            goto  range_err;
        }
#else   /* MODE == PRE_STANDARD */
    if (UCHARMAX < value) {
        value &= UCHARMAX;
        goto  range_err;
    }
#endif
    return  (expr_t) value;

undefined:
    uc1 = **seq_pp;
    **seq_pp = EOS;                         /* For diagnostic       */
    if ((! skip && (warn_level & 1)) || (skip && (warn_level & 8)))
        cwarn(
    "Undefined escape sequence%s %.0ld'%s'"         /* _W1_ _W8_    */
                , skip ? non_eval : NULLST, 0L, seq);
    **seq_pp = uc1;
    *seq_pp = seq + 1;
    return  (expr_t) '\\';              /* Returns the escape char  */

range_err:
    uc1 = **seq_pp;
    **seq_pp = EOS;                         /* For diagnostic       */
#if MODE == STANDARD
    if (wide) {
        if (! skip)
            cerror( out_of_range, NULLST, (long) mbits, seq);
        else if (warn_level & 8)
            cwarn( out_of_range, non_eval, (long) mbits, seq);
    } else {
        if (! skip)
            cerror( out_of_range, NULLST, (long) CHARBIT, seq);
        else if (warn_level & 8)
            cwarn( out_of_range, non_eval, (long) CHARBIT, seq);
    }
#else   /* MODE == PRE_STANDARD */
    if (! skip)
        cerror( out_of_range, NULLST, (long) CHARBIT, seq);
    else if (warn_level & 8)
        cwarn( out_of_range, non_eval, (long) CHARBIT, seq);
#endif

    **seq_pp = uc1;
    if (! skip)
        return  -1L;
    else
        return  (expr_t) value;
}

static VAL_SIGN *
#if PROTO
eval_eval( VAL_SIGN * valp, int op)
#else
eval_eval( valp, op)
    VAL_SIGN *      valp;
    int             op;
#endif
/*
 * One or two values are popped from the value stack and do arithmetic.
 * The result is pushed onto the value stack.
 * eval_eval() returns the new pointer to the top of the value stack.
 */
{
    const char * const  zero_div = "%sDivision by zero%.0ld%s"; /* _E_ _W8_ */
    const char * const  neg_long =
"Negative value \"%ld\" is converted to positive \"%lu\"%%s";   /* _W1_ _W8_*/
#if HAVE_LONG_LONG && MODE == STANDARD
    const char * const  neg_llong =
"Negative value \"%" LL_FORM "d\" is converted to positive \"%" /* _W1_ _W8_*/
                    LL_FORM "u\"%%s";
#endif
    register expr_t     v1, v2;
    int     sign1, sign2;

    if (is_binary( op)) {
        v2 = (--valp)->val;
        sign2 = valp->sign;
    } else {
        v2 = 0L;                            /* Dummy    */
        sign2 = SIGNED;                     /* Dummy    */
    }
    v1 = (--valp)->val;
    sign1 = valp->sign;
#if DEBUG_EVAL
    if (debug & EXPRESSION) {
        fprintf( fp_debug, "%s op %s", (is_binary( op)) ? "binary" : "unary"
                , opname[ op]);
        dump_val( ", v1 = ", valp);
        if (is_binary( op))
            dump_val( ", v2 = ", valp + 1);
        fputc( '\n', fp_debug);
    }
#endif

#if HAVE_UNSIGNED_LONG && MODE == STANDARD  /* Usual arithmetic conversion  */
    if ((sign1 == UNSIGNED || sign2 == UNSIGNED) && is_binary( op)
            && op != OP_ANA && op != OP_ORO && op != OP_SR && op != OP_SL) {
#if HAVE_LONG_LONG
        if (((sign1 == SIGNED && v1 < 0L) || (sign2 == SIGNED && v2 < 0L)
                    || (!stdc3 && ((sign1 == SIGNED && v1 > LONGMAX)
                                 || (sign2 == SIGNED && v2 > LONGMAX)))
                ) && ((! skip && (warn_level & 1))
                    || (skip && (warn_level & 8)))) {
            char    negate[(((sizeof (expr_t) * 8) / 3) + 1) * 2 + 50];
            expr_t  v3;

            /*
             * PLAN9_PCC fails to compile the next line.
             *      v3 = (sign1 == SIGNED ? v1 : v2);
             */
            if (sign1 == SIGNED)
                v3 = v1;
            else
                v3 = v2;
            if (stdc3) {
                sprintf( negate, neg_llong, v3, v3);
            } else {
                if (sign1 == SIGNED)
                    v1 = (unsigned long) v3;
                else
                    v2 = (unsigned long) v3;
                sprintf( negate, neg_long, (long) v3, (unsigned long) v3);
            }
            cwarn( negate, skip ? non_eval : NULLST, 0L, NULLST);
        }
#else   /* ! HAVE_LONG_LONG && HAVE_UNSIGNED_LONG && MODE == STANDARD   */
        if (((sign1 == SIGNED && v1 < 0L) || (sign2 == SIGNED && v2 < 0L))
                && ((! skip && (warn_level & 1))
                || (skip && (warn_level & 8)))) {
            char    negate[(((sizeof (expr_t) * 8) / 3) + 1) * 2 + 50];
            expr_t  v3 = (v1 < 0L ? v1 : v2);

            sprintf( negate, neg_long, (long) v3, (unsigned long) v3);
            cwarn( negate, skip ? non_eval : NULLST, 0L, NULLST);
        }
#endif
        valp->sign = sign1 = sign2 = UNSIGNED;
    }
#endif
    if ((op == OP_SL || op == OP_SR)
            && (v2 < 0L || v2 >= sizeof (expr_t) * CHARBIT
#if HAVE_LONG_LONG && MODE == STANDARD
                || (!stdc3 && v2 >= sizeof (long) * CHARBIT)
#endif
            ) && ((!skip && (warn_level & 1)) || (skip && (warn_level & 8))))
        cwarn( "Illegal shift count %.0s\"%ld\"%s"          /* _W1_ _W8_    */
                , NULLST, (long) v2, skip ? non_eval : NULLST);
    if ((op == OP_DIV || op == OP_MOD) && v2 == 0L) {
        if (! skip) {
            cerror( zero_div, NULLST, 0L, NULLST);
            valp->sign = VAL_ERROR;
            return  valp;
        } else {
            if (warn_level & 8)
                cwarn( zero_div, NULLST, 0L, non_eval);
            valp->sign = sign1;
            valp->val = (expr_t) EXPR_MAX;
            return  valp;
        }
    }

#if HAVE_UNSIGNED_LONG && MODE == STANDARD
    if (sign1 == SIGNED)
        v1 = eval_signed( & valp, v1, v2, op);
    else
        v1 = eval_unsigned( & valp, (uexpr_t) v1, (uexpr_t) v2, op);
#else
    v1 = eval_signed( & valp, v1, v2, op);
#endif

    if (valp->sign == VAL_ERROR)                /* Out of range */
        return  valp;

    switch (op) {
    case OP_NOT:    case OP_EQ:     case OP_NE:
    case OP_LT:     case OP_LE:     case OP_GT:     case OP_GE:
    case OP_ANA:    case OP_ORO:
        valp->sign = SIGNED;
        break;
    default:
        valp->sign = sign1;
        break;
    }
    valp->val = v1;
    return  valp;
}

static expr_t
#if PROTO
eval_signed( VAL_SIGN ** valpp, expr_t v1, expr_t v2, int op)
#else
eval_signed( valpp, v1, v2, op)
    VAL_SIGN ** valpp;
    expr_t      v1, v2;
    int         op;
#endif
/*
 * Apply the argument operator to the signed data.
 * OP_COL is a special case.
 */
{
#if DEBUG_EVAL
    const char * const   illeg_op
        = "Bug: Illegal operator \"%s\" in eval_signed()";      /* _F_  */
#endif
    const char * const  not_portable
        = "\"%s\" of negative number isn't portable%.0ld%s";    /* _W1_ _W8_*/
    const char *    op_name = opname[ op];
    VAL_SIGN *      valp = *valpp;
    expr_t  val;

    switch (op) {
    case OP_EOE:
    case OP_PLU:                        break;
    case OP_NEG:
        if ((v1 && v1 == -v1)
#if HAVE_LONG_LONG && MODE == STANDARD
                || (!stdc3 && (long) v1 == (long) -v1)
#endif
            )
            overflow( op_name, valpp);
        v1 = -v1;
        break;
    case OP_COM:    v1 = ~v1;           break;
    case OP_NOT:    v1 = !v1;           break;
    case OP_MUL:
        val = v1 * v2;
        if (v1 && v2) {
            if ((val / v1 != v2 || val / v2 != v1)
#if HAVE_LONG_LONG && MODE == STANDARD
                    || (!stdc3 && ((long)val / (long)v1 != (long)v2
                                 || (long)val / (long)v2 != (long)v1))
#endif
                )
                overflow( op_name, valpp);
        }
        v1 = val;
        break;
    case OP_DIV:
    case OP_MOD:
        /* Division by 0 has been already diagnosed by eval_eval().  */
        if ((-v1 == v1 && v2 == -1)     /* LONG_MIN / -1 on two's complement*/
#if HAVE_LONG_LONG && MODE == STANDARD
                || (!stdc3 && (long)-v1 == (long)v1 && (long)v2 == (long)-1)
#endif
            )
            overflow( op_name, valpp);
        else if ((v1 < 0L || v2 < 0L) && ((!skip && (warn_level & 1))
                || (skip && (warn_level & 8))))
            cwarn( not_portable, op_name, 0L, skip ? non_eval : NULLST);
        if (op == OP_DIV)
            v1 /= v2;
        else
            v1 %= v2;
        break;
    case OP_ADD:
        val = v1 + v2;
        if ((v2 > 0L && v1 > val) || (v2 < 0L && v1 < val)
#if HAVE_LONG_LONG && MODE == STANDARD
                || (!stdc3 && (((long)v2 > 0L && (long)v1 > (long)val)
                             || ((long)v2 < 0L && (long)v1 < (long)val)))
#endif
            )
            overflow( op_name, valpp);
        v1 = val;
        break;
    case OP_SUB:
        val = v1 - v2;
        if (((v2 > 0L && val > v1) || (v2 < 0L && val < v1))
#if HAVE_LONG_LONG && MODE == STANDARD
                || (!stdc3 && (((long)v2 > 0L && (long)val > (long)v1)
                             || ((long)v2 < 0L && (long)val < (long)v1)))
#endif
            )
            overflow( op_name, valpp);
        v1 = val;
        break;
    case OP_SL:     v1 <<= v2;          break;
    case OP_SR:
        if (v1 < 0L
                && ((!skip && (warn_level & 1))
                    || (skip && (warn_level & 8))))
            cwarn( not_portable, op_name, 0L, skip ? non_eval : NULLST);
        v1 >>= v2;
        break;
    case OP_LT:     v1 = (v1 < v2);     break;
    case OP_LE:     v1 = (v1 <= v2);    break;
    case OP_GT:     v1 = (v1 > v2);     break;
    case OP_GE:     v1 = (v1 >= v2);    break;
    case OP_EQ:     v1 = (v1 == v2);    break;
    case OP_NE:     v1 = (v1 != v2);    break;
    case OP_AND:    v1 &= v2;           break;
    case OP_XOR:    v1 ^= v2;           break;
    case OP_OR:     v1 |= v2;           break;
    case OP_ANA:    v1 = (v1 && v2);    break;
    case OP_ORO:    v1 = (v1 || v2);    break;
    case OP_COL:
        /*
         * If v1 has the "true" value, v2 has the "false" value.
         * The top of the value stack has the test.
         */
        /*
         * PLAN9_PCC fails to compile the next line.
         *      v1 = (--*valpp)->val ? v1 : v2;
         */
        valp--;
        if (valp->val == 0)
            v1 = v2;
        break;
#if DEBUG_EVAL
    default:
        cfatal( illeg_op, op_name, 0L, NULLST);
#endif
    }

    *valpp = valp;
    return  v1;
}

#if HAVE_UNSIGNED_LONG && MODE == STANDARD

static expr_t
#if PROTO
eval_unsigned( VAL_SIGN ** valpp, uexpr_t v1u, uexpr_t v2u, int op)
#else
eval_unsigned( valpp, v1u, v2u, op)
    VAL_SIGN **     valpp;
    uexpr_t     v1u, v2u;
    int         op;
#endif
/*
 * Apply the argument operator to the unsigned data.
 */
{
#if DEBUG_EVAL
    const char * const   illeg_op
        = "Bug: Illegal operator \"%s\" in eval_unsigned()";    /* _F_  */
#endif
    const char *    op_name = opname[ op];
    VAL_SIGN *      valp = *valpp;
    uexpr_t     v1;

    switch (op) {
    case OP_EOE:
    case OP_PLU:    v1 = v1u;           break;
    case OP_NEG:
        v1 = -v1u;
        if (v1u)
            overflow( op_name, valpp);
        break;
    case OP_COM:    v1 = ~v1u;          break;
    case OP_NOT:    v1 = !v1u;          break;
    case OP_MUL:
        v1 = v1u * v2u;
        if (v1u && v2u && ((v1 / v2u != v1u
                || v1 / v1u != v2u)
#if HAVE_LONG_LONG
                || (!stdc3 && v1 > ULONGMAX)
#endif
            ))
            overflow( op_name, valpp);
        break;
    case OP_DIV:
        /* Division by 0 has been already diagnosed by eval_eval().  */
        v1 = v1u / v2u;
        break;
    case OP_MOD:
        v1 = v1u % v2u;
        break;
    case OP_ADD:
        v1 = v1u + v2u;
        if ((v1 < v1u)
#if HAVE_LONG_LONG
                || (!stdc3 && v1 > ULONGMAX)
#endif
            )
            overflow( op_name, valpp);
        break;
    case OP_SUB:
        v1 = v1u - v2u;
        if ((v1 > v1u)
#if HAVE_LONG_LONG
                || (!stdc3 && v1 > ULONGMAX)
#endif
            )
            overflow( op_name, valpp);
        break;
    case OP_SL:     v1 = v1u << v2u;    break;
    case OP_SR:     v1 = v1u >> v2u;    break;
    case OP_LT:     v1 = (v1u < v2u);   break;
    case OP_LE:     v1 = (v1u <= v2u);  break;
    case OP_GT:     v1 = (v1u > v2u);   break;
    case OP_GE:     v1 = (v1u >= v2u);  break;
    case OP_EQ:     v1 = (v1u == v2u);  break;
    case OP_NE:     v1 = (v1u != v2u);  break;
    case OP_AND:    v1 = v1u & v2u;     break;
    case OP_XOR:    v1 = v1u ^ v2u;     break;
    case OP_OR:     v1 = v1u | v2u;     break;
    case OP_ANA:    v1 = (v1u && v2u);  break;
    case OP_ORO:    v1 = (v1u || v2u);  break;
    case OP_COL:    valp--;
        if (valp->val)
            v1 = v1u;
        else
            v1 = v2u;
        break;
#if DEBUG_EVAL
    default:
        cfatal( illeg_op, op_name, 0L, NULLST);
#endif
    }

    *valpp = valp;
    return  v1;
}

#endif

static void
#if PROTO
overflow( const char * op_name, VAL_SIGN ** valpp)
#else
overflow( op_name, valpp)
    char *  op_name;
    VAL_SIGN **     valpp;
#endif
{
    const char * const  out_of_range
        = "Result of \"%s\" is out of range%.0ld%s";    /* _E_ _W1_ _W8_    */

    if (skip) {
        if (warn_level & 8)
            cwarn( out_of_range, op_name, 0L, non_eval);
        /* Else don't warn  */
#if HAVE_UNSIGNED_LONG && MODE == STANDARD
    } else if ((*valpp)->sign == UNSIGNED) {     /* Never overflow  */
        if (warn_level & 1)
            cwarn( out_of_range, op_name, 0L, NULLST);
#endif
    } else {
        cerror( out_of_range, op_name, 0L, NULLST);
        (*valpp)->sign = VAL_ERROR;
    }
}

#if DEBUG_EVAL

static void
#if PROTO
dump_val( const char * msg, const VAL_SIGN * valp)
#else
dump_val( msg, valp)
    char *      msg;
    VAL_SIGN *  valp;
#endif
/*
 * Dump a value by internal representation.
 */
{
#if HAVE_LONG_LONG && MODE == STANDARD
    const char * const  format_ll
                = "%s(%ssigned long long) 0x%016" LL_FORM "x";
#endif
    const char * const  format = "%s(%ssigned long) 0x%08lx";
    int     sign = valp->sign;

#if HAVE_LONG_LONG && MODE == STANDARD
    if (stdc3) {
        fprintf( fp_debug, format_ll, msg, sign ? "" : "un", valp->val);
    } else {
        fprintf( fp_debug, format, msg, sign ? "" : "un", (long) valp->val);
    }
#else
    fprintf( fp_debug, format, msg, sign ? "" : "un", valp->val);
#endif
}

static void
#if PROTO
dump_stack( const OPTAB * opstack, const OPTAB * opp, const VAL_SIGN * value
        , const VAL_SIGN * valp)
#else
dump_stack( opstack, opp, value, valp)
    OPTAB *         opstack;        /* Operator stack               */
    register OPTAB *    opp;        /* Pointer into operator stack  */
    VAL_SIGN *      value;          /* Value stack                  */
    VAL_SIGN *      valp;           /* -> value vector              */
#endif
/*
 * Dump stacked operators and values.
 */
{
    if (opstack < opp)
        fprintf( fp_debug, "Index op prec skip name -- op stack at %s"
                , infile->bptr);

    while (opstack < opp) {
        fprintf( fp_debug, " [%2d] %2d %04o    %d %s\n", (int)(opp - opstack)
                , opp->op, opp->prec, opp->skip, opname[ opp->op]);
        opp--;
    }

    while (value <= --valp) {
        fprintf( fp_debug, "value[%d].val = ", (int)(valp - value));
        dump_val( "", valp);
        fputc( '\n', fp_debug);
    }
}

#endif  /* DEBUG_EVAL   */

