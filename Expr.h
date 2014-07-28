#ifndef __nj_interface
#define __nj_interface

#include <vector>
#include "Types.h"

namespace nj
{
   struct Expr;

   class EvalFunc
   {
      public:

         virtual std::vector<std::shared_ptr<Value>> eval(std::vector<std::shared_ptr<Value>> &args) = 0;
   };

   struct Expr
   {
      std::shared_ptr<EvalFunc> F;
      std::vector<std::shared_ptr<Value>> args;

      std::vector<std::shared_ptr<Value>> eval()
      {
         printf("In Expr::eval\n");
         if(F.get()) return F->eval(args);
         return args;
      }
   };
};

#endif
