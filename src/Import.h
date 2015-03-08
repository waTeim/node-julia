#ifndef __nj_Import
#define __nj_Import

#include "Expr.h"

namespace nj
{
   class Import:public EvalFunc
   {
      public:

         virtual Result eval(std::vector<std::shared_ptr<Value>> &args,int64_t exprId);
   };
};

#endif
