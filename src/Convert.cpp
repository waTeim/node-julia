#if defined(WIN32)
#pragma warning(disable:4200)
#endif

#include <iostream>
#include <julia.h>
#include "Convert.h"
#include "error.h"
#include "lvalue.h"
#include "JuliaHandle.h"

using namespace std;

nj::Result nj::Convert::eval(vector<shared_ptr<nj::Value>> &args,int64_t exprId)
{
   vector<shared_ptr<nj::Value>> res;

   if(args.size() != 1 || args[0]->type() != JuliaHandle_t::instance()) return Result(res,exprId);

   JuliaHandle &handle = static_cast<JuliaHandle&>(*args[0]);

   try
   {
      jl_value_t *val = handle.val();

      JL_GC_PUSH1(&val);
      res = lvalue(val);
      JL_GC_POP();
      return Result(res,exprId);
   }
   catch(JuliaException e)
   {
     JL_GC_POP();
     return Result(e.err,exprId);
   }
}
