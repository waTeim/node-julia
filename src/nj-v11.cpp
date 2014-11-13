#include <stdio.h>
#include <iostream>
#include <string>
#include <node_buffer.h>
#include "Types.h"
#include "request.h"
#include "ScriptEncapsulated-v11.h"

using namespace std;
using namespace v8;

void returnNull(Isolate *I,const FunctionCallbackInfo<Value> &args)
{
   args.GetReturnValue().SetNull();
}

void callback(Isolate *I,const FunctionCallbackInfo<Value>& args,const Local<Function> &cb,int argc,Local<Value> *argv)
{
  cb->Call(I->GetCurrentContext()->Global(),argc,argv);
}

Local<Value> createPrimitiveRes(HandleScope &scope,const nj::Primitive &primitive)
{
   Isolate *I = Isolate::GetCurrent();

   switch(primitive.type()->getId())
   {
      case nj::null_type:
      {
         Local<Value> dest = Null(I);

         return dest;
      }
      break;
      case nj::boolean_type:
      {
         Local<Value> dest = Boolean::New(I,primitive.toBoolean());

         return dest;
      }
      break;
      case nj::int64_type:
      case nj::int32_type:
      case nj::int16_type:
      case nj::int8_type:
      {
         Local<Value> dest = Number::New(I,primitive.toInt());

         return dest;
      }
      break;
      case nj::uint64_type:
      case nj::uint32_type:
      case nj::uint16_type:
      case nj::uint8_type:
      {
         Local<Value> dest = Number::New(I,primitive.toUInt());

         return dest;
      }
      break;
      case nj::float64_type:
      case nj::float32_type:
      {
         Local<Value> dest = Number::New(I,primitive.toFloat());

         return dest;
      }
      break;
      case nj::ascii_string_type:
      case nj::utf8_string_type:
      {
         Local<Value> dest = String::NewFromUtf8(I,primitive.toString().c_str());

         return dest;
      }
      break;
      case nj::date_type:
      {
         Local<Value> dest = Date::New(I,primitive.toFloat());

         return dest;
      }
      break;
   }

   return Null(I);
}

template <typename V> Local<Value> getNumberFromValue(Isolate *I,const V &val)
{
   return Number::New(I,val);
}

Local<Value> getStringFromValue(Isolate *I,const string &val)
{
   return String::NewFromUtf8(I,val.c_str());
}

Local<Value> getNullValue(Isolate *I,const unsigned char &val)
{
   return Null(I);
}

template<typename V,typename E,Local<Value> getElement(Isolate *I,const V &val)> Local<Array> createArrayRes(HandleScope &scope,const shared_ptr<nj::Value> &value)
{
   Isolate *I = Isolate::GetCurrent();
   const nj::Array<V,E> &array = static_cast<const nj::Array<V,E>&>(*value);

   if(array.size() == 0) return Local<Array>();
   if(array.dims().size() == 1)
   {
      size_t size0 = array.dims()[0];
      V *p = array.ptr();
      Local<Array> dest = Array::New(I,size0);

      for(size_t i = 0;i < size0;i++) dest->Set(i,getElement(I,p[i]));
      return dest;
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
         for(size_t j = 0;j < size1;j++) row->Set(j,getElement(I,p[size0*j + i]));
      }
      return dest;
   }
   return Array::New(I,0);
}

Local<Array> createArrayRes(HandleScope &scope,const shared_ptr<nj::Value> &value,const nj::Type *elementType)
{
   switch(elementType->getId())
   {
      case nj::null_type: return createArrayRes<unsigned char,nj::Null_t,getNullValue>(scope,value); break;
      case nj::float64_type: return createArrayRes<double,nj::Float64_t,getNumberFromValue<double>>(scope,value); break;
      case nj::float32_type: return createArrayRes<float,nj::Float32_t,getNumberFromValue<float>>(scope,value); break;
      case nj::int64_type: return createArrayRes<int64_t,nj::Int64_t,getNumberFromValue<int64_t>>(scope,value); break;
      case nj::int32_type: return createArrayRes<int,nj::Int32_t,getNumberFromValue<int>>(scope,value); break;
      case nj::int16_type: return createArrayRes<short,nj::Int16_t,getNumberFromValue<short>>(scope,value); break;
      case nj::int8_type: return createArrayRes<char,nj::Int8_t,getNumberFromValue<char>>(scope,value); break;
      case nj::uint64_type: return createArrayRes<uint64_t,nj::UInt64_t,getNumberFromValue<uint64_t>>(scope,value); break;
      case nj::uint32_type: return createArrayRes<unsigned,nj::UInt32_t,getNumberFromValue<unsigned>>(scope,value); break;
      case nj::uint16_type: return createArrayRes<unsigned short,nj::UInt16_t,getNumberFromValue<unsigned short>>(scope,value); break;
      case nj::ascii_string_type: return createArrayRes<string,nj::ASCIIString_t,getStringFromValue>(scope,value); break;
      case nj::utf8_string_type: return createArrayRes<string,nj::UTF8String_t,getStringFromValue>(scope,value); break;
   }

   Isolate *I = Isolate::GetCurrent();

   return Array::New(I,0);
}

Local<Object> createBufferRes(HandleScope &scope,const shared_ptr<nj::Value> &value)
{
   const nj::Array<unsigned char,nj::UInt8_t> &array = static_cast<const nj::Array<unsigned char,nj::UInt8_t>&>(*value);
   Isolate *I = Isolate::GetCurrent();
   size_t length = array.dims()[0];
   unsigned char *p = array.ptr();
   Local<Object> buffer = node::Buffer::New(I,length);

   memcpy(node::Buffer::Data(buffer),p,length);
   return buffer;
}

