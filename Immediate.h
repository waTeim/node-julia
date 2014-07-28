#ifndef __nj_Immediate
#define __nj_Immediate

#include "Expr.h"

namespace nj
{
   class Immediate:public EvalFunc
   {
      public:

         virtual std::vector<std::shared_ptr<Value>> eval(std::vector<std::shared_ptr<Value>> &args);
   };
};

#endif
