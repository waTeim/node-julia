#ifndef __nj_Kernel
#define __nj_Kernel

#include <string>
#include <vector>
#include <julia.h>
#include "Exception.h"

namespace nj
{
   class Kernel
   {
      protected:

         static Kernel *singleton;

         int64_t freelist_start;
         std::vector<int64_t> freelist;
         jl_module_t *nj_module;
         jl_value_t *preserve_array;

         jl_value_t *get_preserve_array() throw(JuliaException);
         jl_value_t *invoke(const std::string &functionName) throw(JuliaException);
         jl_value_t *invoke(const std::string &functionName,jl_value_t *arg) throw(JuliaException);
         jl_module_t *load() throw(JuliaException);

      public:

         static Kernel *getSingleton();
   
         Kernel();

         jl_value_t *scriptify(jl_module_t *isolatingMod,jl_value_t *filenameToInclude) throw(JuliaException);
         jl_value_t *newRegex(jl_value_t *pattern) throw(JuliaException);
         jl_value_t *getPattern(jl_value_t *re) throw(JuliaException);
         jl_datatype_t *getDateTimeType() throw(JuliaException);
         jl_datatype_t *getRegexType() throw(JuliaException);
         int64_t preserve(jl_value_t *val) throw(JuliaException);
         jl_value_t *free(int64_t valIndex) throw(JuliaException);
   };
};

#endif
