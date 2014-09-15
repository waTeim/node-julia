#include <iostream>
#include <julia.h>
#include "Call.h"
#include "error.h"
#include "rvalue.h"
#include "lvalue.h"

using namespace std;

nj::Result nj::Call::eval(vector<shared_ptr<nj::Value>> &args)
{
   vector<shared_ptr<nj::Value>> res;

   if(args.size() == 0 || !args[0]->isPrimitive()) return res;

   Primitive &funcName = static_cast<Primitive&>(*args[0]);
   int numArgs = args.size() - 1;
   jl_value_t *jl_res = 0;
   jl_function_t *func = jl_get_function(jl_core_module,funcName.toString().c_str());

   if(!func) func = jl_get_function(jl_base_module,funcName.toString().c_str());
   if(!func) func = jl_get_function(jl_main_module,funcName.toString().c_str());
   if(!func) return res;

   if(numArgs <= 3)
   {
      switch(numArgs)
      {
         case 0: jl_res = jl_call0(func); break;
         case 1: jl_res = jl_call1(func,rvalue(args[1])); break;
         case 2: jl_res = jl_call2(func,rvalue(args[1]),rvalue(args[2])); break;
         case 3: jl_res = jl_call3(func,rvalue(args[1]),rvalue(args[2]),rvalue(args[3])); break;
      }
   }
   else
   {
      jl_value_t **jl_args = new jl_value_t*[numArgs];

      for(int i = 0;i < numArgs;i++) jl_args[i] = rvalue(args[i + 1]);
      jl_res = jl_call(func,jl_args,numArgs - 1);
      delete jl_args;
   }

   jl_value_t *jl_ex = jl_exception_occurred();
   
   if(jl_ex)
   {
     JL_GC_PUSH1(&jl_ex);

     shared_ptr<Exception> ex = genJuliaError(jl_ex);

     JL_GC_POP();
     return Result(ex);
   }
   else
   {
      JL_GC_PUSH1(&jl_res);
      res = lvalue(jl_res);
      JL_GC_POP();
      return Result(res);
   }
}
