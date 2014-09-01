#include <stdio.h>
#include <iostream>
#include <string>
#include "nj-vi.h"

using namespace std;
using namespace v8;

Local<Value> buildPrimitiveResponse(const nj::Primitive &primitive);
Local<Array> buildArrayResponse(const shared_ptr<nj::Value> &value);

JuliaExecEnv *J = 0;

static shared_ptr<nj::Value> buildPrimitiveRequest(const Local<Value> &prim)
{
   shared_ptr<nj::Value> v;

   if(prim->IsBoolean()) v.reset(new nj::Boolean(prim->BooleanValue()));
   //else if(prim->IsInt32()) v.reset(new nj::Int32(prim->Int32Value()));
   //else if(prim->IsUint32()) v.reset(new nj::UInt32(prim->Uint32Value()));
   else if(prim->IsInt32() || prim->IsUint32()) v.reset(new nj::Int64(prim->IntegerValue()));
   else if(prim->IsNumber()) v.reset(new nj::Float64(prim->NumberValue()));

   return v;
}

static nj::Type *getPrimitiveType(const Local<Value> &prim)
{   
   if(prim->IsBoolean()) return nj::Boolean_t::instance();
   //else if(prim->IsInt32()) return nj::Int32_t::instance();
   //else if(prim->IsUint32()) return nj::UInt32_t::instance();
   else if(prim->IsInt32() || prim->IsUint32()) return nj::Int64_t::instance();
   else if(prim->IsNumber()) return nj::Float64_t::instance();
   return 0;
}

template <typename V,typename E> static void fillArrayRequest(shared_ptr<nj::Value> &to,const Local<Array> &from)
{
    nj::Array<V,E> &a = static_cast<nj::Array<V,E>&>(*to);
    const nj::Array_t *atype = static_cast<const nj::Array_t*>(a.type());
    V *p = a.ptr();

   if(a.dims().size() == 1)
   {
      size_t length = a.dims()[0];

      for(size_t index = 0;index < length;index++)
      {
         switch(atype->etype()->getId())
         {
            case nj::boolean_type: *p++ = from->Get(index)->BooleanValue(); break;
            case nj::int32_type: *p++ = from->Get(index)->Int32Value(); break;
            case nj::uint32_type: *p++ = from->Get(index)->Uint32Value(); break;
            case nj::int64_type: *p++ = from->Get(index)->IntegerValue(); break;
            case nj::float64_type: *p++ = from->Get(index)->NumberValue(); break;
         }
      }
   }
   else if(a.dims().size() == 2)
   {
      size_t rows = a.dims()[0];
      size_t cols = a.dims()[1];

      for(size_t row = 0;row < rows;row++)
      {
         Local<Array> rowVector = Local<Array>::Cast(from->Get(row));

         for(size_t col = 0;col < cols;col++)
         {
            switch(atype->etype()->getId())
            {
               case nj::boolean_type:
                  p[col*rows + row] = rowVector->Get(col)->BooleanValue();
               break;
               case nj::int32_type:
               {
                  Local<Value> el = rowVector->Get(col);   
          
                  if(el->IsInt32())
                  {
                     p[col*rows + row] = el->Int32Value();
                  }
               }
               break;
               case nj::uint32_type:
                  p[col*rows + row] = rowVector->Get(col)->Uint32Value();
               break;
               case nj::int64_type:
                  p[col*rows + row] = rowVector->Get(col)->IntegerValue();
               break;
               case nj::float64_type:
                  p[col*rows + row] = rowVector->Get(col)->NumberValue();
               break;
            }
         }
      }
   }
}

static shared_ptr<nj::Value> buildArrayRequest(const Local<Value> &from)
{
   shared_ptr<nj::Value> to;

   if(from->IsArray())
   {
      Local<Array> a = Local<Array>::Cast(from);
      vector<size_t> dims;

      dims.push_back(a->Length());
      if(dims[0] == 0)
      {
         to.reset(new nj::Array<char,nj::Any_t>(dims));
         return to;
      }

      Local<Value> el = a->Get(0);

      while(el->IsArray())
      {
         Local<Array> sub = Local<Array>::Cast(el);
         dims.push_back(sub->Length());
         if(dims[0] == 0) return to;
         el = sub->Get(0);
      }
      if(!el->IsObject())
      {
         nj::Type *etype = getPrimitiveType(el);

         if(etype)
         {
            switch(etype->getId())
            {
               case nj::boolean_type:
                  to.reset(new nj::Array<bool,nj::Boolean_t>(dims));
                  fillArrayRequest<bool,nj::Boolean_t>(to,a);
               break;
               case nj::int32_type:
                  to.reset(new nj::Array<int,nj::Int32_t>(dims));
                  fillArrayRequest<int,nj::Int32_t>(to,a);
               break;
               case nj::uint32_type:
                  to.reset(new nj::Array<unsigned int,nj::UInt32_t>(dims));
                  fillArrayRequest<unsigned int,nj::UInt32_t>(to,a);
               break;
               case nj::int64_type:
                  to.reset(new nj::Array<int64_t,nj::Int64_t>(dims));
                  fillArrayRequest<int64_t,nj::Int64_t>(to,a);
               break;
               case nj::float64_type:
                  to.reset(new nj::Array<double,nj::Float64_t>(dims));
                  fillArrayRequest<double,nj::Float64_t>(to,a);
               break;
            }
         }
      }
   }
   return to;
}

shared_ptr<nj::Value> buildRequest(const Local<Value> &value)
{
   if(value->IsArray()) return buildArrayRequest(value);
   return buildPrimitiveRequest(value);
}
