#ifndef __nj_error
#define __nj_error

#include <memory>
#include <string>
#include "Exception.h"

namespace nj
{
   std::shared_ptr<nj::Exception> genJuliaError(jl_value_t *jl_ex);
};

#endif
