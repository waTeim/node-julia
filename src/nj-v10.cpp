#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <node_buffer.h>
#include "Types.h"
#include "request.h"
#include "JuliaHandle.h"
#include "ScriptEncapsulated-v10.h"
#include "JRef-v10.h"
#include "JMain.h"
#include "NativeArray.h"
#include "Callback.h"
#include "nj-v10.h"
#include "dispatch.h"
#include "util.h"

using namespace std;
using namespace v8;

extern constexpr char Float64Array[] = "Float64Array";
extern constexpr char Float32Array[] = "Float32Array";
extern constexpr char Int32Array[] = "Int32Array";
extern constexpr char Int16Array[] = "Int16Array";
extern constexpr char Int8Array[] = "Int8Array";
extern constexpr char Uint32Array[] = "Uint32Array";
extern constexpr char Uint16Array[] = "Uint16Array";
extern constexpr char Uint8Array[] = "Uint8Array";

Handle<Value> callback(HandleScope &scope,const Local<Function> &cb,int argc,Local<Value> *argv)
{
   cb->Call(Context::GetCurrent()->Global(),argc,argv);
   return scope.Close(Undefined());
}

Local<Value> createPrimitiveRes(HandleScope &scope,const nj::Primitive &primitive)
{

   switch(primitive.type()->id())
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
      case nj::date_type:
      {
         Local<Value> dest = Date::New(primitive.toFloat());

         return dest;
      }
      break;
      case nj::regex_type:
      {
         Local<String> pattern = String::New(primitive.toString().c_str());
         Local<Value> dest = RegExp::New(pattern,RegExp::kNone);

         return dest;
      }
      break;
   }

   return Local<Value>::New(Null());
}

template <typename V> Local<Value> getNumberFromValue(const V &val)
{
   return Number::New(val);
}

Local<Value> getStringFromValue(const string &val)
{
   return String::New(val.c_str());
}

Local<Value> getDateFromValue(const double &val)
{
   return Date::New(val);
}

Local<Value> getRegexFromValue(const string &val)
{
   Local<String> pattern = String::New(val.c_str());

   return RegExp::New(pattern,RegExp::kNone);
}

Local<Value> getNullValue(const unsigned char &val)
{
   return Local<Value>::New(Null());
}

template<typename V,Local<Value> getElement(const V &val)> static void connectSubArrays(const vector<size_t> &dims,const vector<size_t> &strides,size_t ixNum,size_t offset,const Local<Array> &to,V *from)
{
   size_t numElements = dims[ixNum];
   size_t stride = strides[ixNum];

   if(ixNum == dims.size() - 1)
   {
      for(size_t elementNum = 0;elementNum < numElements;elementNum++)
      {
         to->Set(elementNum,getElement(*(from + offset)));
         offset += stride;
      }
   }
   else
   {
      for(size_t elementNum = 0;elementNum < numElements;elementNum++)
      {
         Local<Array> subArray = Array::New(dims[ixNum + 1]);

         to->Set(elementNum,subArray);
         connectSubArrays<V,getElement>(dims,strides,ixNum + 1,offset,subArray,from);
         offset += stride;
      }
   }
}

template<typename V,typename E,Local<Value> getElement(const V &val)> Local<Array> createArrayRes(HandleScope &scope,const shared_ptr<nj::Value> &value)
{
   const nj::Array<V,E> &array = static_cast<const nj::Array<V,E>&>(*value);

   if(array.size() == 0) return Array::New(0);
   if(array.dims().size() == 1)
   {
      size_t size0 = array.dims()[0];
      V *p = array.ptr();
      Local<Array> dest = Array::New(size0);

      for(size_t i = 0;i < size0;i++) dest->Set(i,getElement(p[i]));
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
         for(size_t j = 0;j < size1;j++) row->Set(j,getElement(p[size0*j + i]));
      }
      return dest;
   }
   else
   {
      vector<size_t> strides;
      size_t size0 = array.dims()[0];
      Local<Array> dest = Array::New(size0);

      strides.push_back(1);
      for(size_t idxNum = 1;idxNum < array.dims().size();idxNum++) strides.push_back(array.dims()[idxNum - 1]*strides[idxNum - 1]);
      connectSubArrays<V,getElement>(array.dims(),strides,0,0,dest,array.ptr());
      return dest;
   }
   return Array::New(0);
}

