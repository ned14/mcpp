#ifndef MCPP_LIB_H
#define MCPP_LIB_H

/* Choices for output destination */
typedef enum {
    OUT,                        /* ~= fp_out    */
    ERR,                        /* ~= fp_err    */
    DBG,                        /* ~= fp_debug  */
    NUM_OUTDEST
} OUTDEST;

#if MCPP_LIB
#if _WIN32 || __CYGWIN__ || __MINGW32__
#if     DLL_EXPORT || (__CYGWIN__ && PIC)
#define DLL_DECL    __declspec( dllexport)
#elif   DLL_IMPORT
#define DLL_DECL    __declspec( dllimport)
#else
#define DLL_DECL
#endif
#else
#define DLL_DECL
#endif

extern DLL_DECL int     mcpp_lib_main( int argc, char ** argv);
extern DLL_DECL void    mcpp_reset_def_out_func( void);
extern DLL_DECL void    mcpp_set_out_func(
                    int (* func_fputc)  ( int c, OUTDEST od),
                    int (* func_fputs)  ( const char * s, OUTDEST od),
                    int (* func_fprintf)( OUTDEST od, const char * format, ...)
                    );
extern DLL_DECL void    mcpp_use_mem_buffers( int tf);
extern DLL_DECL char *  mcpp_get_mem_buffer( OUTDEST od);
#endif  /* MCPP_LIB */
#endif  /* MCPP_LIB_H   */
