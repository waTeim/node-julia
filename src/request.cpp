#include <math.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "request.h"

using namespace std;
using namespace v8;

static nj::Type *getPrimitiveType(const Local<Value> &prim)
{   
   if(prim->IsBoolean()) return nj::Boolean_t::instance();
   else if(prim->IsNumber())
   {
      double v_d = prim->NumberValue();

      if(trunc(v_d) == v_d) return nj::Int64_t::instance();
      return nj::Float64_t::instance();
   }
   else if(prim->IsString()) return nj::UTF8String_t::instance();
   return 0;
}

static shared_ptr<nj::Value> buildPrimitive(const Local<Value> &prim)
{
   shared_ptr<nj::Value> v;

   if(prim->IsBoolean()) v.reset(new nj::Boolean(prim->BooleanValue()));
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

static void examineArray(Local<Array> &a,size_t level,vector<size_t> &dims,nj::Type *&etype,bool &determineDimensions) throw(nj::InvalidException)
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

         examineArray(sub,level + 1,dims,etype,determineDimensions);
      }
      else
      {
         nj::Type *etypeNarrow = getPrimitiveType(el);

         if(!etypeNarrow) throw(nj::InvalidException("unknown array element type"));
         if(!etype || *etype < *etypeNarrow) etype = etypeNarrow;
         if((etype->getId() == nj::int64_type || etype->getId() == nj::uint64_type) && etypeNarrow->getId() == nj::float64_type) etype = etypeNarrow;
         if(etype != etypeNarrow && !(*etype < *etypeNarrow)) etype = nj::Any_t::instance();
      }
   }
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
}

static shared_ptr<nj::Value> buildArray(const Local<Value> &from)
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
            switch(etype->getId())
            {
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
            }
         }
      }
      catch(nj::InvalidException e) {}
   }
   return to;
}

shared_ptr<nj::Value> buildRequest(const Local<Value> &value)
{
   if(value->IsArray()) return buildArray(value);
   return buildPrimitive(value);
}
