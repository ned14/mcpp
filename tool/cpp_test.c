/*
 * cpp_test.c:  to compile and run n_*.c, i_*.c;
 *              make a summary of the results.
 *      1998/08 by kmatsui
 *              Made after runtest.c and summtest.c of
 *              "Plum-Hall Validation Sampler".
 *      2005/03 by kmatsui
 *              Decreased 'PGNUM' by 1.
 */

#include    "stdio.h"
#include    "stdlib.h"
#include    "string.h"
#include    "ctype.h"
#include    "errno.h"

#define NAMEMAX     8
#define PGMNUM      37

#if     __MSDOS__ || __WIN32__ || _WIN32
#define PATH_DELIM  '\\'
#else
#define PATH_DELIM  '/'
#endif

#if 0
extern int  sleep();
#endif

void    test_cpp();
void    sum_test();
void    usage();

char    comp_fmt[ BUFSIZ/2] = "cc -ansi -o%s %s.c";
char    cmp_name[ NAMEMAX+1];
char    out_file[ NAMEMAX+5];
char    err_file[ NAMEMAX+5];
char    sum_file[ NAMEMAX+5];
char    err_name[ PGMNUM][ NAMEMAX+5];
char    buf[ BUFSIZ];

main(argc, argv)
    int     argc;
    char    *argv[];
{
    char    *extender;
    char    *cp;
    int     i;
    int     ac = 1;

    if (argc < 2)
        usage();
    if (ac < argc && (argv[ac][0] == '-' || argv[ac][0] == '/')) {
        if (argv[ac][1] == '?')
            usage();
        else
            extender = argv[ac++] + 1;
    } else {
        extender = NULL;
    }
    if (argc <= ac || (! isalpha( argv[ac][ 0]) && argv[ac][0] != '_')
            || strchr( argv[ac], '.') != NULL
            || strlen( argv[ac]) > NAMEMAX) {
        usage();
    }
    strcpy( cmp_name, argv[ac]);
    sprintf( sum_file, "%s.sum", cmp_name);
    sprintf( out_file, "%s.out", cmp_name);
    sprintf( err_file, "%s.err", cmp_name);
    for (cp = cmp_name; (i = *cp) != 0; cp++) {
        if (i == '-' || i == '_' || i == '~')
            *cp = '|';      /* Convert horizontal line to vertical line */
    }
    if (++ac < argc)
        strcpy( comp_fmt, argv[ac]);
    ac++;
    if (freopen( out_file, "w", stdout) == NULL)
        usage();
    if (freopen( err_file, "w", stderr) == NULL)
        usage();
    setbuf( stdout, NULL);
    setbuf( stderr, NULL);

    test_cpp( argc, argv, ac, extender);
    sum_test();

    return  0;
}

void    test_cpp( argc, argv, ac, extender)
    int     argc;
    char    **argv;
    int     ac;
    char    *extender;
{
    int     i, len;
    int     pgm_num;

    for (pgm_num = 0; pgm_num < PGMNUM && ! feof( stdin); pgm_num++) {
        /* for each input program   */
        char    pgm_name[ NAMEMAX+2];

        fgets( pgm_name, NAMEMAX+1, stdin);
        if ((len = strlen( pgm_name)) < 2 || *(pgm_name + len - 1) != '\n')
            continue;           /* skip erroneous line      */
        else
            *(pgm_name + len - 1) = '\0';   /* remove '\n'  */

        sprintf( buf, comp_fmt, pgm_name, pgm_name);
        system( buf);
        printf( "COMPILE:    %s\n", buf);
#if 0
        sleep( 2);                      /* Wait a moment    */
#endif
        sprintf( err_name[ pgm_num], "%s.err", pgm_name);
        freopen( err_name[ pgm_num], "w", stderr);
        if (extender)
            sprintf( buf, "%s .%c%s", extender, PATH_DELIM, pgm_name);
        else
            sprintf( buf, ".%c%s", PATH_DELIM, pgm_name);
        system( buf);
        printf( "EXECUTE:    %s\n", buf);
#if 0
        sleep( 2);                      /* Wait a moment    */
#endif
        freopen( err_file, "a", stderr);
        for (i = ac; i < argc; ++i) {
            sprintf( buf, argv[i], pgm_name, pgm_name);
            system( buf);
            printf( "CLEANUP:    %s\n", buf);
        }
    }   /* end loop over each program   */
}

void    sum_test()
{
    FILE    *sumfp, *errfp;
    char    *cp;
    int     i, len;
    int     nerror = 0;
    int     pgm_num;

    /* Make a column of summary */
    if ((sumfp = fopen( sum_file, "w")) == NULL) {
        fprintf( stderr, "Can't open %s\n", sum_file);
        exit( errno);
    }
    len = strlen( cmp_name);
    for (i = 0; i < len; i++)
        fprintf( sumfp, "  %c\n", cmp_name[i]);
    for (i = len; i <= NAMEMAX; i++)
        fputs( "   \n", sumfp);

    for (pgm_num = 0; pgm_num < PGMNUM; pgm_num++) {
        if ((errfp = fopen( err_name[ pgm_num], "r")) == NULL) {
            fputs( "  -\n", sumfp);
            nerror++;
        } else {
            while ((cp = fgets( buf, BUFSIZ, errfp)) != NULL
                    && strcmp( buf, "started\n") != 0)
                ;           /* To work around the message of "go32" */
            if (cp == NULL) {
                fputs( "  -\n", sumfp);
                nerror++;
            } else {
                fgets( buf, BUFSIZ, errfp);
                if (strcmp( "success\n", buf) != 0) {
                    fputs( "  o\n", sumfp);
                    nerror++;
                } else {
                    fputs( "  *\n", sumfp);
                }
            }
            fclose( errfp);
        }
    }

    fprintf( sumfp, "\n%3d\n", nerror);
}

void    usage( void)
{
    fputs( "Usage:\n", stderr);
    fputs(
"  cpp_test [-extender] compiler-name \"compile command\" \"cleaning command\"s\n"
            , stderr);
    fputs( "  Option specifies the name of DOS-extender (eg. -go32).\n",
            stderr);
    fprintf( stderr,
        "  Compiler-name must be %d bytes or less and must be without dot.\n",
            NAMEMAX);
    fputs( "  Number of \"cleaning command\"s can be any.\n", stderr);
    fputs(
"Example: cpp_test GNUC_272 \"cc -ansi -o%s %s.c\" \"rm %s\" < n_i_.lst\n"
            , stderr);
    exit( 0);
}

