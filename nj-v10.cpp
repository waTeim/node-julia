#include <stdio.h>
#include <iostream>
#include <string>
#include "Types.h"
#include "nj-vi.h"

using namespace std;
using namespace v8;

Handle<Value> returnNull(HandleScope &scope)
{
   return scope.Close(Null());
}

Handle<Value> returnBoolean(HandleScope &scope,bool b)
{
   return scope.Close(Boolean::New(b));
}

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
      case nj::null_type: returnNull(scope); break;
      case nj::boolean_type: returnBoolean(scope,primitive.toBoolean()); break;
      case nj::char_type:
      {
         Local<Value> dest = String::New(primitive.toString().c_str());

         return dest;
      }
      break;
      case nj::int64_type:
      case nj::int32_type:
      case nj::int16_type:
      {
         Local<Value> dest = Number::New(primitive.toInt());

         return dest;
      }
      break;
      case nj::uint64_type:
      case nj::uint32_type:
      case nj::uint16_type:
      case nj::uchar_type:
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
      case nj::string_type:
      {
         Local<Value> dest = String::New(primitive.toString().c_str());

         return dest;
      }
      break;
   }

   return Array::New(0);
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
      case nj::uint64_type: return buildArrayResponse<uint64_t,nj::UInt64_t>(scope,value); break;
      case nj::uint32_type: return buildArrayResponse<unsigned,nj::UInt32_t>(scope,value); break;
      case nj::uint16_type: return buildArrayResponse<unsigned short,nj::UInt16_t>(scope,value); break;
      case nj::char_type: return buildArrayResponse<char,nj::Char_t>(scope,value); break;
      case nj::uchar_type: return buildArrayResponse<unsigned char,nj::UChar_t>(scope,value); break;
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

Handle<Value> doStart(const Arguments &args)
{
   HandleScope scope;
   int numArgs = args.Length();

   if(numArgs == 0)
   {
      if(!J) J = new JuliaExecEnv();
      return returnString(scope,"Julia Started");
   }

   Local<String> arg0 = Local<String>::Cast(args[0]);
   String::AsciiValue juliaDirectory(arg0);

   if(!J) J = new JuliaExecEnv(*juliaDirectory);
   return returnString(scope,"Julia Started");
}

Handle<Value> doEval(const Arguments &args)
{
   HandleScope scope;
   int numArgs = args.Length();

   if(numArgs < 2 || !J) return returnNull(scope);

   Local<String> arg0 = Local<String>::Cast(args[0]);
   Local<Function> cb = Local<Function>::Cast(args[1]);
   String::AsciiValue text(arg0);
   JMain *engine;

   if(text.length() > 0 && (engine = J->getEngine()))
   {
      engine->evalQueuePut(*text);
      shared_ptr<nj::Result> res = engine->resultQueueGet();
  
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

   if(numArgs < 2 || !J) return returnNull(scope);

   Local<String> arg0 = Local<String>::Cast(args[0]);
   String::AsciiValue funcName(arg0);
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
      shared_ptr<nj::Result> res = engine->resultQueueGet();
 
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
   else
   {
      const unsigned argc = 1;
      Local<Value> argv[argc] = { String::New("") };

      return callback(scope,cb,argc,argv);
   }
}

void init(Handle<Object> exports)
{
  NODE_SET_METHOD(exports,"start",doStart);
  NODE_SET_METHOD(exports,"eval",doEval);
  NODE_SET_METHOD(exports,"exec",doExec);
}

NODE_MODULE(nj,init)
