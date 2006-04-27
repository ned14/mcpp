/* preproc.c:   to "pre-preprocess" header files.   */

#pragma MCPP preprocess

#include    "system.H"
#include    "internal.H"

#ifdef  __STDC__
#pragma MCPP put_defines
#else
#ifdef  __cplusplus
#pragma MCPP put_defines
#else
#put_defines
#endif
#endif

