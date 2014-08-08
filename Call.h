#ifndef __nj_Call
#define __nj_Call

#include "Expr.h"

namespace nj
{
   class Call:public EvalFunc
   {
      public:

         virtual std::vector<std::shared_ptr<Value>> eval(std::vector<std::shared_ptr<Value>> &args);
   };
};

#endif
