#ifndef __nj_Call
#define __nj_Call

#include "Expr.h"

namespace nj
{
   class Call:public EvalFunc
   {
      public:

         virtual Result eval(std::vector<std::shared_ptr<Value>> &args,int64_t exprId);
   };
};

#endif
