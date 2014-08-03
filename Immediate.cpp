#include <julia.h>
#include "Immediate.h"
#include "lvalue.h"

using namespace std;

vector<shared_ptr<nj::Value>> nj::Immediate::eval(vector<shared_ptr<nj::Value>> &args)
{
   vector<shared_ptr<nj::Value>> res;

   if(args.size() != 1 || !args[0]->isPrimitive()) return res;

   Primitive &p = static_cast<Primitive&>(*args[0]);

   jl_value_t *jvalue = (jl_value_t*)jl_eval_string((char*)p.toString().c_str());

   return lvalue(jvalue);
}
