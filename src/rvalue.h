#ifndef __nj_rvalue
#define __nj_rvalue

#include <vector>
#include <memory>
#include "Value.h"

namespace nj
{
   jl_value_t *rvalue(const std::shared_ptr<nj::Value> &value);
};

#endif
