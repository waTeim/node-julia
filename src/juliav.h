#ifndef __juliav
#define __juliav

extern "C"
{
#include <julia.h>
}

/*************************************************************/
/* Some things that need to be defined differently depending */
/* on the version of Julia.                                  */
/*                                                           */
/* The define JL_COMPILEROPT_COMPILE_ALL  is only defined in */
/* Julia 0.4 and later                                       */
/*************************************************************/
#if defined(JULIA_VERSION_MINOR) && JULIA_VERSION_MINOR >= 4
#define JVOID_T jl_void_type
#else
#define JVOID_T ((jl_datatype_t*)jl_bottom_type)
#endif

#if (JULIA_VERSION_MINOR == 4 && !defined(JL_NULL)) || JULIA_VERSION_MINOR >= 5
#define USES_SVEC 1
#define TUPLE_CAST(x) (x)
#define JL_IS_NULL(v) (jl_is_nothing(v) || (jl_is_tuple(v) && jl_nfields(v) == 0))
#define JL_NULL jl_nothing
#else
#define JL_IS_NULL(v) jl_is_null(v)
#define TUPLE_CAST(x) ((jl_tuple_t*)x)
#endif

#endif
