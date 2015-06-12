#if defined(WIN32)
#pragma warning(disable:4200)
#endif

#include <iostream>
#include <julia.h>
#include "juliav.h"
#include "rvalue.h"
#include "Values.h"
#include "error.h"
#include "Kernel.h"
#include "JuliaHandle.h"

using namespace std;

jl_value_t *getDate(const double &milliseconds) throw(nj::JuliaException)
{   
   nj::Kernel *kernel = nj::Kernel::getSingleton();

   return kernel->toDate(milliseconds);
}   

jl_value_t *getJuliaRegexFromString(const string &val) throw(nj::JuliaException)
{
   jl_value_t *pattern  = jl_cstr_to_string(val.c_str());
   nj::Kernel *kernel = nj::Kernel::getSingleton();

   return kernel->newRegex(pattern);
}

static jl_value_t *rPrimitive(const nj::Primitive &prim) throw(nj::JuliaException)
{
   jl_value_t *res = 0;

   switch(prim.type()->id())
   {
      case nj::null_type: res = (jl_value_t*)JL_NULL; break;
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

         res = getDate(v.val());
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

jl_array_t *zeroLengthArray(jl_datatype_t *jl_element_type) throw(nj::JuliaException)
{
   jl_value_t *atype = jl_apply_array_type(jl_element_type,1);
   vector<jl_value_t*> argv;
   nj::Kernel *kernel = nj::Kernel::getSingleton();

   argv.push_back(jl_box_long(0));

   jl_value_t *dims = kernel->newTuple(argv);

   return jl_new_array(atype,TUPLE_CAST(dims));
}

template<typename V,typename E> jl_array_t *arrayFromBuffer(const shared_ptr<nj::Value> &val,jl_datatype_t *jl_element_type) throw(nj::JuliaException)
{
   const nj::Array<V,E> &A = static_cast<nj::Array<V,E>&>(*val);
   nj::Kernel *kernel = nj::Kernel::getSingleton();

   if(A.v()->container())
   {
      shared_ptr<nj::Alloc> loc0 = A.v()->container()->loc0();
      nj::JuAlloc *L = static_cast<nj::JuAlloc*>(loc0.get());

      return (jl_array_t*)kernel->get(L->pindex());
   }
   else
   {
      jl_value_t *atype = jl_apply_array_type(jl_element_type,A.dims().size());
      vector<jl_value_t*> argv;

      for(size_t dim: A.dims()) argv.push_back(jl_box_long(dim));

      jl_value_t *dims = kernel->newTuple(argv);

      return jl_ptr_to_array(atype,A.ptr(),TUPLE_CAST(dims),0);
   }
}

jl_value_t *getJuliaNullElement(const unsigned char &c)
{
   return 0;
}

jl_value_t *getJuliaStringFromSTDString(const string &s)
{
   return jl_cstr_to_string(s.c_str());
}

template <typename V,typename E,jl_value_t *(&getElement)(const V&)> jl_array_t *arrayFromElements(const shared_ptr<nj::Value> &val,jl_datatype_t *jl_element_type)
{
   const nj::Array<V,E> &A = static_cast<nj::Array<V,E>&>(*val);
   jl_value_t *atype = jl_apply_array_type(jl_element_type,A.dims().size());
   vector<jl_value_t*> argv;
   nj::Kernel *kernel = nj::Kernel::getSingleton();

   for(size_t dim: A.dims()) argv.push_back(jl_box_long(dim));

   jl_value_t *dims = kernel->newTuple(argv);
   jl_array_t *A_jl = jl_new_array(atype,TUPLE_CAST(dims));
   V *A_p = A.ptr();
   jl_value_t **Ajl_p = (jl_value_t**)A_jl->data;
    
   for(size_t elNum = 0;elNum < A.size();elNum++) *Ajl_p++ = getElement(*A_p++);

   return A_jl;
}

jl_array_t *rArray(const shared_ptr<nj::Value> &array) throw(nj::JuliaException)
{
   jl_array_t *res = 0;
   const nj::Array_t *atype = static_cast<const nj::Array_t*>(array->type());

   switch(atype->etype()->id())
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
      case nj::date_type:
      {
         nj::Kernel *kernel = nj::Kernel::getSingleton();
         jl_datatype_t *dateTimeType = kernel->getDateTimeType();

         res = arrayFromElements<double,nj::Date_t,getDate>(array,dateTimeType);
      }
      break;
      case nj::regex_type:
      {
         nj::Kernel *kernel = nj::Kernel::getSingleton();
         jl_datatype_t *regexType = kernel->getRegexType();

         res = arrayFromElements<string,nj::Regex_t,getJuliaRegexFromString>(array,regexType);
      }
      break;
      case nj::any_type:
      {
          if(array->dims().size() == 0) res = zeroLengthArray(jl_any_type);
      }
      break;
   }
   return res;
}

jl_value_t *nj::rvalue(const shared_ptr<nj::Value> &value) throw(JuliaException)
{
   if(value->type() == JuliaHandle_t::instance())
   {
      const nj::JuliaHandle &h = static_cast<const nj::JuliaHandle&>(*value);   

      return h.val();
   }
   if(value->isPrimitive())
   {
      const nj::Primitive &p = static_cast<const nj::Primitive&>(*value);

      return rPrimitive(p);
   }
   else return (jl_value_t*)rArray(value);
}