template<typename V,char const *N,typename Nv> static void connectSubArrays(const vector<size_t> &dims,const vector<size_t> &strides,size_t ixNum,size_t offset,const Local<Value> &to,V *from)
{
   size_t numElements = dims[ixNum];
   size_t stride = strides[ixNum];

   if(ixNum == dims.size() - 1)
   {
      nj::NativeArray<Nv> arr(Local<Object>::Cast(to));
      Nv *dptr = arr.dptr();

      for(size_t elementNum = 0;elementNum < numElements;elementNum++)
      {
         *dptr++ = from[offset];
         offset += stride;
      }
   }
   else
   {
      Local<Array> arr = Local<Array>::Cast(to);

      for(size_t elementNum = 0;elementNum < numElements;elementNum++)
      {
         Local<Value> subArray;

         if(ixNum == dims.size() - 2)
         {
            Local<String> constructorName = String::New(N);
            Handle<Value> constructor_val = Context::GetCurrent()->Global()->Get(constructorName);
            Handle<Function> constructor = Handle<Function>::Cast(constructor_val);
            const unsigned argc = 1;
            Local<Value> argv[argc] = { Uint32::New(dims[ixNum + 1]) };

            subArray = constructor->NewInstance(argc,argv);
         }
         else subArray = Array::New(dims[ixNum + 1]);

         arr->Set(elementNum,subArray);
         connectSubArrays<V,N,Nv>(dims,strides,ixNum + 1,offset,subArray,from);
         offset += stride;
      }
   }
}

template<typename V,typename E,char const *N,typename Nv> Local<Value> createArrayRes(HandleScope &scope,const shared_ptr<nj::Value> &value)
{
   const nj::Array<V,E> &array = static_cast<const nj::Array<V,E>&>(*value);

   if(array.size() == 0) return Array::New(0);
   if(array.dims().size() == 1)
   {
      size_t size0 = array.dims()[0];
      Local<String> constructorName = String::New(N);
      Handle<Value> constructor_val = Context::GetCurrent()->Global()->Get(constructorName);
      Handle<Function> constructor = Handle<Function>::Cast(constructor_val);
      const unsigned argc = 1;
      Local<Value> argv[argc] = { Uint32::New(size0) };
      Local<Object> dest = constructor->NewInstance(argc,argv);
      nj::NativeArray<Nv> arr(dest);
      V *p = array.ptr();
      Nv *dptr = arr.dptr();

      for(size_t i = 0;i < size0;i++) *dptr++ = *p++;
      return dest;
   }
   else
   {
      vector<size_t> strides;
      size_t size0 = array.dims()[0];
      Local<Array> dest = Array::New(size0);

      strides.push_back(1);
      for(size_t idxNum = 1;idxNum < array.dims().size();idxNum++) strides.push_back(array.dims()[idxNum - 1]*strides[idxNum - 1]);
      connectSubArrays<V,N,Nv>(array.dims(),strides,0,0,dest,array.ptr());

      return dest;
   }
   return Array::New(0);
}

