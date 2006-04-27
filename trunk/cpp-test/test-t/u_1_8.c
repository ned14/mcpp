/* u_1_8.c:     Undefined behaviors on unterminated quotations. */

/* { dg-do preprocess } */

/* u.1.8:   Unterminated character constant.    */
/*  The following "comment" may not interpreted as a comment but swallowed by
        the unterminated character constant.    */
#error  I can't understand. /* Token error prior to execution of #error.    */
/* { dg-error "missing terminating ' character| unterminated string or character constant| Unterminated character constant" "" { target *-*-* } 8 } */

/* u.1.9:   Unterminated string literal.    */
    "String literal
    across the lines.
"
/* { dg-error "multi-line string literals are deprecated| string constant runs past end of line| Unterminated string literal, skipped the line\n\[\^ \]* error: Unterminated" "" { target *-*-* } 12 } */

/* u.1.10:  Unterminated header-name.   */
#include    <assert.h
/* { dg-error "missing terminating > character| (`#include'|#include) expects | Unterminated header name" "" { target *-*-* } 18 } */

