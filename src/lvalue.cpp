#if defined(WIN32)
#pragma warning(disable:4200)
#endif

#include <iostream>
#include <julia.h>
#include "Kernel.h"
#include "JuliaHandle.h"
#include "juliav.h"
#include <memory.h>
#include "Values.h"
#include "lvalue.h"
#include "error.h"

using namespace std;

static jl_function_t *juliaConvert = 0;
static const string JuliaSubString("SubString");
static const string JuliaDateTime("DateTime");
static const string JuliaRegex("Regex");
static const string JuliaFunction("Function");

template <typename V,typename E> static shared_ptr<nj::Value> arrayFromBuffer(jl_value_t *Ainput)
{
   shared_ptr<nj::Value> value;
   V *p = (V*)jl_array_data(Ainput);
   int ndims = jl_array_ndims(Ainput);
   vector<size_t> dims;

   for(int dim = 0;dim < ndims;dim++) dims.push_back(jl_array_dim(Ainput,dim));

   if(ndims == 1)
   {
      const nj::Type *etype = E::instance();

      switch(etype->id())
      {
         case nj::int8_type:
         case nj::uint8_type:
         case nj::int16_type:
         case nj::uint16_type:
         case nj::int32_type:
         case nj::uint32_type:
         case nj::float32_type:
         case nj::float64_type:
         {
            nj::Array<V,E> *Aoutput = new nj::Array<V,E>(dims,Ainput);

            value.reset(Aoutput);
         }
         break;
         default:
         {
            nj::Array<V,E> *Aoutput = new nj::Array<V,E>(dims);

            memcpy(Aoutput->ptr(),p,Aoutput->size()*sizeof(V));
            value.reset(Aoutput);
         }
         break;
      }
   }
   else
   {
      nj::Array<V,E> *Aoutput = new nj::Array<V,E>(dims);

      value.reset(Aoutput);
      memcpy(Aoutput->ptr(),p,Aoutput->size()*sizeof(V));
   }
   return value;
}

string getSTDStringFromJuliaString(jl_value_t *val)
{
   string res = string(jl_string_data(val));

   return res;
}

double getMilliseconds(jl_value_t *val) throw(nj::JuliaException)
{
   nj::Kernel *kernel = nj::Kernel::getSingleton();

   return kernel->toMilliseconds(val);
}

string getStringFromJuliaRegex(jl_value_t *val) throw(nj::JuliaException)
{
   JL_GC_PUSH1(&val);

   nj::Kernel *kernel = nj::Kernel::getSingleton();
   jl_value_t *pattern = kernel->getPattern(val);
   string res = string(jl_string_data(pattern));

   JL_GC_POP();

   return res;
}

template <typename V,typename E,V (&getElement)(jl_value_t*)> static shared_ptr<nj::Value> arrayFromElements(jl_value_t *Ainput)
{
   shared_ptr<nj::Value> value;
   jl_value_t **Ainput_p = (jl_value_t**)jl_array_data(Ainput);
   int ndims = jl_array_ndims(Ainput);
   vector<size_t> dims;

   for(int dim = 0;dim < ndims;dim++) dims.push_back(jl_array_dim(Ainput,dim));

   nj::Array<V,E> *Aoutput = new nj::Array<V,E>(dims);
   V *Aoutput_p = Aoutput->ptr();

   value.reset(Aoutput);

   for(size_t elNum = 0;elNum < Aoutput->size();elNum++) *Aoutput_p++ = getElement(*Ainput_p++);

   return value;
}

static shared_ptr<nj::Value> arrayOfNull(jl_value_t *Ainput)
{
   shared_ptr<nj::Value> value;
   int ndims = jl_array_ndims(Ainput);
   vector<size_t> dims;

   for(int dim = 0;dim < ndims;dim++) dims.push_back(jl_array_dim(Ainput,dim));

   nj::Array<unsigned char,nj::Null_t> *Aoutput = new nj::Array<unsigned char,nj::Null_t>(dims);
   unsigned char *Aoutput_p = Aoutput->ptr();

   value.reset(Aoutput);

   for(size_t elNum = 0;elNum < Aoutput->size();elNum++) *Aoutput_p++ = 0;

   return value;
}

