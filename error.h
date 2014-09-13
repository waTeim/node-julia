#ifndef __nj_error
#define __nj_error

#include <string>

namespace nj
{
   std::string getErrorText(jl_value_t *ex);
};

#endif
