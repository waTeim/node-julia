#include <julia.h>
#include "Values.h"
#include "lvalue.h"

#include <iostream>

using namespace std;

vector<shared_ptr<nj::Value>> nj::lvalue(jl_value_t *jvalue)
{
   vector<shared_ptr<nj::Value>> res;

   if(jl_is_null(jvalue))
   {
printf("Result is null\n");
      shared_ptr<nj::Value>  value(new nj::Null);
      res.push_back(value);
   }
   else if(jl_is_array(jvalue))
   {
      double *p = (double*)jl_array_data(jvalue);
      int ndims = jl_array_ndims(jvalue);
      vector<int> dims;
      shared_ptr<nj::Value> value;

      for(int dim = 0;dim < ndims;dim++) dims.push_back(jl_array_dim(jvalue,dim));

      Array<double,Float_t> *array = new Array<double,Float_t>(dims);
      
      value.reset(array);
      memcpy(array->ptr(),p,array->size()*sizeof(double));
      for(int i = 0;i < array->size();i++)
      {
         printf("%f ",array->ptr()[i]);
      }
      printf("\nResults was arrays: %lu %lu\n",array->dims().size(),array->size());
cout << "Array Results:";
for(int i:array->dims()) cout << " " << i;
cout << " size == " << array->size() << endl;
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