static jl_value_t *convertValue(jl_value_t *from,jl_datatype_t *destType)
{
   if(!juliaConvert) juliaConvert = jl_get_function(jl_base_module,"convert");
   if(juliaConvert)
   {
      JL_GC_PUSH2(&from,&destType);

      jl_value_t *to = jl_call2(juliaConvert,(jl_value_t*)destType,from);

      JL_GC_POP();

      return to;
   }
   return 0;
}

void getNamedTypeValue(jl_value_t *from,shared_ptr<nj::Value> &value) throw(nj::JuliaException)
{
   const char *juliaTypename = jl_typename_str(jl_typeof(from));

   if(juliaTypename == JuliaFunction) return;
   if(juliaTypename == JuliaSubString)
   {
      jl_value_t *utf = convertValue(from,jl_utf8_string_type);

      if(utf) value.reset(new nj::UTF8String(jl_string_data(utf)));
   }
   else if(juliaTypename == JuliaDateTime)
   {
      value.reset(new nj::Date(getMilliseconds(from)));
   }
   else if(juliaTypename == JuliaRegex)
   {
      nj::Kernel *kernel = nj::Kernel::getSingleton();
      jl_value_t *pattern = kernel->getPattern(from);

      value.reset(new nj::Regex(jl_string_data(pattern)));
   }
   else value.reset(new nj::JuliaHandle(from,true));
}

