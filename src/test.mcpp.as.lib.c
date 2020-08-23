/*

	dc
	
	gcc test.mcpp.as.lib.c /usr/local/lib/libmcpp.a -o program
	./program ../test.new/input.c
	
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "mcpp_lib.h"

// ************
// 		MAIN
// ************

#define isAnOption(X) ((((X))=='-') || (((X))=='/'))

int main(int argc, char *argv[])
{
	printf("\n");
	
	/*
		a) il preprocessore vuole in ordine la seguente modalita' di input parametri
	
			1) prima le opzioni
			2) infine il file di input
		
		b) il processore necessita che le opzioni vadano unnite col parametro
	
			1) -o output.c -> -ooutput.c
			
			questo per-lutente e' scomodo, per tanto se ne occupera' il check option

		c) per una compatibilita' col lexer (RE-Flex) il file di input sara; preceduto da -i
		 
		 	1) -i intput.txt ->  input.txt
		 	
		 	e verra omesso nella stringa finale
	
		d) non sono permesse opzioni finali, ma solo iniziali
	
			1) -i input.txt -3 -> -3 -i input.txt
		
		e) 
			assume strict order: <options, ...> <files, ...> and
			each option and its argument are specified without intervening spaces
			such as '-I/dir' or '-otmp.i' (not '-I /dir' nor '-o tmp.i').
			 
			./program -o output.c -3 ../test.new/input.c		[v]
			./program -ooutput.c  -3 ../test.new/input.c		[x] -> ./program -o output.c  -3 ../test.new/input.c 
			./program -ooutput.c  ../test.new/input.c -3		[x] -> ./program -ooutput.c  -3 ../test.new/input.c
			./program -ooutput.c  -i ../test.new/input.c -3		[x] -> ./program -ooutput.c  -3 -i../test.new/input.c
	
	*/

	// ***********	
	// init tmp_argv 	
	// ***********

	const int maxNumOption		=	16; // KISS
	const int maxLengthOption	=	32;
    char tmp_argv[maxNumOption][maxLengthOption];
    
	int ktmp_argv=0;
	
	// ***********	
	// CHECK input 	
	// ***********
		
	// se l'ultimo parametro e' un'opzione ritorna errore
	if ( isAnOption(argv[argc-1][0])  )
	{ 
		printf("ERROR 	 : option must be before of input file [%s].\n",&argv[argc-1][0]);
		return 13; 
	} ;

	// se il primo parametro e' un file ritorna errore
	if (argc>1)
	if ( !isAnOption(argv[1][0])  )
	{ 
		printf("ERROR 	 : file without option (with -i -o) [%s].\n",&argv[1][0]);
		return 13; 
	} ; 

	int nargc	=	0	;
	int i		=	1	;

	while(i+1<argc) // non controlla il primo e l'ultimo parametro
	{
			//printf("\n[[[%s]]]\n",&argv[i][0]);
			static int fI=0;
			if ( strlen(&argv[i][0]) > maxLengthOption )
			{
				printf("ERROR 	 : option length exceed limit %d/%d.\n",(int)strlen(&argv[i][0]),(int)maxLengthOption);
				return 13; 				
			} 

			tmp_argv[ktmp_argv][0] = 0 ; // init default
			strcat(tmp_argv[ktmp_argv],&argv[i][0]);	
			
			// se l'opzione e' 'i' input file non va fato il merge
			if ( (argv[i][1])=='i')
			{
				if (fI)
				{
					printf("ERROR 	 : allowed only 1 input file\n");
					return 14; 				
				} 	
				fI=1;
				tmp_argv[ktmp_argv][0] = 0 ;
			}
				
			if ( isAnOption(argv[i][0])  ) 			// option
			{
				if ( !isAnOption(argv[i+1][0])  )	// parameter
				{
					//printf ( "[%s %s]\n",&argv[i][0],&argv[i+1][0]) ;
					// merge option to previous
					
					if ( strlen(&argv[i+1][0]) > maxLengthOption )
					{
						printf("ERROR 	 : parameter length exceed limit %d/%d.\n",(int)strlen(&argv[i+1][0]),(int)maxLengthOption);
						return 13; 				
					} 		
					strcat(tmp_argv[ktmp_argv],&argv[i+1][0]);
					++ktmp_argv;
					++nargc;					
					++i;
					
				}
			}
			if ( isAnOption(argv[i][0])  ) 			// option
			{
				if ( isAnOption(argv[i+1][0])  )	// parameter
				{
					//printf ( "[%s]\n",&argv[i][0]) ;
					// next option
					++ktmp_argv;										
					++nargc;					
				}
			}
		++i;		
	}
	// ultimo controllo
	if (argc>2)
		if ( argv[argc-2][1]!='i' )
		{
			printf("ERROR 	 : input file without option (with -i) [%s].\n",&argv[argc-1][0]);
			return 13; 					
		}		

	/*	
		printf ( "### argc::%d nargc::%d\n",argc,nargc) ;
		i=0;
		while(i<ktmp_argv)
		{
			printf ( "### {%s}\n",tmp_argv[i]) ;
			++i;	
		};
	*/

	// ************
	// 		PREPROCESSOR
	// ************
	
	// copy options ( parte come input dall-ultimo file che e' quello input e cos' via )
	
	int cpo = 0 ;
    for (cpo = 0; cpo < ktmp_argv; ++cpo) 
    {
        if (       (tmp_argv[cpo][0] != '-')
                && (tmp_argv[cpo][0] != '/')
                && (i > 0)) 
        {
            break;
        }
    }	

	char** pptmp_argv ;
	
	
	pptmp_argv = (char **) malloc(sizeof(char *) * (ktmp_argv + 1));	
	
	for(int i=0;i<ktmp_argv;i++)
	{
		pptmp_argv[i] = &tmp_argv[i][0];
	}
	

	int     retval;
	
	char * resultOUT = NULL;  
	char * resultERR = NULL; 
	char * resultDBG = NULL; 
			
	for (int j = cpo ; j < ktmp_argv; ++j) 
	{

		#define OUT2MEM       			// Use memory buffer     

		mcpp_use_mem_buffers(1);  		// enable memory output  

		// call MCPP    

		retval = mcpp_lib_main(j+1, pptmp_argv); 

		// get the output       

		resultOUT = mcpp_get_mem_buffer(OUT); 
		if (resultOUT) 
		{
			fputs( resultOUT, stdout);
		}
		
		// get the diagnostics    

		resultERR = mcpp_get_mem_buffer(ERR); 
		if (resultERR)
		{ 
			fputs( resultERR, stderr);
		}
		
		// get the debug output  
		    
		resultDBG = mcpp_get_mem_buffer(DBG);
	
		if (resultDBG)
		{	
			fputs( resultDBG, stdout);   
		}				
		fprintf(stderr, "\nresult %p %p %p.\n",resultOUT,resultERR,resultDBG );	
		
		if ( resultOUT != NULL ) free(resultOUT) ;
		if ( resultERR != NULL ) free(resultERR) ;
		if ( resultDBG != NULL ) free(resultDBG) ;
						   
		fprintf(stderr, "\nReturned status:%d.\n",retval );
	}
	
	// ************
	// 		THE END
	// ************

    free(pptmp_argv);


    return(0);
}



/**/