Local<Value> createArrayRes(HandleScope &scope,const shared_ptr<nj::Value> &value,const nj::Type *elementType)
{
   switch(elementType->id())
   {
      case nj::null_type: return createArrayRes<unsigned char,nj::Null_t,getNullValue>(scope,value); break;
      case nj::float64_type: return createArrayRes<double,nj::Float64_t,Float64Array,double>(scope,value); break;
      case nj::float32_type: return createArrayRes<float,nj::Float32_t,Float32Array,float>(scope,value); break;
      case nj::int64_type: return createArrayRes<int64_t,nj::Int64_t,Float64Array,double>(scope,value); break;
      case nj::int32_type: return createArrayRes<int,nj::Int32_t,Int32Array,int>(scope,value); break;
      case nj::int16_type: return createArrayRes<short,nj::Int16_t,Int16Array,short>(scope,value); break;
      case nj::int8_type: return createArrayRes<char,nj::Int8_t,Int8Array,char>(scope,value); break;
      case nj::uint64_type: return createArrayRes<uint64_t,nj::UInt64_t,Float64Array,double>(scope,value); break;
      case nj::uint32_type: return createArrayRes<unsigned,nj::UInt32_t,Uint32Array,unsigned>(scope,value); break;
      case nj::uint16_type: return createArrayRes<unsigned short,nj::UInt16_t,Uint16Array,unsigned short>(scope,value); break;
      case nj::uint8_type: return createArrayRes<unsigned char,nj::UInt8_t,Uint8Array,unsigned char>(scope,value); break;
      case nj::ascii_string_type: return createArrayRes<string,nj::ASCIIString_t,getStringFromValue>(scope,value); break;
      case nj::utf8_string_type: return createArrayRes<string,nj::UTF8String_t,getStringFromValue>(scope,value); break;
      case nj::date_type: return createArrayRes<double,nj::Date_t,getDateFromValue>(scope,value); break;
      case nj::regex_type: return createArrayRes<string,nj::Regex_t,getRegexFromValue>(scope,value); break;
   }

   return Array::New(0);
}

Local<Object> createBufferRes(HandleScope &scope,const shared_ptr<nj::Value> &value)
{
   const nj::Array<unsigned char,nj::UInt8_t> &array = static_cast<const nj::Array<unsigned char,nj::UInt8_t>&>(*value);

   size_t length = array.dims()[0];
   unsigned char *p = array.ptr();
   node::Buffer *buffer = node::Buffer::New(length);

   memcpy(node::Buffer::Data(buffer),p,length);

   Local<Object> globalObj = Context::GetCurrent()->Global();
   Local<Function> cons = Local<Function>::Cast(globalObj->Get(String::New("Buffer")));
   Handle<Value> cArgs[3] = { buffer->handle_, v8::Integer::New(length), v8::Integer::New(0) };

   return cons->NewInstance(3,cArgs);
}

