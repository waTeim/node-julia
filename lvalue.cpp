#include <iostream>
#include <julia.h>
#include "Values.h"
#include "lvalue.h"

using namespace std;

template <typename V,typename E> static shared_ptr<nj::Value> reboxArray(jl_value_t *jlarray)
{
   shared_ptr<nj::Value> value;
   V *p = (V*)jl_array_data(jlarray);
   int ndims = jl_array_ndims(jlarray);
   vector<size_t> dims;

   for(int dim = 0;dim < ndims;dim++) dims.push_back(jl_array_dim(jlarray,dim));

   nj::Array<V,E> *array = new nj::Array<V,E>(dims);
   
   value.reset(array);
   memcpy(array->ptr(),p,array->size()*sizeof(V));
   return value;
}

static shared_ptr<nj::Value> getArrayValue(jl_value_t *jlarray)
{
   shared_ptr<nj::Value> value;
   jl_value_t *elementType = jl_tparam0(jl_typeof(jlarray));

   if(elementType == (jl_value_t*)jl_float64_type) value = reboxArray<double,nj::Float64_t>(jlarray); 
   else if(elementType == (jl_value_t*)jl_int64_type) value = reboxArray<int64_t,nj::Int64_t>(jlarray);
   else if(elementType == (jl_value_t*)jl_int32_type) value = reboxArray<int,nj::Int32_t>(jlarray);
   else if(elementType == (jl_value_t*)jl_int8_type) value = reboxArray<char,nj::Char_t>(jlarray);
   else if(elementType == (jl_value_t*)jl_float32_type) value = reboxArray<float,nj::Float32_t>(jlarray);
   else if(elementType == (jl_value_t*)jl_uint64_type) value = reboxArray<uint64_t,nj::UInt64_t>(jlarray); 
   else if(elementType == (jl_value_t*)jl_uint32_type) value = reboxArray<unsigned,nj::UInt32_t>(jlarray);
   else if(elementType == (jl_value_t*)jl_int16_type) value = reboxArray<short,nj::Int16_t>(jlarray);
   else if(elementType == (jl_value_t*)jl_uint8_type) value = reboxArray<unsigned char,nj::UChar_t>(jlarray);
   else if(elementType == (jl_value_t*)jl_uint16_type) value = reboxArray<unsigned short,nj::UInt16_t>(jlarray);

   return value;
}

vector<shared_ptr<nj::Value>> nj::lvalue(jl_value_t *jl_value)
{
   vector<shared_ptr<nj::Value>> res;
  
   if(!jl_value) return res;

   if(jl_is_null(jl_value))
   {
      shared_ptr<nj::Value>  value(new nj::Null);
      res.push_back(value);
   }
   else if(jl_is_array(jl_value))
   {
      res.push_back(getArrayValue(jl_value));
   }
   else
   {   
      shared_ptr<nj::Value> value;

      if(jl_is_float64(jl_value)) value.reset(new nj::Float64(jl_unbox_float64(jl_value)));
      else if(jl_is_int64(jl_value)) value.reset(new nj::Int64(jl_unbox_int64(jl_value)));
      else if(jl_is_int32(jl_value)) value.reset(new nj::Int32(jl_unbox_int32(jl_value)));
      else if(jl_is_int8(jl_value)) value.reset(new nj::Char(jl_unbox_int8(jl_value)));
      else if(jl_is_ascii_string(jl_value)) value.reset(new nj::String((char*)jl_unbox_voidpointer(jl_value)));
      else if(jl_is_float32(jl_value)) value.reset(new nj::Float32(jl_unbox_float32(jl_value)));
      else if(jl_is_uint64(jl_value)) value.reset(new nj::UInt64(jl_unbox_uint64(jl_value)));
      else if(jl_is_uint32(jl_value)) value.reset(new nj::UInt32(jl_unbox_uint32(jl_value)));
      else if(jl_is_int16(jl_value)) value.reset(new nj::Int16(jl_unbox_int16(jl_value)));
      else if(jl_is_uint8(jl_value)) value.reset(new nj::UChar(jl_unbox_uint8(jl_value)));
      else if(jl_is_uint16(jl_value)) value.reset(new nj::UInt16(jl_unbox_uint16(jl_value)));

      if(value.get()) res.push_back(value);
   }
   return res;
}
