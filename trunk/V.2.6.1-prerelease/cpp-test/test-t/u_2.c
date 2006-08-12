/* u_2.c:   Undefined behaviors on undefined constant expression.   */

/* { dg-do preprocess } */

/* u.2.1:   Undefined escape sequence.  */
#if     '\x'    /* { dg-error "no following hex digits| Undefined escape sequence '\\\\x'\n\[^ \]* warning:" } */
#endif

/* u.2.2:   Illegal bit shift count.    */
#if     1 << -1 /* { dg-error "(i|I)llegal shift count" } */
#endif
#if     1 << 64 /* { dg-error "integer overflow | Illegal shift count" } */
#endif

