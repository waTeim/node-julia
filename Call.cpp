#include <julia.h>
#include "Call.h"
#include "rvalue.h"
#include "lvalue.h"

using namespace std;

vector<shared_ptr<nj::Value>> nj::Call::eval(vector<shared_ptr<nj::Value>> &args)
{
   vector<shared_ptr<nj::Value>> res;

   if(args.size() == 0 || !args[0]->isPrimitive()) return res;

   Primitive &funcName = static_cast<Primitive&>(*args[0]);
   jl_function_t *func = jl_get_function(jl_base_module,funcName.toString().c_str());
   int numArgs = args.size() - 1;
   jl_value_t *jl_res = 0;

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

   JL_GC_PUSH1(&jl_res);
   res = lvalue(jl_res);
   JL_GC_POP();
   return res;
}
