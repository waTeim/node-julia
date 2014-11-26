#ifndef __nj_Kernel
#define __nj_Kernel

#include <julia.h>
#include "Exception.h"

namespace nj
{
   class Kernel
   {
      protected:

         static Kernel *singleton;

         jl_module_t *njModule;

         jl_module_t *load() throw(JuliaException);

      public:

         static Kernel *getSingleton();
   
         Kernel() { njModule = 0; }

         jl_value_t *scriptify(jl_module_t *isolatingMod,jl_value_t *filenameToInclude) throw(JuliaException);
         jl_value_t *newRegex(jl_value_t *pattern) throw(JuliaException);
         jl_value_t *getPattern(jl_value_t *re) throw(JuliaException);
   };
};

#endif
