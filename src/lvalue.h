#ifndef __nj_lvalue
#define __nj_lvalue

#include <vector>
#include <memory>
#include "Value.h"

namespace nj
{
   std::vector<std::shared_ptr<nj::Value>> lvalue(jl_value_t *jvalue) throw(JuliaException);
};

#endif
