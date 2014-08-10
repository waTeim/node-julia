#include <julia.h>
#include "rvalue.h"
#include "Values.h"

using namespace std;

static jl_value_t *rPrimitive(const nj::Primitive &prim)
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
      case nj::char_type:
      {
         const nj::Char &v = static_cast<const nj::Char&>(prim);

         res = jl_box_char(v.val());
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
      case nj::uchar_type:
      {
         const nj::UChar &v = static_cast<const nj::UChar&>(prim);

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
      case nj::string_type:
      {
         const nj::String &v = static_cast<const nj::String&>(prim);

         res = jl_cstr_to_string(v.val().c_str());
      }
      break;
   }
   return res;
}

template<typename V,typename E> static jl_array_t *rArray(const shared_ptr<nj::Value> &array,jl_datatype_t *jl_element_type)
{
   const nj::Array<V,E> &a = static_cast<nj::Array<V,E>&>(*array);
   jl_value_t *jl_atype = jl_apply_array_type(jl_element_type,a.dims().size());
   jl_tuple_t *dims = jl_alloc_tuple(a.dims().size());
   int i = 0;

   for(size_t dim: a.dims()) jl_tupleset(dims,i++,jl_box_long(dim));

   return jl_ptr_to_array(jl_atype,a.ptr(),dims,0);
}


static jl_array_t *rArray(const shared_ptr<nj::Value> &array)
{
   jl_array_t *res = 0;
   const nj::Array_t *atype = static_cast<const nj::Array_t*>(array->type());

   switch(atype->etype()->getId())
   {  
      case nj::boolean_type: res = rArray<bool,nj::Boolean_t>(array,jl_bool_type); break;
      case nj::int64_type: res = rArray<int64_t,nj::Int64_t>(array,jl_int64_type); break;
      case nj::int32_type: res = rArray<int,nj::Int32_t>(array,jl_int32_type); break;
      case nj::int16_type: res = rArray<short,nj::Int16_t>(array,jl_int16_type); break;
      case nj::uint64_type: res = rArray<uint64_t,nj::UInt64_t>(array,jl_uint64_type); break;
      case nj::uint32_type: res = rArray<unsigned int,nj::UInt32_t>(array,jl_uint32_type); break;
      case nj::uint16_type: res = rArray<unsigned short,nj::UInt16_t>(array,jl_uint16_type); break;
      case nj::float64_type: res = rArray<double,nj::Float64_t>(array,jl_float64_type); break;
      case nj::float32_type: res = rArray<float,nj::Float32_t>(array,jl_float32_type); break;
      case nj::char_type: res = rArray<char,nj::Char_t>(array,jl_int8_type); break;
      case nj::uchar_type: res = rArray<unsigned char,nj::UChar_t>(array,jl_uint8_type); break;
   }
   return res;
}

jl_value_t *nj::rvalue(const shared_ptr<nj::Value> &value)
{
   if(value->isPrimitive())
   {
      const nj::Primitive &p = static_cast<const nj::Primitive&>(*value);

      return rPrimitive(p);
   }
   else return (jl_value_t*)rArray(value);
}
