/* i_36_jis.c:  Handling of '\\' in ISO-2022-JP multi--byte character.  */

#include    "defs.h"

#define     str( a)     # a

main( void)
{
    fputs( "started\n", stderr);

/* 36.1:    0x5c in multi-byte character is not an escape character */

#pragma __setlocale( "jis")                 /* For MCPP     */
#pragma setlocale( "jis")                   /* For MCPP on VC   */

#if     '字' == '\x3b\x7a' && '移' != '\x30\x5c'
    fputs( "Bad handling of '\\' in multi-byte character", stderr);
    exit( 1);
#endif

/* 36.2:    # operater should not insert '\\' before 0x5c, 0x22 or 0x27
        in multi-byte character */
    assert( strcmp( str( "移動"), "\"移動\"") == 0);
    assert( strcmp( str( "陰陽"), "\"陰陽\"") == 0);
    assert( strcmp( str( "宇宙"), "\"宇宙\"") == 0);

    fputs( "success\n", stderr);
    return  0;
}

