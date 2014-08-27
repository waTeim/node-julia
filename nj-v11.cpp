#include <stdio.h>
#include <iostream>
#include <string>
#include "Types.h"
#include "nj-vi.h"

using namespace std;
using namespace v8;

void returnNull(const FunctionCallbackInfo<Value> &args)
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

Local<Value> buildPrimitiveResponse(const nj::Primitive &primitive)
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

template<typename V,typename E> Local<Array> buildArrayResponse(const shared_ptr<nj::Value> &value)
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

Local<Array> buildArrayResponse(const shared_ptr<nj::Value> &value)
{
   const nj::Array_t *array_type = static_cast<const nj::Array_t*>(value->type());
   const nj::Type *element_type = array_type->etype();
   Isolate *I = Isolate::GetCurrent();
   EscapableHandleScope scope(I);

   switch(element_type->getId())
   {
      case nj::float64_type: return scope.Escape(buildArrayResponse<double,nj::Float64_t>(value)); break;
      case nj::float32_type: return scope.Escape(buildArrayResponse<float,nj::Float32_t>(value)); break;
      case nj::int64_type: return scope.Escape(buildArrayResponse<int64_t,nj::Int64_t>(value)); break;
      case nj::int32_type: return scope.Escape(buildArrayResponse<int,nj::Int32_t>(value)); break;
      case nj::int16_type: return scope.Escape(buildArrayResponse<short,nj::Int16_t>(value)); break;
      case nj::uint64_type: return scope.Escape(buildArrayResponse<uint64_t,nj::UInt64_t>(value)); break;
      case nj::uint32_type: return scope.Escape(buildArrayResponse<unsigned,nj::UInt32_t>(value)); break;
      case nj::uint16_type: return scope.Escape(buildArrayResponse<unsigned short,nj::UInt16_t>(value)); break;
      case nj::char_type: return scope.Escape(buildArrayResponse<char,nj::Char_t>(value)); break;
      case nj::uchar_type: return scope.Escape(buildArrayResponse<unsigned char,nj::UChar_t>(value)); break;
   }

   return scope.Escape(Array::New(I,0));
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
   String::Utf8Value juliaDirectory(arg0);

   if(!J) J = new JuliaExecEnv(*juliaDirectory);
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
   Local<Function> cb = Local<Function>::Cast(args[1]);
   String::Utf8Value text(arg0);
   JMain *engine;

   if(text.length() > 0 && (engine = J->getEngine()))
   {
      engine->evalQueuePut(*text);
      shared_ptr<vector<shared_ptr<nj::Value>>> res = engine->resultQueueGet();
  
      if(res.get())
      {
         int argc = res->size();
         Local<Value> *argv = new Local<Value>[argc];

         argc = buildResponse(res,argc,argv);
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

   Local<String> arg0 = Local<String>::Cast(args[0]);
   String::Utf8Value funcName(arg0);
   Local<Function> cb = Local<Function>::Cast(args[args.Length() - 1]);
   JMain *engine;

   if(funcName.length() > 0 && (engine = J->getEngine()))
   {
      vector<shared_ptr<nj::Value>> req;

      for(int i = 1;i < args.Length() - 1;i++)
      {
         shared_ptr<nj::Value> reqElement = buildRequest(args[i]);

         if(reqElement.get()) req.push_back(reqElement);
      }
      engine->evalQueuePut(*funcName,req);
      shared_ptr<vector<shared_ptr<nj::Value>>> res = engine->resultQueueGet();
 
      if(res.get())
      {
         int argc = res->size();
         Local<Value> *argv = new Local<Value>[argc];

         argc = buildResponse(res,argc,argv);
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
