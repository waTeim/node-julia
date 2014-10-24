#include <stdio.h>
#include <iostream>
#include <string>
#include "Types.h"
#include "request.h"
#include "ScriptEncapsulated-v10.h"

using namespace std;
using namespace v8;

Handle<Value> returnString(HandleScope &scope,const string &s)
{
   return scope.Close(String::New(s.c_str()));
}

Handle<Value> callback(HandleScope &scope,const Local<Function> &cb,int argc,Local<Value> *argv)
{
   cb->Call(Context::GetCurrent()->Global(),argc,argv);
   return scope.Close(Undefined());
}

Local<Value> buildPrimitiveResponse(HandleScope &scope,const nj::Primitive &primitive)
{

   switch(primitive.type()->getId())
   {
      case nj::null_type:
      {
         Local<Value> dest = Local<Value>::New(Null());

         return dest;
      }
      break;
      case nj::boolean_type:
      {
         Local<Value> dest = Boolean::New(primitive.toBoolean())->ToBoolean();

         return dest;
      }
      break;
      case nj::int64_type:
      case nj::int32_type:
      case nj::int16_type:
      case nj::int8_type:
      {
         Local<Value> dest = Number::New(primitive.toInt());

         return dest;
      }
      break;
      case nj::uint64_type:
      case nj::uint32_type:
      case nj::uint16_type:
      case nj::uint8_type:
      {
         Local<Value> dest = Number::New(primitive.toUInt());

         return dest;
      }
      break;
      case nj::float64_type:
      case nj::float32_type:
      {
         Local<Value> dest = Number::New(primitive.toFloat());

         return dest;
      }
      break;
      case nj::ascii_string_type:
      case nj::utf8_string_type:
      {
         Local<Value> dest = String::New(primitive.toString().c_str());

         return dest;
      }
      break;
   }

   return Local<Value>::New(Null());
}

template<typename V,typename E> Local<Array> buildArrayResponse(HandleScope &scope,const shared_ptr<nj::Value> &value)
{
   const nj::Array<V,E> &array = static_cast<const nj::Array<V,E>&>(*value);

   if(array.size() == 0) return Local<Array>();
   if(array.dims().size() == 1)
   {
      size_t size0 = array.dims()[0];
      V *p = array.ptr();
      Local<Array> dest = Array::New(size0);

      for(size_t i = 0;i < size0;i++) dest->Set(i,Number::New(p[i]));
      return dest;
   }
   else if(array.dims().size() == 2)
   {
      size_t size0 = array.dims()[0];
      size_t size1 = array.dims()[1];
      V *p = array.ptr();
      Local<Array> dest = Array::New(size0);

      for(size_t i = 0;i < size0;i++)
      {
         Local<Array> row  = Array::New(size1);

         dest->Set(i,row);
         for(size_t j = 0;j < size1;j++) row->Set(j,Number::New(p[size0*j + i]));
      }
      return dest;
   }
   return Array::New(0);
}

Local<Array> buildArrayResponse(HandleScope &scope,const shared_ptr<nj::Value> &value)
{
   const nj::Array_t *array_type = static_cast<const nj::Array_t*>(value->type());
   const nj::Type *element_type = array_type->etype();

   switch(element_type->getId())
   {
      case nj::float64_type: return buildArrayResponse<double,nj::Float64_t>(scope,value); break;
      case nj::float32_type: return buildArrayResponse<float,nj::Float32_t>(scope,value); break;
      case nj::int64_type: return buildArrayResponse<int64_t,nj::Int64_t>(scope,value); break;
      case nj::int32_type: return buildArrayResponse<int,nj::Int32_t>(scope,value); break;
      case nj::int16_type: return buildArrayResponse<short,nj::Int16_t>(scope,value); break;
      case nj::int8_type: return buildArrayResponse<char,nj::Int8_t>(scope,value); break;
      case nj::uint64_type: return buildArrayResponse<uint64_t,nj::UInt64_t>(scope,value); break;
      case nj::uint32_type: return buildArrayResponse<unsigned,nj::UInt32_t>(scope,value); break;
      case nj::uint16_type: return buildArrayResponse<unsigned short,nj::UInt16_t>(scope,value); break;
      case nj::uint8_type: return buildArrayResponse<unsigned char,nj::UInt8_t>(scope,value); break;
   }

   return Array::New(0);
}

int buildResponse(HandleScope &scope,const shared_ptr<nj::Result> &res,int argc,Local<Value> *argv)
{
   int index = 0;

   for(shared_ptr<nj::Value> value: res->results())
   {
      if(value.get())
      {
         if(value->isPrimitive())
         {
            const nj::Primitive &primitive = static_cast<const nj::Primitive&>(*value);

            argv[index++] = buildPrimitiveResponse(scope,primitive);
         }
         else
         {
            argv[index++] = buildArrayResponse(scope,value);
         }
      }
   }
   return index;
}

Handle<Value> callbackWithResult(HandleScope &scope,Local<Function> &cb,shared_ptr<nj::Result> &res)
{
   if(res.get())
   {
      int exId = res->exId();

      if(exId != nj::Exception::no_exception)
      {
         switch(exId)
         {
            case nj::Exception::julia_undef_var_error_exception:
            case nj::Exception::julia_method_error_exception:
               ThrowException(Exception::ReferenceError(String::New(res->exText().c_str())));
            break;
            default:
               ThrowException(Exception::Error(String::New(res->exText().c_str())));
            break;
         }
         return scope.Close(Undefined());
      }
      else
      {
         int argc = res->results().size();
         Local<Value> *argv = new Local<Value>[argc];

         argc = buildResponse(scope,res,argc,argv);
         return callback(scope,cb,argc,argv);
      }
   }
   else return callback(scope,cb,0,0);
}


Handle<Value> doEval(const Arguments &args)
{
   HandleScope scope;
   int numArgs = args.Length();

   if(numArgs < 2) return scope.Close(Null());
   if(!J) J = new JuliaExecEnv();

   Local<String> arg0 = Local<String>::Cast(args[0]);
   Local<Function> cb = Local<Function>::Cast(args[1]);
   String::Utf8Value text(arg0);
   JMain *engine;

   if(text.length() > 0 && (engine = J->getEngine()))
   {
      engine->eval(*text);
      shared_ptr<nj::Result> res = engine->resultQueueGet();
 
      return callbackWithResult(scope,cb,res);
   }
   else
   {
      const unsigned argc = 1;
      Local<Value> argv[argc] = { String::New("") };
      return callback(scope,cb,argc,argv);
   }
}

Handle<Value> doExec(const Arguments &args)
{
   HandleScope scope;
   int numArgs = args.Length();

   if(numArgs < 2) return scope.Close(Null());
   if(!J) J = new JuliaExecEnv();

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
      engine->exec(*funcName,req);
      shared_ptr<nj::Result> res = engine->resultQueueGet();

      return callbackWithResult(scope,cb,res);
   }
   else
   {
      const unsigned argc = 1;
      Local<Value> argv[argc] = { String::New("") };

      return callback(scope,cb,argc,argv);
   }
}

Handle<Value> newScript(const Arguments& args)
{
  HandleScope scope;

  return scope.Close(nj::ScriptEncapsulated::NewInstance(args));
}

void init(Handle<Object> exports)
{
   nj::ScriptEncapsulated::Init(exports);

   NODE_SET_METHOD(exports,"eval",doEval);
   NODE_SET_METHOD(exports,"exec",doExec);
   NODE_SET_METHOD(exports,"newScript",newScript);
}

NODE_MODULE(nj,init)
