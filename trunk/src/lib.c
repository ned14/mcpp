/*
 * Redistribution and use of this code in source and binary forms, with or
 * without modification, are freely permitted.              [Kiyoshi Matsui]
 */

/*
 *                              L I B . C
 *                  L i b r a r y   R o u t i n e s
 *
 * Some variation of standard library functions.
 * Some standard functions for the library which has not those or the library
 *      which has only non-conforming ones.
 *
 * 1998/08      First released.                     kmatsui
 * 2003/11      Added strstr() and strcspn().
 *              Uses macros defined by configure.   kmatsui
 */

#if 1
#if PREPROCESSED
#include    "mcpp.H"
#else
#include    "system.H"  /* For PROTO, UCHARMAX and function declarations    */
#endif
#endif

#if ! HOST_HAVE_GETOPT || HOST_LIB_IS_GLIBC

/*
 * Note: The getopt() of glibc should not be used since the specification
 *  differs from the standard one.
 *  Use this getopt() for this cpp.
 */

/* Based on the public-domain-software released by AT&T.    */

#define OPTERR( s, c)   if (opterr) {   \
    fputs( argv[0], stderr);    \
    fputs( s, stderr);          \
    putc( c, stderr);           \
    putc( '\n', stderr);        \
    }

int     optind = 1;
int     opterr = 1;
int     optopt;
char *  optarg;

int
#if PROTO
getopt( int argc, char * const * argv, const char * opts)
#else
getopt( argc, argv, opts)
    int         argc;
    char **     argv;
    char *      opts;
#endif
/*
 * Get the next option (and it's argument) from the command line.
 */
{
    const char * const   error1 = ": option requires an argument --";
    const char * const   error2 = ": illegal option --";
    static int      sp = 1;
    register int    c;
    register char *     cp;

    if (sp == 1) {
        if (argc <= optind ||
                argv[ optind][ 0] != '-' || argv[ optind][ 1] == '\0') {
            return  EOF;
        } else if (strcmp( argv[ optind], "--") == 0) {
            optind++;
            return  EOF;
        }
    }
/*  optopt = c = (unsigned char) argv[ optind][ sp];    */
    optopt = c = argv[ optind][ sp] & UCHARMAX;
    if (c == ':' || (cp = strchr( opts, c)) == NULL) {
        OPTERR( error2, c)
        if (argv[ optind][ ++sp] == '\0') {
            optind++;
            sp = 1;
        }
        return  '?';
    }
    if (*++cp == ':') {
        if (argv[ optind][ sp+1] != '\0') {
            optarg = &argv[ optind++][ sp+1];
        } else if (argc <= ++optind) {
            OPTERR( error1, c)
            sp = 1;
            return  '?';
        } else {
            optarg = argv[ optind++];
        }
        sp = 1;
    } else {
        if (argv[ optind][ ++sp] == '\0') {
            sp = 1;
            optind++;
        }
        optarg = NULL;
    }
    return  c;
}

#endif

#if ! HOST_HAVE_SANE_FGETS

char *
#if PROTO
fgets( char * buf, int size, FILE * fp)
#else
fgets( buf, size, fp)
    char *  buf;
    int     size;
    FILE *  fp;
#endif
{
    int     c;
    char *  cp = buf;

    if (size <= 1)
        return  NULL;
    while (--size && (c = getc( fp)) != EOF && c != '\n')
        *cp++ = c;
    if (c != EOF || buf < cp) {
        if (c == '\n')
            *cp++ = '\n';
        *cp = '\0';
        return  buf;
    } else {
        return  NULL;
    }
}

#endif

#if ! HOST_HAVE_STPCPY

char *
#if PROTO
stpcpy( char * dest, const char * src)
#else
stpcpy( dest, src)
    char *  dest;
    char *  src;
#endif
/*
 * Copy the string and return the advanced pointer.
 */
{
    const char * s;
    char *  d;

    for (s = src, d = dest; (*d++ = *s++) != '\0'; )
        ;
    return  d - 1;
}

#endif

#if ! HOST_HAVE_STRSTR

char *
#if PROTO
strstr( const char * src, const char * pat)
#else
strstr( src, pat)
    char *  src;
    char *  pat;
#endif
{
    const char  *s1, *s2, *p;

    if (*src == '\0' || *pat == '\0')       /* No element specified */
        return  (char *)src;
    for (s1 = src; *s1 != '\0'; s1++)
        if (*s1 == *pat)            /* Matched the first element    */
            break;                          /* Else *s1 == '\0'     */
    for (s2 = s1, p = pat; *s2 != '\0', *p != '\0'; s2++, p++)
        if (*s2 != *p)                      /* Unmatched the rest   */
            break;                          /* Else *p == '\0'      */
    return  *p ? NULL : (char *)s1;
}

#endif

#if ! HOST_HAVE_STRCSPN

size_t
#if PROTO
strcspn( const char * src, const char * cset)
#else
strcspn( src, cset)
    char *  src;
    char *  cset;
#endif
{
    const char  *s1, *cs;

    for (s1 = src; *s1 != '\0'; s1++)
        for (cs = cset; *cs != '\0'; cs++)
            if (*s1 == *cs)
                    /* Found the first occurence of any of cset[]   */
                return  s1 - src;
    return  NULL;
}

#endif

#if ! HOST_HAVE_MEMMOVE

char *
#if PROTO
memmove( char * dest, const char * src, size_t size)
#else
memmove( dest, src, size)
    char *  dest;
    char *  src;
    size_t  size;
#endif
{
    char *  cp = dest;

    if (dest < src) {
        while (size--)
            *cp++ = *src++;
    } else {
        cp += size - 1;
        src += size - 1;
        while (size--)
            *cp-- = *src--;
    }
    return  dest;
}

#endif

#if ! HOST_HAVE_MEMCPY

char *
#if PROTO
memcpy( char * dest, const char * src, size_t size)
#else
memcpy( dest, src, size)
    char *  dest;
    char *  src;
    size_t  size;
#endif
{
    char *  p = dest;

    while (size-- > 0)
        *p++ = *src++;
    return  dest;
}

#endif

#if ! HOST_HAVE_MEMCMP

int
#if PROTO
memcmp( const char * s1, const char * s2, size_t size)
#else
memcmp( s1, s2, size)
    char *  s1;
    char *  s2;
    size_t  size;
#endif
{
    for ( ; size; size--, s1++, s2++) {
        if (*s1 != *s2)
            return  ((*s1 & UCHARMAX) < (*s2 & UCHARMAX)) ? -1 : 1;
    }
    return  0;
}

#endif