int createResponse(HandleScope &scope,const shared_ptr<nj::Result> &res,int argc,Local<Value> *argv)
{
   int index = 0;

   for(shared_ptr<nj::Value> value: res->results())
   {
      if(value.get())
      {
         if(value->isPrimitive())
         {
            const nj::Primitive &primitive = static_cast<const nj::Primitive&>(*value);

            argv[index++] = createPrimitiveRes(scope,primitive);
         }
         else
         {
            const nj::Array_t *array_type = static_cast<const nj::Array_t*>(value->type());
            const nj::Type *etype = array_type->etype();

            if(etype == nj::UInt8_t::instance()  && value->dims().size() == 1) argv[index++] = createBufferRes(scope,value);
            else argv[index++] = createArrayRes(scope,value,etype);
         }
      }
   }
   return index;
}

void raiseException(const FunctionCallbackInfo<Value> &args,HandleScope &scope,shared_ptr<nj::Result> &res)
{
   Isolate *I = Isolate::GetCurrent();
   int exId = res->exId();

   switch(exId)
   {
      case nj::Exception::julia_undef_var_error_exception:
      case nj::Exception::julia_method_error_exception:
         I->ThrowException(Exception::ReferenceError(String::NewFromUtf8(I,res->exText().c_str())));
      break;
      default:
         I->ThrowException(Exception::Error(String::NewFromUtf8(I,res->exText().c_str())));
      break;
   }
   args.GetReturnValue().SetUndefined();
}

void callbackWithResult(const FunctionCallbackInfo<Value> &args,HandleScope &scope,Local<Function> &cb,shared_ptr<nj::Result> &res)
{
   Isolate *I = Isolate::GetCurrent();

   if(res.get())
   {
      int exId = res->exId();

      if(exId != nj::Exception::no_exception) raiseException(args,scope,res);
      else
      {
         int argc = res->results().size();
         Local<Value> *argv = new Local<Value>[argc];

         argc = createResponse(scope,res,argc,argv);
         callback(I,args,cb,argc,argv);
      }
   }
   else callback(I,args,cb,0,0);
}

void returnResult(const FunctionCallbackInfo<Value> &args,HandleScope &scope,shared_ptr<nj::Result> &res)
{
   Isolate *I = Isolate::GetCurrent();

   if(res.get())
   {
      int exId = res->exId();

      if(exId != nj::Exception::no_exception) raiseException(args,scope,res);
      else
      {
         int argc = res->results().size();
         Local<Value> *argv = new Local<Value>[argc];

         argc = createResponse(scope,res,argc,argv);

         if(argc != 0)
         {
            if(argc > 1)
            {
               Local<Array> rV = Array::New(I,argc);

               for(int i = 0;i < argc;i++) rV->Set(i,argv[i]);

               args.GetReturnValue().Set(rV);
            }
            else args.GetReturnValue().Set(argv[0]);
         }
         else returnNull(I,args);
      }
   }
   else args.GetReturnValue().SetUndefined();
}


void doEval(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   int numArgs = args.Length();

   if(numArgs  == 0 || numArgs > 2 || (numArgs == 2 && !args[1]->IsFunction()))
   {
      returnNull(I,args);
      return;
   }

   if(!J) J = new JuliaExecEnv();

   Local<String> arg0 = args[0]->ToString();
   Local<Function> cb;
   String::Utf8Value text(arg0);
   JMain *engine;

   if(numArgs == 2) cb = Local<Function>::Cast(args[1]);

   if(text.length() > 0 && (engine = J->getEngine()))
   {
      engine->eval(*text);
      shared_ptr<nj::Result> res = engine->resultQueueGet();

      if(numArgs == 2) callbackWithResult(args,scope,cb,res);
      else returnResult(args,scope,res);
   }
   else
   {
      if(numArgs == 2)
      {
         const unsigned argc = 1;
         Local<Value> argv[argc] = { String::NewFromUtf8(I,"") };

         callback(I,args,cb,argc,argv);
      }
      else returnNull(I,args);
   }
}

void doExec(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   int numArgs = args.Length();
   bool useCallback = false;

   if(numArgs == 0)
   {
      returnNull(I,args);
      return;
   }

   if(!J) J = new JuliaExecEnv();

   Local<String> arg0 = Local<String>::Cast(args[0]);
   String::Utf8Value funcName(arg0);
   Local<Function> cb;
   JMain *engine;

   if(numArgs >= 2 && args[args.Length() - 1]->IsFunction())
   {
      useCallback = true;
      cb = Local<Function>::Cast(args[args.Length() - 1]);
   }

   if(funcName.length() > 0 && (engine = J->getEngine()))
   {
      vector<shared_ptr<nj::Value>> req;
      int numExecArgs = numArgs - 1;

      for(int i = 0;i < numExecArgs;i++)
      {
         shared_ptr<nj::Value> reqElement = createRequest(args[i + 1]);

         if(reqElement.get()) req.push_back(reqElement);
      }
      engine->exec(*funcName,req);
      shared_ptr<nj::Result> res = engine->resultQueueGet();
 
      if(useCallback) callbackWithResult(args,scope,cb,res);
      else returnResult(args,scope,res);
   }
   else
   {
      if(useCallback)
      {
         const unsigned argc = 1;
         Local<Value> argv[argc] = { String::NewFromUtf8(I,"") };

         callback(I,args,cb,argc,argv);
      }
      else returnNull(I,args);
   }
}

void newScript(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);

   nj::ScriptEncapsulated::NewInstance(args);
}

void init(Handle<Object> exports)
{
  nj::ScriptEncapsulated::Init(exports);

  NODE_SET_METHOD(exports,"eval",doEval);
  NODE_SET_METHOD(exports,"exec",doExec);
  NODE_SET_METHOD(exports,"newScript",newScript);
}

NODE_MODULE(nj,init)