int createResponse(HandleScope &scope,const shared_ptr<nj::Result> &res,int argc,Local<Value> *argv)
{
   int index = argc - res->results().size();

   for(shared_ptr<nj::Value> value: res->results())
   {
      if(value.get())
      {
         if(value->type()->id() == nj::julia_handle_type)
         {
            nj::JuliaHandle *handle = static_cast<nj::JuliaHandle*>(value.get());
            int64_t hIndex = handle->intern();
            Local<Value> arguments[1] = { Number::New(hIndex) };

            argv[index++] = nj::JRef::constructor->Call(Context::GetCurrent()->Global(),1,arguments);
         }
         else if(value->isPrimitive())
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

Local<String> genError(HandleScope &scope,const shared_ptr<nj::Result> &res)
{
   return String::New(res->exceptionText().c_str());
}

void raiseException(HandleScope &scope,const shared_ptr<nj::Result> &res)
{
   int exceptionId = res->exceptionId();
   Local<Value> ex;

   switch(exceptionId)
   {
      case nj::Exception::julia_undef_var_error_exception:
      case nj::Exception::julia_method_error_exception:
         ex = Exception::ReferenceError(String::New(res->exceptionText().c_str()));
      break;
      default:
         ex = Exception::Error(String::New(res->exceptionText().c_str()));
      break;
   }

   Local<Object> obj = Local<Object>::Cast(ex);
   Local<String> stackElementKey = String::New("stack");
   Local<Value> stack_v = obj->Get(stackElementKey);
   String::Utf8Value stackText(stack_v);
   vector<string> lines = nj::split(string(*stackText),'\n');
   string stack_s = lines[0];

   for(string line: res->exceptionStack()) stack_s += "\n    at " + line;
   for(size_t i = 1;i < lines.size();i++) stack_s += "\n" + lines[i];

   Local<String> message = String::New(stack_s.c_str());

   obj->Set(v8::String::New("stack"),message);
   ThrowException(ex);
}

Handle<Value> callbackWithResult(HandleScope &scope,const Local<Function> &cb,const shared_ptr<nj::Result> &res)
{
   if(res.get())
   {
      int exceptionId = res->exceptionId();

      if(exceptionId != nj::Exception::no_exception)
      {
         int argc = 1;
         Local<Value> *argv = new Local<Value>[argc];

         argv[0] = genError(scope,res);
         return callback(scope,cb,argc,argv);
      }
      else
      {
         int argc = res->results().size() + 1;
         Local<Value> *argv = new Local<Value>[argc];

         argv[0] = Local<Value>::New(Null());
         argc = createResponse(scope,res,argc,argv);
         return callback(scope,cb,argc,argv);
      }
   }
   else return callback(scope,cb,0,0);
}

Local<Value> mapResult(HandleScope &scope,shared_ptr<nj::Result> &res)
{
   int argc = res->results().size();
   Local<Value> *argv = new Local<Value>[argc];

   argc = createResponse(scope,res,argc,argv);

   if(argc != 0)
   {
      if(argc > 1)
      {
         Local<Array> rV = Array::New(argc);

         for(int i = 0;i < argc;i++) rV->Set(i,argv[i]);

         return rV;
      }
      else return argv[0];
   }
   else return Local<Value>();
}

Handle<Value> returnResult(HandleScope &scope,shared_ptr<nj::Result> &res)
{
   if(res.get())
   {
      int exceptionId = res->exceptionId();

      if(exceptionId != nj::Exception::no_exception) raiseException(scope,res);
      else return scope.Close(mapResult(scope,res));
   }
   return scope.Close(Undefined());
}

Handle<Value> doEval(const Arguments &args)
{
   HandleScope scope;
   int numArgs = args.Length();

   if(numArgs  == 0 || numArgs > 2 || (numArgs == 2 && !args[1]->IsFunction())) return scope.Close(Null());

   JuliaExecEnv *J = JuliaExecEnv::getSingleton();
   Local<String> arg0 = Local<String>::Cast(args[0]);
   Local<Function> cb;
   String::Utf8Value text(arg0);
   JMain *engine;
   bool useCallback = false;

   if(numArgs >= 2 && args[args.Length() - 1]->IsFunction())
   {
      useCallback = true;
      cb = Local<Function>::Cast(args[args.Length() - 1]);
   }

   if(text.length() > 0 && (engine = J->getEngine()))
   {
      if(!useCallback)
      {
         engine->eval(*text);

         shared_ptr<nj::Result> res = engine->syncQueueGet();

         return returnResult(scope,res);
      }
      else
      {
         nj::Callback *c = new nj::Callback(cb);

         engine->eval(*text,c);
         return scope.Close(Undefined());
      }
   }
   else
   {
      if(useCallback)
      {
         const unsigned argc = 1;
         Local<Value> argv[argc] = { String::New("missing eval string") };

         return callback(scope,cb,argc,argv);
      }
      else return scope.Close(Null());
   }
}

Handle<Value> doExec(const Arguments &args)
{
   HandleScope scope;
   int numArgs = args.Length();
   int funcNameIndex = 0;
   bool useCallback = false;
   shared_ptr<nj::Value> module;

   if(numArgs == 0) return scope.Close(Null());

   if(args[0]->IsObject())
   {
      Local<Object> obj = args[0]->ToObject();
      String::Utf8Value utf(obj->GetConstructorName());
      string cname(*utf);

      if(cname == "JRef")
      {
         nj::JRef *src = node::ObjectWrap::Unwrap<nj::JRef>(obj);

         funcNameIndex = 1;
         module = dynamic_pointer_cast<nj::Value>(src->getHandle());
      }
      else return scope.Close(Null());
   }

   JuliaExecEnv *J = JuliaExecEnv::getSingleton();
   Local<String> text = Local<String>::Cast(args[funcNameIndex]);
   String::Utf8Value funcName(text);
   Local<Function> cb;
   JMain *engine;

   if(numArgs >= (funcNameIndex + 2) && args[args.Length() - 1]->IsFunction())
   {
      useCallback = true;
      cb = Local<Function>::Cast(args[args.Length() - 1]);
   }

   if(funcName.length() > 0 && (engine = J->getEngine()))
   {
      vector<shared_ptr<nj::Value>> req;
      int numExecArgs = numArgs - (funcNameIndex + 1);

      try
      {
         for(int i = 0;i < numExecArgs;i++)
         {
            shared_ptr<nj::Value> reqElement = createRequest(args[i + funcNameIndex + 1]);

            if(reqElement.get()) req.push_back(reqElement);
         }

         if(!useCallback)
         {
            if(funcNameIndex == 0) engine->exec(*funcName,req);
            else engine->exec(module,*funcName,req);

            shared_ptr<nj::Result> res = engine->syncQueueGet();

            return returnResult(scope,res);
         }
         else
         {
            nj::Callback *c = new nj::Callback(cb);

            if(funcNameIndex == 0) engine->exec(*funcName,req,c);
            else engine->exec(module,*funcName,req,c);
            return scope.Close(Undefined());
         }
      }
      catch(nj::InvalidException e)
      {
         if(!useCallback)
         {
            ThrowException(Exception::Error(String::New(e.what().c_str())));
            return scope.Close(Undefined());
         }
         else
         {
            const unsigned argc = 1;
            Local<Value> argv[argc] = { String::New(e.what().c_str()) };

            return callback(scope,cb,argc,argv);
         }
      }
   }
   else
   {
      if(useCallback)
      {
         const unsigned argc = 1;
         Local<Value> argv[argc] = { String::New("could not execute") };

         return callback(scope,cb,argc,argv);
      }
      else return scope.Close(Null());
   }
}

Handle<Value> doImport(const Arguments &args)
{
   HandleScope scope;
   int numArgs = args.Length();

   if(numArgs  == 0 || numArgs > 2 || (numArgs == 2 && !args[1]->IsFunction())) return scope.Close(Null());

   JuliaExecEnv *J = JuliaExecEnv::getSingleton();
   Local<String> arg0 = Local<String>::Cast(args[0]);
   Local<Function> cb;
   String::Utf8Value text(arg0);
   JMain *engine;
   bool useCallback = false;

   if(numArgs >= 2 && args[args.Length() - 1]->IsFunction())
   {
      useCallback = true;
      cb = Local<Function>::Cast(args[args.Length() - 1]);
   }

   if(text.length() > 0 && (engine = J->getEngine()))
   {
      if(!useCallback)
      {
         engine->import(*text);

         shared_ptr<nj::Result> res = engine->syncQueueGet();

         return returnResult(scope,res);
      }
      else
      {
         nj::Callback *c = new nj::Callback(cb);

         engine->import(*text,c);
         return scope.Close(Undefined());
      }
   }
   else
   {
      if(useCallback)
      {
         const unsigned argc = 1;
         Local<Value> argv[argc] = { String::New("missing module name") };

         return callback(scope,cb,argc,argv);
      }
      else return scope.Close(Null());
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
   nj::JRef::Init(exports);
   NODE_SET_METHOD(exports,"eval",doEval);
   NODE_SET_METHOD(exports,"exec",doExec);
   NODE_SET_METHOD(exports,"import",doImport);
   NODE_SET_METHOD(exports,"newScript",newScript);
   nj::dispatch_init();
}

NODE_MODULE(nj,init)
