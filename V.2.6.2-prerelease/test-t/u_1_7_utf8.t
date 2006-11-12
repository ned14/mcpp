/* u_1_7_utf8.t:    Invalid multi-byte character sequence (in string literal,
        character constant, header-name or comment).    */

#define str( a)     # a
#pragma __setlocale( "utf8")                /* For MCPP     */

    str( "à¡p");   /* 0xe0a170 */

