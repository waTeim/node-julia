#include <stdio.h>
#include <iostream>
#include <node.h>
#include <v8.h>
#include <string>
#include "Types.h"
#include "JuliaExecEnv.h"

using namespace std;
using namespace v8;

static JuliaExecEnv *J = 0;

void returnNull(const FunctionCallbackInfo<Value> &args,Isolate *I)
{
   args.GetReturnValue().SetNull();
}

void returnString(const FunctionCallbackInfo<Value> &args,Isolate *I,const string &s)
{
   args.GetReturnValue().Set(String::NewFromUtf8(I,s.c_str()));
}

void callback(const FunctionCallbackInfo<Value>& args,Isolate *I,const Local<Function> &cb,int argc,Local<Value> *argv)
{
  cb->Call(I->GetCurrentContext()->Global(),argc,argv);
}

Local<Value> buildPrimitiveRes(const nj::Primitive &primitive)
{
   Isolate *I = Isolate::GetCurrent();
   EscapableHandleScope scope(I);

   switch(primitive.type()->getId())
   {
      case nj::null_type:
      {
         Local<Value> dest = Null(I);

         return scope.Escape(dest);
      }
      break;
      case nj::boolean_type:
      {
         Local<Value> dest = Boolean::New(I,primitive.toBoolean());

         return scope.Escape(dest);
      }
      break;
      case nj::char_type:
      {
         Local<Value> dest = String::NewFromUtf8(I,primitive.toString().c_str());

         return scope.Escape(dest);
      }
      break;
      case nj::int64_type:
      case nj::int32_type:
      case nj::int16_type:
      {
         Local<Value> dest = Number::New(I,primitive.toInt());

         return scope.Escape(dest);
      }
      break;
      case nj::uint64_type:
      case nj::uint32_type:
      case nj::uint16_type:
      case nj::uchar_type:
      {
         Local<Value> dest = Number::New(I,primitive.toUInt());

         return scope.Escape(dest);
      }
      break;
      case nj::float64_type:
      case nj::float32_type:
      {
         Local<Value> dest = Number::New(I,primitive.toFloat());

         return scope.Escape(dest);
      }
      break;
      case nj::string_type:
      {
         Local<Value> dest = String::NewFromUtf8(I,primitive.toString().c_str());

         return scope.Escape(dest);
      }
      break;
   }

   return scope.Escape(Array::New(I,0));
}

template<typename V,typename E> Local<Array> buildArrayRes(const shared_ptr<nj::Value> &value)
{
   Isolate *I = Isolate::GetCurrent();
   EscapableHandleScope scope(I);
   const nj::Array<V,E> &array = static_cast<const nj::Array<V,E>&>(*value);

   if(array.size() == 0) return Local<Array>();
   if(array.dims().size() == 1)
   {
      size_t size0 = array.dims()[0];
      V *p = array.ptr();
      Local<Array> dest = Array::New(I,size0);

      for(size_t i = 0;i < size0;i++) dest->Set(i,Number::New(I,p[i]));
      return scope.Escape(dest);
   }
   else if(array.dims().size() == 2)
   {
      size_t size0 = array.dims()[0];
      size_t size1 = array.dims()[1];
      V *p = array.ptr();
      Local<Array> dest = Array::New(I,size0);

      for(size_t i = 0;i < size0;i++)
      {
         Local<Array> row  = Array::New(I,size1);

         dest->Set(i,row);
         for(size_t j = 0;j < size1;j++) row->Set(j,Number::New(I,p[size0*j + i]));
      }
      return scope.Escape(dest);
   }
   return scope.Escape(Array::New(I,0));
}

Local<Array> buildArrayRes(const shared_ptr<nj::Value> &value)
{
   const nj::Array_t *array_type = static_cast<const nj::Array_t*>(value->type());
   const nj::Type *element_type = array_type->etype();
   Isolate *I = Isolate::GetCurrent();
   EscapableHandleScope scope(I);

   switch(element_type->getId())
   {
      case nj::float64_type: return scope.Escape(buildArrayRes<double,nj::Float64_t>(value)); break;
      case nj::float32_type: return scope.Escape(buildArrayRes<float,nj::Float32_t>(value)); break;
      case nj::int64_type: return scope.Escape(buildArrayRes<int64_t,nj::Int64_t>(value)); break;
      case nj::int32_type: return scope.Escape(buildArrayRes<int,nj::Int32_t>(value)); break;
      case nj::int16_type: return scope.Escape(buildArrayRes<short,nj::Int16_t>(value)); break;
      case nj::uint64_type: return scope.Escape(buildArrayRes<uint64_t,nj::UInt64_t>(value)); break;
      case nj::uint32_type: return scope.Escape(buildArrayRes<unsigned,nj::UInt32_t>(value)); break;
      case nj::uint16_type: return scope.Escape(buildArrayRes<unsigned short,nj::UInt16_t>(value)); break;
      case nj::char_type: return scope.Escape(buildArrayRes<char,nj::Char_t>(value)); break;
      case nj::uchar_type: return scope.Escape(buildArrayRes<unsigned char,nj::UChar_t>(value)); break;
   }

   return scope.Escape(Array::New(I,0));
}

int buildRes(Isolate *I,const shared_ptr<vector<shared_ptr<nj::Value>>> &res,int argc,Local<Value> *argv)
{
   int index = 0;

   for(shared_ptr<nj::Value> value: *res)
   {
      if(value.get())
      {
         if(value->isPrimitive())
         {
            const nj::Primitive &primitive = static_cast<const nj::Primitive&>(*value);

            argv[index++] = buildPrimitiveRes(primitive);
         }
         else
         {
            argv[index++] = buildArrayRes(value);
         }
      }
   }
   return index;
}

