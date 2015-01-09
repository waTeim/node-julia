#include <string>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <node_buffer.h>
#include "NativeArray.h"
#include "JuliaHandle.h"
#include "request.h"
#include "JMain.h"
#include "Trampoline.h"

#if NODE_MINOR_VERSION == 10
#include "JRef-v10.h"
#else
#include "JRef-v11.h"
#endif

using namespace std;
using namespace v8;

static nj::Type *getPrimitiveType(const Local<Value> &prim)
{
   if(prim->IsNull()) return nj::Null_t::instance();
   else if(prim->IsBoolean()) return nj::Boolean_t::instance();
   else if(prim->IsNumber())
   {
      double v_d = prim->NumberValue();

      if(trunc(v_d) == v_d) return nj::Int64_t::instance();
      return nj::Float64_t::instance();
   }
   else if(prim->IsString()) return nj::UTF8String_t::instance();
   else if(prim->IsDate()) return nj::Date_t::instance();
   else if(prim->IsRegExp()) return nj::Regex_t::instance();
   return 0;
}

static shared_ptr<nj::Value> createPrimitiveReq(const Local<Value> &prim)
{
   shared_ptr<nj::Value> v;

   if(prim->IsNull()) v.reset(new nj::Null());
   else if(prim->IsBoolean()) v.reset(new nj::Boolean(prim->BooleanValue()));
   else if(prim->IsNumber())
   {
      double v_d = prim->NumberValue();

      if(trunc(v_d) == v_d) v.reset(new nj::Int64(prim->IntegerValue()));
      else v.reset(new nj::Float64(v_d));
   }
   else if(prim->IsString())
   {
      Local<String> s = Local<String>::Cast(prim);
      String::Utf8Value text(s);

      v.reset(new nj::UTF8String(*text));
   }

   return v;
}

double getDateValue(const Local<Value> &val)
{
   Local<Date> s = Local<Date>::Cast(val);
   return s->NumberValue();
}

string getRegexValue(const Local<Value> &val)
{
   Local<RegExp> re = Local<RegExp>::Cast(val);
   Local<String> s = re->GetSource();
   String::Utf8Value text(s);

   return string(*text);
}

static shared_ptr<nj::Value> createDateReq(const Local<Value> &value)
{
   double milliseconds = getDateValue(value);

   return shared_ptr<nj::Value>(new nj::Date(milliseconds));
}

static shared_ptr<nj::Value> createRegexReq(const Local<Value> &value)
{
   string text = getRegexValue(value);

   return shared_ptr<nj::Value>(new nj::Regex(text));
}

static shared_ptr<nj::Value> createJRefReq(const Local<Object> &obj)
{
   nj::JRef *src = node::ObjectWrap::Unwrap<nj::JRef>(obj);
   shared_ptr<nj::Value> handle = dynamic_pointer_cast<nj::Value>(src->getHandle());

   return handle;
}

template <typename V,typename E> static nj::Type *nativeArrayProperties(const Local<Value> &val,int &len)
{
   Local<Object> obj = Local<Object>::Cast(val);
   nj::NativeArray<V> nat(obj);

   len = nat.len();
   return E::instance();
}

static nj::Type *examineNativeArray(const Local<Value> &val,vector<size_t> &dims,bool &determineDimensions)
{
   Local<Object> obj = val->ToObject();
   String::Utf8Value utf(obj->GetConstructorName());
   string cname(*utf);
   nj::NativeArrayType naType = nj::toType(cname);
   nj::Type *etype = 0;
   int len = 0;

   if(naType != nj::NativeArrayType::none)
   {
      switch(naType)
      {
         case nj::NativeArrayType::Int8Array: etype = nativeArrayProperties<char,nj::Int8_t>(val,len); break;
         case nj::NativeArrayType::Uint8Array: etype = nativeArrayProperties<unsigned char,nj::UInt8_t>(val,len); break;
         case nj::NativeArrayType::Int16Array: etype = nativeArrayProperties<short,nj::Int16_t>(val,len); break;
         case nj::NativeArrayType::Uint16Array: etype = nativeArrayProperties<unsigned short,nj::UInt16_t>(val,len); break;
         case nj::NativeArrayType::Int32Array: etype = nativeArrayProperties<int,nj::Int32_t>(val,len); break;
         case nj::NativeArrayType::Uint32Array: etype = nativeArrayProperties<unsigned,nj::UInt32_t>(val,len); break;
         case nj::NativeArrayType::Float32Array: etype = nativeArrayProperties<float,nj::Float32_t>(val,len); break;
         case nj::NativeArrayType::Float64Array: etype = nativeArrayProperties<double,nj::Float64_t>(val,len); break;
         default: break;
      }
      if(determineDimensions)
      {
         dims.push_back(len);
         determineDimensions = false;
      }
   }
   return etype;
}

