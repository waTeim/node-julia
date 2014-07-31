#include <julia.h>
#include "Immediate.h"

using namespace std;

vector<shared_ptr<nj::Value>> nj::Immediate::eval(vector<shared_ptr<nj::Value>> &args)
{
   vector<shared_ptr<nj::Value>> res;

   if(args.size() != 1 || !args[0]->isPrimitive()) return res;

   Primitive &p = static_cast<Primitive&>(*args[0]);

   jl_value_t *jvalue = (jl_value_t*)jl_eval_string((char*)p.toString().c_str());

   if(jl_is_null(jvalue))
   {
      shared_ptr<nj::Value>  value(new nj::Null);
      res.push_back(value);
   }
   else
   {   
      shared_ptr<nj::Value> value;

      if(jl_is_int64(jvalue)) value.reset(new nj::Int(jl_unbox_int64(jvalue)));
      else if(jl_is_int32(jvalue)) value.reset(new nj::Int(jl_unbox_int32(jvalue)));
      else if(jl_is_float64(jvalue)) value.reset(new nj::Float(jl_unbox_float64(jvalue)));
      else if(jl_is_float32(jvalue)) value.reset(new nj::Float(jl_unbox_float32(jvalue)));
      else if(jl_is_uint64(jvalue)) value.reset(new nj::Int(jl_unbox_uint64(jvalue)));
      else if(jl_is_uint32(jvalue)) value.reset(new nj::Int(jl_unbox_uint32(jvalue)));
      else if(jl_is_int8(jvalue)) value.reset(new nj::Int(jl_unbox_int8(jvalue)));
      else if(jl_is_int16(jvalue)) value.reset(new nj::Int(jl_unbox_int16(jvalue)));
      else if(jl_is_uint8(jvalue)) value.reset(new nj::Int(jl_unbox_uint8(jvalue)));
      else if(jl_is_uint16(jvalue)) value.reset(new nj::Int(jl_unbox_uint16(jvalue)));
      else if(jl_is_ascii_string(jvalue)) value.reset(new nj::String((char*)jl_unbox_voidpointer(jvalue)));

      if(value.get()) res.push_back(value);
   }
   return res;
}