shared_ptr<nj::Value> buildPrimitiveReq(const Local<Value> &prim)
{
   shared_ptr<nj::Value> v;

   if(prim->IsBoolean()) v.reset(new nj::Boolean(prim->BooleanValue()));
   //else if(prim->IsInt32()) v.reset(new nj::Int32(prim->Int32Value()));
   //else if(prim->IsUint32()) v.reset(new nj::UInt32(prim->Uint32Value()));
   else if(prim->IsInt32() || prim->IsUint32()) v.reset(new nj::Int64(prim->IntegerValue()));
   else if(prim->IsNumber()) v.reset(new nj::Float64(prim->NumberValue()));

   return v;
}

nj::Type *getPrimitiveType(const Local<Value> &prim)
{   
   if(prim->IsBoolean()) return nj::Boolean_t::instance();
   //else if(prim->IsInt32()) return nj::Int32_t::instance();
   //else if(prim->IsUint32()) return nj::UInt32_t::instance();
   else if(prim->IsInt32() || prim->IsUint32()) return nj::Int64_t::instance();
   else if(prim->IsNumber()) return nj::Float64_t::instance();
   return 0;
}

template <typename V,typename E> void fillArrayReq(shared_ptr<nj::Value> &to,const Local<Array> &from)
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

shared_ptr<nj::Value> buildArrayReq(const Local<Value> &from)
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
                  fillArrayReq<bool,nj::Boolean_t>(to,a);
               break;
               case nj::int32_type:
                  to.reset(new nj::Array<int,nj::Int32_t>(dims));
                  fillArrayReq<int,nj::Int32_t>(to,a);
               break;
               case nj::uint32_type:
                  to.reset(new nj::Array<unsigned int,nj::UInt32_t>(dims));
                  fillArrayReq<unsigned int,nj::UInt32_t>(to,a);
               break;
               case nj::int64_type:
                  to.reset(new nj::Array<int64_t,nj::Int64_t>(dims));
                  fillArrayReq<int64_t,nj::Int64_t>(to,a);
               break;
               case nj::float64_type:
                  to.reset(new nj::Array<double,nj::Float64_t>(dims));
                  fillArrayReq<double,nj::Float64_t>(to,a);
               break;
            }
         }
      }
   }
   return to;
}

shared_ptr<nj::Value> buildReq(const Local<Value> &value)
{
   if(value->IsArray()) return buildArrayReq(value);
   return buildPrimitiveReq(value);
}

void doStart(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   int numArgs = args.Length();

   if(numArgs == 0)
   {
      if(!J) J = new JuliaExecEnv();
      returnString(args,I,"Julia Started");
      return;
   }

   Local<String> arg0 = args[0]->ToString();
   String::Utf8Value plainText_av(arg0);

   if(!J) J = new JuliaExecEnv(*plainText_av);
   returnString(args,I,"Julia Started");
}

void doEval(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   int numArgs = args.Length();

   if(numArgs < 2 || !J)
   {
      returnNull(args,I);
      return;
   }

   Local<String> arg0 = args[0]->ToString();
   String::Utf8Value text(arg0);
   Local<Function> cb = Local<Function>::Cast(args[1]);
   JMain *engine;

   if(text.length() > 0 && (engine = J->getEngine()))
   {
      engine->evalQueuePut(*text);
      shared_ptr<vector<shared_ptr<nj::Value>>> res = engine->resultQueueGet();
  
      if(res.get())
      {
         int argc = res->size();
         Local<Value> *argv = new Local<Value>[argc];
         argc = buildRes(I,res,argc,argv);
         callback(args,I,cb,argc,argv);
      }
   }
   else
   {
      const unsigned argc = 1;
      Local<Value> argv[argc] = { String::NewFromUtf8(I,"") };
      callback(args,I,cb,argc,argv);
   }
}

void doExec(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   int numArgs = args.Length();

   if(numArgs < 2 || !J)
   {
      returnNull(args,I);
      return;
   }

   Local<String> arg0 = args[0]->ToString();
   String::Utf8Value funcName(arg0);
   Local<Function> cb = Local<Function>::Cast(args[args.Length() - 1]);
   JMain *engine;

   if(funcName.length() > 0 && (engine = J->getEngine()))
   {
      vector<shared_ptr<nj::Value>> req;

      for(int i = 1;i < args.Length() - 1;i++)
      {
         shared_ptr<nj::Value> reqElement = buildReq(args[i]);

         if(reqElement.get()) req.push_back(reqElement);
      }
      engine->evalQueuePut(*funcName,req);
      shared_ptr<vector<shared_ptr<nj::Value>>> res = engine->resultQueueGet();
 
      if(res.get())
      {
         int argc = res->size();
         Local<Value> *argv = new Local<Value>[argc];
         argc = buildRes(I,res,argc,argv);
         callback(args,I,cb,argc,argv);
      }
   }
   else
   {
      const unsigned argc = 1;
      Local<Value> argv[argc] = { String::NewFromUtf8(I,"") };
      callback(args,I,cb,argc,argv);
   }
}

void init(Handle<Object> exports)
{
  NODE_SET_METHOD(exports,"start",doStart);
  NODE_SET_METHOD(exports,"eval",doEval);
  NODE_SET_METHOD(exports,"exec",doExec);
}

NODE_MODULE(nj,init)
