/* e_24_5.c:    Operand of # operator in function-like macro definition shall
        be a parameter name.    */

/* { dg-do preprocess } */

/* 24.5:    */
#define FUNC( a)    # b     /* { dg-error "not followed by a macro parameter| should be followed by a macro argument name| Not a formal parameter" } */

