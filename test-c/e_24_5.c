/* e_24_5.c:    Operand of # operator in function-like macro definition should
        be a parameter. */

/* 24.5:    */
#define FUNC( a)    # b

main( void)
{
    return  0;
}