static void examineArray(const Local<Array> &a,size_t level,vector<size_t> &dims,nj::Type *&maxType,bool &determineDimensions) throw(nj::InvalidException)
{
   size_t len = a->Length();

   for(size_t i = 0;i < len;i++)
   {
      Local<Value> el = a->Get(i);

      if(determineDimensions)
      {
         dims.push_back(len);
         if(!el->IsArray()) determineDimensions = false;
      }
      else
      {
         if(level == dims.size() || len != dims[level]) throw(nj::InvalidException("malformed array"));
         if(!el->IsArray() && level != dims.size() - 1) throw(nj::InvalidException("malformed array"));
         if(el->IsArray() && level == dims.size() - 1) throw(nj::InvalidException("malformed array"));
      }
      if(el->IsArray())
      {
         Local<Array> sub = Local<Array>::Cast(el);

         examineArray(sub,level + 1,dims,maxType,determineDimensions);
      }
      else
      {
         nj::Type *etype = 0;

         if(el->IsObject() && !el->IsDate() && !el->IsRegExp()) etype = examineNativeArray(el,dims,determineDimensions);
         else etype = getPrimitiveType(el);

         if(!etype) throw(nj::InvalidException("unknown array element type"));
         if(!maxType || *maxType < *etype) maxType = etype;
         if((maxType->id() == nj::int64_type || maxType->id() == nj::uint64_type) && etype->id() == nj::float64_type) maxType = etype;
         if(maxType->id() == nj::float64_type && (etype->id() == nj::int64_type || etype->id() == nj::uint64_type)) continue;
         if(maxType != etype && !(*maxType < *etype)) maxType = nj::Any_t::instance();
      }
   }
}

unsigned char getNullValue(const Local<Value> &val)
{
   return 0;
}

unsigned char getBooleanValue(const Local<Value> &val)
{
   return val->BooleanValue();
}

int getInt32Value(const Local<Value> &val)
{
   return val->Int32Value();
}

unsigned int getUInt32Value(const Local<Value> &val)
{
   return val->Uint32Value();
}

int64_t getInt64Value(const Local<Value> &val)
{
   return val->IntegerValue();
}

double getFloat64Value(const Local<Value> &val)
{
   return val->NumberValue();
}

string getStringValue(const Local<Value> &val)
{
   String::Utf8Value text(val);

   return string(*text);
}

template <typename V,V (&accessor)(const Local<Value>&)> static void fillSubArray(const vector<size_t> &dims,const vector<size_t> &strides,size_t ixNum,size_t offset,V *to,const Local<Array> &from)
{
   size_t numElements = dims[ixNum];
   size_t stride = strides[ixNum];

   if(ixNum == dims.size() - 1)
   {
      for(size_t elementNum = 0;elementNum < numElements;elementNum++)
      {
         *(to + offset) = accessor(from->Get(elementNum));
         offset += stride;
      }
   }
   else
   {
      for(size_t elementNum = 0;elementNum < numElements;elementNum++)
      {
         Local<Array> subArray = Local<Array>::Cast(from->Get(elementNum));

         fillSubArray<V,accessor>(dims,strides,ixNum + 1,offset,to,subArray);
         offset += stride;
      }
   }
}


template <typename V,typename E,V (&accessor)(const Local<Value>&)> static void fillArray(shared_ptr<nj::Value> &to,const Local<Array> &from)
{
   nj::Array<V,E> &a = static_cast<nj::Array<V,E>&>(*to);
   V *p = a.ptr();

   if(a.dims().size() == 1)
   {
      size_t length = a.dims()[0];

      for(size_t index = 0;index < length;index++) *p++ = accessor(from->Get(index));
   }
   else if(a.dims().size() == 2)
   {
      size_t rows = a.dims()[0];
      size_t cols = a.dims()[1];

      for(size_t row = 0;row < rows;row++)
      {
         Local<Array> rowVector = Local<Array>::Cast(from->Get(row));

         for(size_t col = 0;col < cols;col++) p[col*rows + row] = accessor(rowVector->Get(col));
      }
   }
   else
   {
      vector<size_t> strides;

      strides.push_back(1);
      for(size_t idxNum = 1;idxNum < a.dims().size();idxNum++) strides.push_back(a.dims()[idxNum - 1]*strides[idxNum - 1]);
      fillSubArray<V,accessor>(a.dims(),strides,0,0,a.ptr(),from);
   }
}

