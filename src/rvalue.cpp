#include <iostream>
#include <julia.h>
#include "juliav.h"
#include "rvalue.h"
#include "Values.h"
#include "error.h"
#include "Kernel.h"

using namespace std;

static jl_function_t *getUnix2DateTime()
{
   jl_module_t *dates_m = (jl_module_t*)jl_get_global(jl_base_module,jl_symbol("Dates"));

   if(dates_m) return jl_get_function(dates_m,"unix2datetime");
   return 0;
}

static jl_value_t *rPrimitive(const nj::Primitive &prim) throw(nj::JuliaException)
{
   jl_value_t *res = 0;

   switch(prim.type()->getId())
   {
      case nj::null_type: res = (jl_value_t*)jl_null; break;
      case nj::boolean_type:
      {
         const nj::Boolean &v = static_cast<const nj::Boolean&>(prim);

         if(v.val()) res = jl_true;
         else res = jl_false;
      }
      break;
      case nj::int64_type:
      {
         const nj::Int64 &v = static_cast<const nj::Int64&>(prim);
         res = jl_box_int64(v.val());
      }
      break;
      case nj::int32_type:
      {
         const nj::Int32 &v = static_cast<const nj::Int32&>(prim);

         res = jl_box_int32(v.val());
      }
      break;
      case nj::int16_type:
      {
         const nj::Int16 &v = static_cast<const nj::Int16&>(prim);

         res = jl_box_int16(v.val());
      }
      break;
      case nj::int8_type:
      {
         const nj::Int8 &v = static_cast<const nj::Int8&>(prim);

         res = jl_box_char(v.val());
      }
      break;
      case nj::uint64_type:
      {
         const nj::UInt64 &v = static_cast<const nj::UInt64&>(prim);

         res = jl_box_uint64(v.val());
      }
      break;
      case nj::uint32_type:
      {
         const nj::UInt32 &v = static_cast<const nj::UInt32&>(prim);
      
         res = jl_box_uint32(v.val());
      }  
      break;
      case nj::uint16_type:
      {
         const nj::UInt16 &v = static_cast<const nj::UInt16&>(prim);
      
         res = jl_box_uint16(v.val());
      }
      break;
      case nj::uint8_type:
      {
         const nj::UInt8 &v = static_cast<const nj::UInt8&>(prim);

         res = jl_box_uint8(v.val());
      }
      break;
      case nj::float64_type:
      {
         const nj::Float64 &v = static_cast<const nj::Float64&>(prim);
      
         res = jl_box_float64(v.val());
      }
      break; 
      case nj::float32_type:
      {
         const nj::Float32 &v = static_cast<const nj::Float32&>(prim);

         res = jl_box_float32(v.val());
      }
      break;
      case nj::ascii_string_type:
      {
         const nj::ASCIIString &v = static_cast<const nj::ASCIIString&>(prim);

         res = jl_cstr_to_string(v.val().c_str());
      }
      break;
      case nj::utf8_string_type:
      {
         const nj::UTF8String &v = static_cast<const nj::UTF8String&>(prim);

         res = jl_cstr_to_string(v.val().c_str());
      }
      break;
      case nj::date_type:
      {
         const nj::Date &v = static_cast<const nj::Date&>(prim);
         static jl_function_t *unix2DateTime_f = 0;

         if(!unix2DateTime_f) unix2DateTime_f = getUnix2DateTime();

         if(unix2DateTime_f)
         {
            jl_value_t *milliseconds = jl_box_float64(v.val()/1000);

            JL_GC_PUSH1(&milliseconds);
            res = jl_call1(unix2DateTime_f,milliseconds);
            JL_GC_POP();
         }
      }
      break;
      case nj::regex_type:
      {
         const nj::Regex &v = static_cast<const nj::Regex&>(prim);
         jl_value_t *pattern  = jl_cstr_to_string(v.val().c_str());
         nj::Kernel *kernel = nj::Kernel::getSingleton();

         res = kernel->newRegex(pattern);
      }
      break;
   }
   return res;
}

