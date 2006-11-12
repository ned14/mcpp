/*
 * testmain.c: A sample source to show how to use mcpp as a subroutine.
 * 2006/11  contributed by Juergen Mueller.
 * Refer to mcpp-porting.html section 3.12 for compiling mcpp as a subroutine.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern int  mcpp_lib_main(int argc, char **argv);

int main(int argc, char *argv[])
{
    int     retval;
    int     i, j;
    char ** tmp_argv;
    clock_t start, finish;

    tmp_argv = (char **) malloc(sizeof(char *) * (argc + 1));

    if (tmp_argv == NULL) {
        return(0);
    }

    /*
     * assume strict order: <options, ...> <files, ...> and
     * each option and its argument are specified without intervening spaces
     * such as '-I/dir' or '-otmp.i' (not '-I /dir' nor '-o tmp.i').
     */
    for (i = 0; i < argc; ++i) {
        tmp_argv[i] = argv[i];              /* copy options */

        if (       (*argv[i] != '-')
                && (*argv[i] != '/')
                && (i > 0)) {
            break;
        }
    }

    for (j = i; i < argc; ++i) {
    /* this works only if called function does not manipulate pointer array! */
        tmp_argv[j] = argv[i];              /* process each file    */
        fprintf(stderr, "\n%s\n", argv[i]);

        start = clock();                    /* get start time       */
        retval = mcpp_lib_main(j + 1, tmp_argv);    /* call MCPP    */
        finish = clock();                   /* get finish time      */

        fprintf(stderr, "\nElapsed time: %.3f seconds.\n",
                (double)(finish - start) / (double)CLOCKS_PER_SEC);
    }

    free(tmp_argv);

    return(0);
}
