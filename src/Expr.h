#ifndef __nj_interface
#define __nj_interface

#include <vector>
#include "Values.h"
#include "Result.h"

namespace nj
{
   class Expr;

   class EvalFunc
   {
      public:

         virtual Result eval(std::vector<std::shared_ptr<Value>> &args,int64_t exprId) = 0;
   };

   class Expr
   {
      protected:

         static int64_t next_id;

      public:

         enum Dest { syncQ, asyncQ };

         std::shared_ptr<EvalFunc> F;
         std::vector<std::shared_ptr<Value>> args;
         int64_t id;
         Dest dest;
 
         Expr(Dest dest = syncQ)
         { 
            id = next_id++;
            this->dest = dest;
         }

         Result eval()
         {
            if(F.get()) return F->eval(args,id);
            return Result(args,id);
         }
   };
};

#endif
