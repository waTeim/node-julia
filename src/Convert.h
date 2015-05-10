#ifndef __nj_Convert
#define __nj_Convert

#include "Expr.h"

namespace nj
{
   class Convert:public EvalFunc
   {
      public:

         virtual Result eval(std::vector<std::shared_ptr<Value>> &args,int64_t exprId);
   };
};

#endif
