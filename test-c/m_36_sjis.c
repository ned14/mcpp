/* m_36_sjis.c: Handling of '\\' in shift-JIS multi-byte character. */

#include    "defs.h"

#define     str( a)     # a

main( void)
{
    fputs( "started\n", stderr);

/* 36.1:    0x5c in multi-byte character is not an escape character */

#pragma __setlocale( "sjis")                /* For MCPP     */
#pragma setlocale( "japanese")              /* For Visual C */

#if     'Žš' == '\x8e\x9a' && '•\' != '\x95\x5c'
    fputs( "Bad handling of '\\' in multi-byte character", stderr);
    exit( 1);
#endif

/* 36.2:    # operater should not insert '\\' before 0x5c in multi-byte
        character   */
    assert( strcmp( str( "•\Ž¦"), "\"•\Ž¦\"") == 0);

    fputs( "success\n", stderr);
    return  0;
}

