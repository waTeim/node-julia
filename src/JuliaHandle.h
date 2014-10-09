#ifndef __nj_JuliaHandle
#define __nj_JuliaHandle

#include "Value.h"

namespace nj
{
   class JuliaHandle: public Value
   {
      protected:

         static std::vector<size_t> none;
         jl_value_t *jvalue;

      public:

         JuliaHandle(jl_value_t *jvalue) { this->jvalue = jvalue; }
         virtual bool isPrimitive() const { return false; }
         virtual const Type *type() const { return JuliaHandle_t::instance(); }
         virtual const std::vector<size_t> &dims() const {  return none;  }
         jl_value_t *val() const {  return jvalue;  }
   };
};

#endif