static shared_ptr<nj::Value> createArrayReqFromArray(const Local<Value> &from)
{
   shared_ptr<nj::Value> to;

   if(from->IsArray())
   {
      Local<Array> a = Local<Array>::Cast(from);
      vector<size_t> dims;
      bool determineDimensions = true;
      nj::Type *etype = 0;

      try
      {
         examineArray(a,0,dims,etype,determineDimensions);

         if(dims[0] == 0)
         {
            to.reset(new nj::Array<char,nj::Any_t>(dims));
            return to;
         }

         if(etype)
         {
            switch(etype->id())
            {
               case nj::null_type:
                  to.reset(new nj::Array<unsigned char,nj::Null_t>(dims));
                  fillArray<unsigned char,nj::Null_t,getNullValue>(to,a);
               break;
               case nj::boolean_type:
                  to.reset(new nj::Array<unsigned char,nj::Boolean_t>(dims));
                  fillArray<unsigned char,nj::Boolean_t,getBooleanValue>(to,a);
               break;
               case nj::int32_type:
                  to.reset(new nj::Array<int,nj::Int32_t>(dims));
                  fillArray<int,nj::Int32_t,getInt32Value>(to,a);
               break;
               case nj::uint32_type:
                  to.reset(new nj::Array<unsigned int,nj::UInt32_t>(dims));
                  fillArray<unsigned int,nj::UInt32_t,getUInt32Value>(to,a);
               break;
               case nj::int64_type:
                  to.reset(new nj::Array<int64_t,nj::Int64_t>(dims));
                  fillArray<int64_t,nj::Int64_t,getInt64Value>(to,a);
               break;
               case nj::float64_type:
                  to.reset(new nj::Array<double,nj::Float64_t>(dims));
                  fillArray<double,nj::Float64_t,getFloat64Value>(to,a);
               break;
               case nj::ascii_string_type:
               case nj::utf8_string_type:
                  to.reset(new nj::Array<string,nj::UTF8String_t>(dims));
                  fillArray<string,nj::UTF8String_t,getStringValue>(to,a);
               break;
               case nj::date_type:
                  to.reset(new nj::Array<double,nj::Date_t>(dims));
                  fillArray<double,nj::Date_t,getDateValue>(to,a);
               break;
               case nj::regex_type:
                  to.reset(new nj::Array<string,nj::Regex_t>(dims));
                  fillArray<string,nj::Regex_t,getRegexValue>(to,a);
               break;
            }
         }
      }
      catch(nj::InvalidException e) {}
   }
   return to;
}

static shared_ptr<nj::Value> createArrayReqFromBuffer(const Local<Value> &from)
{
   Local<Object> buffer = from->ToObject();
   char *data = node::Buffer::Data(buffer);
   size_t len = node::Buffer::Length(buffer);
   shared_ptr<nj::Value> to;
   vector<size_t> dims;

   dims.push_back(len);
   to.reset(new nj::Array<unsigned char,nj::UInt8_t>(dims));

   nj::Array<unsigned char,nj::UInt8_t> &a = static_cast<nj::Array<unsigned char,nj::UInt8_t>&>(*to);
   unsigned char *p = a.ptr();

   for(size_t index = 0;index < len;index++) *p++ = *data++;
   return to;
}

template <typename V,typename E> static shared_ptr<nj::Value> createArrayReqFromNativeArray(const Local<Object> &array)
{
   shared_ptr<nj::Value> to;
   nj::NativeArray<V> nat(array);
   const V *data = nat.dptr();

   if(data)
   {
      vector<size_t> dims;
      dims.push_back(nat.len());
      to.reset(new nj::Array<V,E>(dims));

      nj::Array<V,E> &a = static_cast<nj::Array<V,E>&>(*to);
      V *p = a.ptr();

      for(unsigned int index = 0;index < nat.len();index++) *p++ = *data++;
   }

   return to;
}

shared_ptr<nj::Value> createRequest(const Local<Value> &value)
{
   if(value->IsArray()) return createArrayReqFromArray(value);
   else if(value->IsDate()) return createDateReq(value);
   else if(value->IsRegExp()) return createRegexReq(value);
   else if(node::Buffer::HasInstance(value)) return createArrayReqFromBuffer(value);
   else if(value->IsObject())
   {
      Local<Object> obj = value->ToObject();
      String::Utf8Value utf(obj->GetConstructorName());
      string cname(*utf);

      if(cname == "JRef") return createJRefReq(obj);
      else if(cname == "Int8Array") return createArrayReqFromNativeArray<char,nj::Int8_t>(obj);
      else if(cname == "Uint8Array") return createArrayReqFromNativeArray<unsigned char,nj::UInt8_t>(obj);
      else if(cname == "Int16Array") return createArrayReqFromNativeArray<short,nj::Int16_t>(obj);
      else if(cname == "Uint16Array") return createArrayReqFromNativeArray<unsigned short,nj::UInt16_t>(obj);
      else if(cname == "Int32Array") return createArrayReqFromNativeArray<int,nj::Int32_t>(obj);
      else if(cname == "Uint32Array") return createArrayReqFromNativeArray<unsigned int,nj::UInt32_t>(obj);
      else if(cname == "Float32Array") return createArrayReqFromNativeArray<float,nj::Float32_t>(obj);
      else if(cname == "Float64Array") return createArrayReqFromNativeArray<double,nj::Float64_t>(obj);
   }
   return createPrimitiveReq(value);
}