static jl_value_t *convertArray(jl_value_t *from,jl_datatype_t *destElementType)
{
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

static shared_ptr<nj::Value> getArrayValue(jl_value_t *Ainput)
{
   shared_ptr<nj::Value> value;

   jl_value_t *elementType = jl_tparam0(jl_typeof(Ainput));

   if(elementType == (jl_value_t*)jl_float64_type) value = arrayFromBuffer<double,nj::Float64_t>(Ainput);
   else if(elementType == (jl_value_t*)jl_int64_type) value = arrayFromBuffer<int64_t,nj::Int64_t>(Ainput);
   else if(elementType == (jl_value_t*)jl_int32_type) value = arrayFromBuffer<int,nj::Int32_t>(Ainput);
   else if(elementType == (jl_value_t*)jl_int8_type) value = arrayFromBuffer<char,nj::Int8_t>(Ainput);
   else if(elementType == (jl_value_t*)jl_float32_type) value = arrayFromBuffer<float,nj::Float32_t>(Ainput);
   else if(elementType == (jl_value_t*)jl_uint64_type) value = arrayFromBuffer<uint64_t,nj::UInt64_t>(Ainput);
   else if(elementType == (jl_value_t*)jl_uint32_type) value = arrayFromBuffer<unsigned,nj::UInt32_t>(Ainput);
   else if(elementType == (jl_value_t*)jl_int16_type) value = arrayFromBuffer<short,nj::Int16_t>(Ainput);
   else if(elementType == (jl_value_t*)jl_uint8_type) value = arrayFromBuffer<unsigned char,nj::UInt8_t>(Ainput);
   else if(elementType == (jl_value_t*)jl_uint16_type) value = arrayFromBuffer<unsigned short,nj::UInt16_t>(Ainput);
   else if(elementType == (jl_value_t*)jl_ascii_string_type) value = arrayFromElements<string,nj::ASCIIString_t,getSTDStringFromJuliaString>(Ainput);
   else if(elementType == (jl_value_t*)jl_utf8_string_type) value = arrayFromElements<string,nj::UTF8String_t,getSTDStringFromJuliaString>(Ainput);
   else if(elementType == (jl_value_t*)JVOID_T) value = arrayOfNull(Ainput);
   else if(elementType == (jl_value_t*)jl_any_type && jl_array_size(Ainput,0) == 0) value = arrayOfNull(Ainput);
   else
   {
      const char *juliaTypename = 0;

      if(!jl_is_uniontype(elementType)) juliaTypename = jl_typename_str(elementType);
      if(!juliaTypename) return 0;
      if(juliaTypename == JuliaSubString)
      {
         jl_value_t *utfArray = convertArray(Ainput,jl_utf8_string_type);

         if(utfArray) value = arrayFromElements<string,nj::UTF8String_t,getSTDStringFromJuliaString>(utfArray);
      }
      else if(juliaTypename == JuliaDateTime) value = arrayFromElements<double,nj::Date_t,getMilliseconds>(Ainput);
      else if(juliaTypename == JuliaRegex) value = arrayFromElements<string,nj::Regex_t,getStringFromJuliaRegex>(Ainput);
   }

   return value;
}

void addLValueElements(jl_value_t *jlVal,vector<shared_ptr<nj::Value>> &res) throw(nj::JuliaException)
{
   if(!jlVal) return;

   if(JL_IS_NULL(jlVal))
   {
      shared_ptr<nj::Value> value(new nj::Null);

      res.push_back(value);
   }
   else if(jl_is_array(jlVal)) res.push_back(getArrayValue(jlVal));
#ifdef USES_SVEC
   else if(jl_is_svec(jlVal))
   {
      jl_svec_t *t = (jl_svec_t*)jlVal;
      size_t len = jl_svec_len(t);

      for(size_t i = 0;i < len;i++)
      {
         jl_value_t *element = jl_svecref(t,i);

         addLValueElements(element,res);
      }
   }
#endif
   else if(jl_is_tuple(jlVal))
   {
      jl_value_t *t = jlVal;

#if defined(USES_SVEC)
      size_t len = jl_nfields(t);

      for(size_t i = 0;i < len;i++)
      {
         jl_value_t *element = jl_fieldref(t,i);

         addLValueElements(element,res);
      }
#else
      size_t len = jl_tuple_len(t);

      for(size_t i = 0;i < len;i++)
      {
         jl_value_t *element = jl_tupleref(t,i);

         addLValueElements(element,res);
      }
#endif

   }
   else
   {
      shared_ptr<nj::Value> value;

      if(jl_is_float64(jlVal)) value.reset(new nj::Float64(jl_unbox_float64(jlVal)));
      else if(jl_is_int64(jlVal)) value.reset(new nj::Int64(jl_unbox_int64(jlVal)));
      else if(jl_is_int32(jlVal)) value.reset(new nj::Int32(jl_unbox_int32(jlVal)));
      else if(jl_is_int8(jlVal)) value.reset(new nj::Int8(jl_unbox_int8(jlVal)));
      else if(jl_is_utf8_string(jlVal)) value.reset(new nj::UTF8String(jl_string_data(jlVal)));
      else if(jl_is_ascii_string(jlVal)) value.reset(new nj::ASCIIString(jl_string_data(jlVal)));
      else if(jl_is_float32(jlVal)) value.reset(new nj::Float32(jl_unbox_float32(jlVal)));
      else if(jl_is_uint64(jlVal)) value.reset(new nj::UInt64(jl_unbox_uint64(jlVal)));
      else if(jl_is_uint32(jlVal)) value.reset(new nj::UInt32(jl_unbox_uint32(jlVal)));
      else if(jl_is_int16(jlVal)) value.reset(new nj::Int16(jl_unbox_int16(jlVal)));
      else if(jl_is_uint8(jlVal)) value.reset(new nj::UInt8(jl_unbox_uint8(jlVal)));
      else if(jl_is_uint16(jlVal)) value.reset(new nj::UInt16(jl_unbox_uint16(jlVal)));
      else if(jl_is_bool(jlVal)) value.reset(new nj::Boolean(jl_unbox_bool(jlVal)));
      else getNamedTypeValue(jlVal,value);

      if(value.get()) res.push_back(value);
   }
}

vector<shared_ptr<nj::Value>> nj::lvalue(jl_value_t *jlVal) throw(JuliaException)
{
   vector<shared_ptr<nj::Value>> res;

   addLValueElements(jlVal,res);

   return res;
}
