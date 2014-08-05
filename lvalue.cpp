#include <julia.h>
#include "Values.h"
#include "lvalue.h"

#include <iostream>

using namespace std;

template <typename V,typename E> static shared_ptr<nj::Value> reboxArray(jl_value_t *jlarray)
{
   shared_ptr<nj::Value> value;
   V *p = (V*)jl_array_data(jlarray);
   int ndims = jl_array_ndims(jlarray);
   vector<int> dims;

   for(int dim = 0;dim < ndims;dim++) dims.push_back(jl_array_dim(jlarray,dim));

   nj::Array<V,E> *array = new nj::Array<V,E>(dims);
   
   value.reset(array);
   memcpy(array->ptr(),p,array->size()*sizeof(double));
   return value;
}

static shared_ptr<nj::Value> getArrayValue(jl_value_t *jlarray)
{
   shared_ptr<nj::Value> value;
   jl_value_t *elementType = jl_tparam0(jl_typeof(jlarray));
  
   if(jl_is_int64(elementType)) value = reboxArray<int64_t,nj::Int64_t>(jlarray);
   else if(jl_is_int32(elementType)) value = reboxArray<int,nj::Int32_t>(jlarray);
   else if(jl_is_float64(elementType)) value = reboxArray<double,nj::Float64_t>(jlarray); 
   else if(jl_is_float32(elementType)) value = reboxArray<float,nj::Float32_t>(jlarray);
   else if(jl_is_uint64(elementType)) value = reboxArray<uint64_t,nj::UInt64_t>(jlarray); 
   else if(jl_is_uint32(elementType)) value = reboxArray<unsigned int,nj::UInt32_t>(jlarray);
   else if(jl_is_int8(elementType)) value = reboxArray<char,nj::Char_t>(jlarray);
   else if(jl_is_int16(elementType)) value = reboxArray<short,nj::Int16_t>(jlarray);
   else if(jl_is_uint8(elementType)) value = reboxArray<unsigned char,nj::UChar_t>(jlarray);
   else if(jl_is_uint16(elementType)) value = reboxArray<unsigned short,nj::UInt16_t>(jlarray);

   return value;
}

vector<shared_ptr<nj::Value>> nj::lvalue(jl_value_t *jlvalue)
{
   vector<shared_ptr<nj::Value>> res;

   if(jl_is_null(jlvalue))
   {
      shared_ptr<nj::Value>  value(new nj::Null);
      res.push_back(value);
   }
   else if(jl_is_array(jlvalue))
   {
      res.push_back(getArrayValue(jlvalue));
   }
   else
   {   
      shared_ptr<nj::Value> value;

      if(jl_is_int64(jlvalue)) value.reset(new nj::Int64(jl_unbox_int64(jlvalue)));
      else if(jl_is_int32(jlvalue)) value.reset(new nj::Int32(jl_unbox_int32(jlvalue)));
      else if(jl_is_float64(jlvalue)) value.reset(new nj::Float64(jl_unbox_float64(jlvalue)));
      else if(jl_is_float32(jlvalue)) value.reset(new nj::Float32(jl_unbox_float32(jlvalue)));
      else if(jl_is_uint64(jlvalue)) value.reset(new nj::UInt64(jl_unbox_uint64(jlvalue)));
      else if(jl_is_uint32(jlvalue)) value.reset(new nj::UInt32(jl_unbox_uint32(jlvalue)));
      else if(jl_is_int8(jlvalue)) value.reset(new nj::Char(jl_unbox_int8(jlvalue)));
      else if(jl_is_int16(jlvalue)) value.reset(new nj::Int16(jl_unbox_int16(jlvalue)));
      else if(jl_is_uint8(jlvalue)) value.reset(new nj::UChar(jl_unbox_uint8(jlvalue)));
      else if(jl_is_uint16(jlvalue)) value.reset(new nj::UInt16(jl_unbox_uint16(jlvalue)));
      else if(jl_is_ascii_string(jlvalue)) value.reset(new nj::String((char*)jl_unbox_voidpointer(jlvalue)));

      if(value.get()) res.push_back(value);
   }
   return res;
}