template<typename V,typename E> static jl_array_t *arrayFromBuffer(const shared_ptr<nj::Value> &val,jl_datatype_t *jl_element_type)
{
   const nj::Array<V,E> &A = static_cast<nj::Array<V,E>&>(*val);
   jl_value_t *atype = jl_apply_array_type(jl_element_type,A.dims().size());
   jl_tuple_t *dims = jl_alloc_tuple(A.dims().size());
   int i = 0;

   for(size_t dim: A.dims()) jl_tupleset(dims,i++,jl_box_long(dim));

   return jl_ptr_to_array(atype,A.ptr(),dims,0);
}

jl_value_t *getJuliaNullElement(const unsigned char &c)
{
   return 0;
}

jl_value_t *getJuliaStringFromSTDString(const string &s)
{
   return jl_cstr_to_string(s.c_str());
}

template <typename V,typename E,jl_value_t *(&getElement)(const V&)> static jl_array_t *arrayFromElements(const shared_ptr<nj::Value> &val,jl_datatype_t *jl_element_type)
{
   const nj::Array<V,E> &A = static_cast<nj::Array<V,E>&>(*val);
   jl_value_t *atype = jl_apply_array_type(jl_element_type,A.dims().size());
   jl_tuple_t *dims = jl_alloc_tuple(A.dims().size());
   int i = 0;

   for(size_t dim: A.dims()) jl_tupleset(dims,i++,jl_box_long(dim));

   jl_array_t *A_jl = jl_new_array(atype,dims);
   V *A_p = A.ptr();
   jl_value_t **Ajl_p = (jl_value_t**)A_jl->data;
    
   for(size_t elNum = 0;elNum < A.size();elNum++) *Ajl_p++ = getElement(*A_p++);

   return A_jl;
}

static jl_array_t *rArray(const shared_ptr<nj::Value> &array)
{
   jl_array_t *res = 0;
   const nj::Array_t *atype = static_cast<const nj::Array_t*>(array->type());

   switch(atype->etype()->getId())
   {  
      case nj::null_type: res = arrayFromElements<unsigned char,nj::Null_t,getJuliaNullElement>(array,JVOID_T); break;
      case nj::boolean_type: res = arrayFromBuffer<unsigned char,nj::Boolean_t>(array,jl_bool_type); break;
      case nj::int64_type: res = arrayFromBuffer<int64_t,nj::Int64_t>(array,jl_int64_type); break;
      case nj::int32_type: res = arrayFromBuffer<int,nj::Int32_t>(array,jl_int32_type); break;
      case nj::int16_type: res = arrayFromBuffer<short,nj::Int16_t>(array,jl_int16_type); break;
      case nj::uint64_type: res = arrayFromBuffer<uint64_t,nj::UInt64_t>(array,jl_uint64_type); break;
      case nj::uint32_type: res = arrayFromBuffer<unsigned int,nj::UInt32_t>(array,jl_uint32_type); break;
      case nj::uint16_type: res = arrayFromBuffer<unsigned short,nj::UInt16_t>(array,jl_uint16_type); break;
      case nj::float64_type: res = arrayFromBuffer<double,nj::Float64_t>(array,jl_float64_type); break;
      case nj::float32_type: res = arrayFromBuffer<float,nj::Float32_t>(array,jl_float32_type); break;
      case nj::int8_type: res = arrayFromBuffer<char,nj::Int8_t>(array,jl_int8_type); break;
      case nj::uint8_type: res = arrayFromBuffer<unsigned char,nj::UInt8_t>(array,jl_uint8_type); break;
      case nj::ascii_string_type: res = arrayFromElements<string,nj::ASCIIString_t,getJuliaStringFromSTDString>(array,jl_ascii_string_type); break;
      case nj::utf8_string_type: res = arrayFromElements<string,nj::UTF8String_t,getJuliaStringFromSTDString>(array,jl_utf8_string_type); break;
   }
   return res;
}

jl_value_t *nj::rvalue(const shared_ptr<nj::Value> &value) throw(JuliaException)
{
   if(value->isPrimitive())
   {
      const nj::Primitive &p = static_cast<const nj::Primitive&>(*value);

      return rPrimitive(p);
   }
   else return (jl_value_t*)rArray(value);
}
