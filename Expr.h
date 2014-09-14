#ifndef __nj_interface
#define __nj_interface

#include <vector>
#include "Values.h"
#include "Result.h"

namespace nj
{
   struct Expr;

   class EvalFunc
   {
      public:

         virtual Result eval(std::vector<std::shared_ptr<Value>> &args) = 0;
   };

   struct Expr
   {
      std::shared_ptr<EvalFunc> F;
      std::vector<std::shared_ptr<Value>> args;

      Result eval()
      {
         if(F.get()) return F->eval(args);
         return Result(args);
      }
   };
};

#endif
