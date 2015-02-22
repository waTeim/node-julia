#ifndef __juliav
#define __juliav

/*************************************************************/
/* Some things that need to be defined differently depending */
/* on the version of Julia.                                  */
/*                                                           */
/* The define JL_COMPILEROPT_COMPILE_ALL  is only defined in */
/* Julia 0.4 and later                                       */
/*************************************************************/
#if defined(JULIA_VERSION_MINOR) && JULIA_VERSION_MINOR == 4
#define JVOID_T jl_void_type
#else
#define JVOID_T ((jl_datatype_t*)jl_bottom_type)
#endif

#endif
