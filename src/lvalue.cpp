#include <iostream>
#include <julia.h>
#include <memory.h>
#include "Values.h"
#include "lvalue.h"

using namespace std;

static const string SubString("SubString");

template <typename V,typename E> static shared_ptr<nj::Value> arrayFromBuffer(jl_value_t *jlA)
{
   shared_ptr<nj::Value> value;
   V *p = (V*)jl_array_data(jlA);
   int ndims = jl_array_ndims(jlA);
   vector<size_t> dims;

   for(int dim = 0;dim < ndims;dim++) dims.push_back(jl_array_dim(jlA,dim));

   nj::Array<V,E> *array = new nj::Array<V,E>(dims);
   
   value.reset(array);
   memcpy(array->ptr(),p,array->size()*sizeof(V));
   return value;
}

string getSTDStringFromJuliaString(jl_value_t *val)
{
   string res = string(jl_string_data(val));

   return res;
}

template <typename V,typename E,V (&getElement)(jl_value_t*)> static shared_ptr<nj::Value> arrayFromElements(jl_value_t *jlA)
{
   shared_ptr<nj::Value> value;
   jl_value_t **jlA_p = (jl_value_t**)jl_array_data(jlA);
   int ndims = jl_array_ndims(jlA);
   vector<size_t> dims;

   for(int dim = 0;dim < ndims;dim++) dims.push_back(jl_array_dim(jlA,dim));

   nj::Array<V,E> *A = new nj::Array<V,E>(dims);
   V *A_p = A->ptr();

   value.reset(A);

   for(size_t elNum = 0;elNum < A->size();elNum++) *A_p++ = getElement(*jlA_p++);

   return value;
}

static jl_value_t *convertArray(jl_value_t *from,jl_datatype_t *destElementType)
{
   static jl_function_t *juliaConvert = 0;

   if(!juliaConvert) juliaConvert = jl_get_function(jl_base_module,"convert");
   if(juliaConvert)
   {
      jl_value_t *atype = jl_apply_array_type(destElementType,jl_array_ndims(from));

      JL_GC_PUSH2(&from,&destElementType);
   
      jl_value_t *to = jl_call2(juliaConvert,atype,from);

      JL_GC_POP();

      return to;
   }
   return 0;
}


static shared_ptr<nj::Value> getArrayValue(jl_value_t *jlA)
{
   shared_ptr<nj::Value> value;

   jl_value_t *elementType = jl_tparam0(jl_typeof(jlA));

   if(elementType == (jl_value_t*)jl_float64_type) value = arrayFromBuffer<double,nj::Float64_t>(jlA); 
   else if(elementType == (jl_value_t*)jl_int64_type) value = arrayFromBuffer<int64_t,nj::Int64_t>(jlA);
   else if(elementType == (jl_value_t*)jl_int32_type) value = arrayFromBuffer<int,nj::Int32_t>(jlA);
   else if(elementType == (jl_value_t*)jl_int8_type) value = arrayFromBuffer<char,nj::Int8_t>(jlA);
   else if(elementType == (jl_value_t*)jl_float32_type) value = arrayFromBuffer<float,nj::Float32_t>(jlA);
   else if(elementType == (jl_value_t*)jl_uint64_type) value = arrayFromBuffer<uint64_t,nj::UInt64_t>(jlA); 
   else if(elementType == (jl_value_t*)jl_uint32_type) value = arrayFromBuffer<unsigned,nj::UInt32_t>(jlA);
   else if(elementType == (jl_value_t*)jl_int16_type) value = arrayFromBuffer<short,nj::Int16_t>(jlA);
   else if(elementType == (jl_value_t*)jl_uint8_type) value = arrayFromBuffer<unsigned char,nj::UInt8_t>(jlA);
   else if(elementType == (jl_value_t*)jl_uint16_type) value = arrayFromBuffer<unsigned short,nj::UInt16_t>(jlA);
   else if(elementType == (jl_value_t*)jl_ascii_string_type) value = arrayFromElements<string,nj::ASCIIString_t,getSTDStringFromJuliaString>(jlA);
   else if(elementType == (jl_value_t*)jl_utf8_string_type) value = arrayFromElements<string,nj::UTF8String_t,getSTDStringFromJuliaString>(jlA);
   else
   { 
      const char *juliaTypename = jl_typename_str(elementType);

      if(SubString == juliaTypename)
      {
         jl_value_t *utfArray = convertArray(jlA,jl_utf8_string_type);

         if(utfArray) value = arrayFromElements<string,nj::UTF8String_t,getSTDStringFromJuliaString>(utfArray);
      }
   }

   return value;
}

void addLValueElements(jl_value_t *jl_value,vector<shared_ptr<nj::Value>> &res)
{
   if(!jl_value) return;

   if(jl_is_null(jl_value))
   {
      shared_ptr<nj::Value>  value(new nj::Null);
      res.push_back(value);
   }
   else if(jl_is_array(jl_value)) res.push_back(getArrayValue(jl_value));
   else if(jl_is_tuple(jl_value))
   {
      jl_tuple_t *t = (jl_tuple_t*)jl_value;
      size_t tupleLen = jl_tuple_len(t);

      for(size_t i = 0;i < tupleLen;i++) 
      {
         jl_value_t *element = jl_tupleref(t,i);

         addLValueElements(element,res);
      }
   }
   else
   {   
      shared_ptr<nj::Value> value;

      if(jl_is_float64(jl_value)) value.reset(new nj::Float64(jl_unbox_float64(jl_value)));
      else if(jl_is_int64(jl_value)) value.reset(new nj::Int64(jl_unbox_int64(jl_value)));
      else if(jl_is_int32(jl_value)) value.reset(new nj::Int32(jl_unbox_int32(jl_value)));
      else if(jl_is_int8(jl_value)) value.reset(new nj::Int8(jl_unbox_int8(jl_value)));
      else if(jl_is_utf8_string(jl_value)) value.reset(new nj::UTF8String(jl_string_data(jl_value)));
      else if(jl_is_ascii_string(jl_value)) value.reset(new nj::ASCIIString(jl_string_data(jl_value)));
      else if(jl_is_float32(jl_value)) value.reset(new nj::Float32(jl_unbox_float32(jl_value)));
      else if(jl_is_uint64(jl_value)) value.reset(new nj::UInt64(jl_unbox_uint64(jl_value)));
      else if(jl_is_uint32(jl_value)) value.reset(new nj::UInt32(jl_unbox_uint32(jl_value)));
      else if(jl_is_int16(jl_value)) value.reset(new nj::Int16(jl_unbox_int16(jl_value)));
      else if(jl_is_uint8(jl_value)) value.reset(new nj::UInt8(jl_unbox_uint8(jl_value)));
      else if(jl_is_uint16(jl_value)) value.reset(new nj::UInt16(jl_unbox_uint16(jl_value)));
      else if(jl_is_bool(jl_value)) value.reset(new nj::Boolean(jl_unbox_bool(jl_value)));

      if(value.get()) res.push_back(value);
   }
}

vector<shared_ptr<nj::Value>> nj::lvalue(jl_value_t *jl_value)
{
   vector<shared_ptr<nj::Value>> res;
 
   addLValueElements(jl_value,res);

   return res;
}
