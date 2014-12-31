#ifndef __nj_Immediate
#define __nj_Immediate

#include "Expr.h"

namespace nj
{
   class Immediate:public EvalFunc
   {
      public:

         virtual Result eval(std::vector<std::shared_ptr<Value>> &args,int64_t exprId);
   };
};

#endif
