#ifndef __nj_Script
#define __nj_Script

#include "Expr.h"

namespace nj
{
   class Script:public EvalFunc
   {
      protected:

         static bool util_loaded;

         void parse(const std::string &filename);

      public:

         virtual Result eval(std::vector<std::shared_ptr<Value>> &args,int64_t exprId);
   };
};

#endif
