#ifndef P
#define P printf
#endif

#ifndef claudio
#define claudio 传/傳
#endif

#define xstr(s) str(s)
#define str(s) #s

#include "prova2.c"

int main ( void)
{
	P("@@%s%x\n",xstr(claudio),XXX );
	return 0 ;
}


