#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <node_buffer.h>
#include "Types.h"
#include "request.h"
#include "JuliaHandle.h"
#include "ScriptEncapsulated-v11.h"
#include "JRef-v11.h"
#include "JMain.h"
#include "NativeArray.h"
#include "Callback.h"
#include "NAlloc.h"
#include "JSAlloc.h"
#include "VAlloc.h"
#include "nj-v11.h"
#include "dispatch.h"
#include "util.h"

using namespace std;
using namespace v8;

char Float64ArrayName[] = "Float64Array";

void returnNull(Isolate *I,const FunctionCallbackInfo<Value> &args)
{
   args.GetReturnValue().SetNull();
}

void callback(Isolate *I,const Local<Function> &cb,int argc,Local<Value> *argv)
{
   cb->Call(I->GetCurrentContext()->Global(),argc,argv);
}

Local<Value> createPrimitiveRes(HandleScope &scope,const nj::Primitive &primitive)
{
   Isolate *I = Isolate::GetCurrent();

   switch(primitive.type()->id())
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
         Local<Value> dest = Number::New(I,(double)primitive.toInt());

         return dest;
      }
      break;
      case nj::uint64_type:
      case nj::uint32_type:
      case nj::uint16_type:
      case nj::uint8_type:
      {
         Local<Value> dest = Number::New(I,(double)primitive.toUInt());

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
      case nj::regex_type:
      {
         Local<String> pattern = String::NewFromUtf8(I,primitive.toString().c_str());
         Local<Value> dest = RegExp::New(pattern,RegExp::kNone);

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

Local<Value> getDateFromValue(Isolate *I,const double &val)
{
   return Date::New(I,val);
}

Local<Value> getRegexFromValue(Isolate *I,const string &val)
{
   Local<String> pattern = String::NewFromUtf8(I,val.c_str());

   return RegExp::New(pattern,RegExp::kNone);
}

Local<Value> getNullValue(Isolate *I,const unsigned char &val)
{
   return Null(I);
}

template<typename V,Local<Value> getElement(Isolate *I,const V &val)> static void connectSubArrays(const vector<size_t> &dims,const vector<size_t> &strides,size_t ixNum,size_t offset,const Local<Array> &to,V *from)
{
   Isolate *I = Isolate::GetCurrent();
   size_t numElements = dims[ixNum];
   size_t stride = strides[ixNum];

   if(ixNum == dims.size() - 1)
   {
      for(size_t elementNum = 0;elementNum < numElements;elementNum++)
      {
         to->Set((int)elementNum,getElement(I,*(from + offset)));
         offset += stride;
      }
   }
   else
   {
      for(size_t elementNum = 0;elementNum < numElements;elementNum++)
      {
         Local<Array> subArray = Array::New(I,(int)dims[ixNum + 1]);

         to->Set((int)elementNum,subArray);
         connectSubArrays<V,getElement>(dims,strides,ixNum + 1,offset,subArray,from);
         offset += stride;
      }
   }
}

template<typename V,typename E,Local<Value> getElement(Isolate *I,const V &val)> Local<Array> createArrayRes(HandleScope &scope,const shared_ptr<nj::Value> &value)
{
   Isolate *I = Isolate::GetCurrent();
   const nj::Array<V,E> &array = static_cast<const nj::Array<V,E>&>(*value);

   if(array.size() == 0) return Array::New(I,0);
   if(array.dims().size() == 1)
   {

      // #TODO do more than simply cast to int
      size_t size0 = array.dims()[0];
      V *p = array.ptr();
      Local<Array> dest = Array::New(I,(int)size0);

      for(size_t i = 0;i < size0;i++) dest->Set((int)i,getElement(I,p[i]));
      return dest;
   }
   else if(array.dims().size() == 2)
   {
      // #TODO do more than simply cast to int
      size_t size0 = array.dims()[0];
      size_t size1 = array.dims()[1];
      V *p = array.ptr();
      Local<Array> dest = Array::New(I,(int)size0);

      for(size_t i = 0;i < size0;i++)
      {
         Local<Array> row  = Array::New(I,(int)size1);

         dest->Set((int)i,row);
         for(size_t j = 0;j < size1;j++) row->Set((int)j,getElement(I,p[size0*j + i]));
      }
      return dest;
   }
   else
   {
      // #TODO do more than simply cast to int
      vector<size_t> strides;
      size_t size0 = array.dims()[0];
      Local<Array> dest = Array::New(I,(int)size0);

      strides.push_back(1);
      for(size_t idxNum = 1;idxNum < array.dims().size();idxNum++) strides.push_back(array.dims()[idxNum - 1]*strides[idxNum - 1]);
      connectSubArrays<V,getElement>(array.dims(),strides,0,0,dest,array.ptr());
      return dest;
   }
   return Array::New(I,0);
}

template<typename V,typename N,typename Nv> static void connectSubArrays(const vector<size_t> &dims,const vector<size_t> &strides,size_t ixNum,size_t offset,const Local<Value> &to,V *from)
{
   Isolate *I = Isolate::GetCurrent();
   size_t numElements = dims[ixNum];
   size_t stride = strides[ixNum];

   if(ixNum == dims.size() - 1)
   {
      nj::NativeArray<Nv> arr(Local<Object>::Cast(to));
      Nv *dptr = arr.dptr();

      for(size_t elementNum = 0;elementNum < numElements;elementNum++)
      {
         *dptr++ = Nv(from[offset]);
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
            Local<ArrayBuffer> buffer = ArrayBuffer::New(I,dims[ixNum + 1]*sizeof(Nv));

            subArray = N::New(buffer,0,dims[ixNum + 1]);
         }
         else subArray = Array::New(I,(int)dims[ixNum + 1]);

         arr->Set((int)elementNum,subArray);
         connectSubArrays<V,N,Nv>(dims,strides,ixNum + 1,offset,subArray,from);
         offset += stride;
      }
   }
}

template<typename V,typename E,typename N,typename Nv> Local<Value> createArrayRes(HandleScope &scope,const shared_ptr<nj::Value> &value)
{
   Isolate *I = Isolate::GetCurrent();
   nj::Array<V,E> &array = static_cast<nj::Array<V,E>&>(*value);

   if(array.size() == 0) return Array::New(I,0);
   if(array.dims().size() == 1)
   {
      size_t size0 = array.dims()[0];
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
            shared_ptr<nj::Alloc> v = array.v();

            if(v->container())
            {
               nj::JSAlloc *L = static_cast<nj::JSAlloc*>(v->container()->loc(1).get());

               return L->obj();
            }
            else
            {
               Local<ArrayBuffer> buffer = ArrayBuffer::New(I,array.ptr(),size0*sizeof(Nv));
               Local<N> dest = N::New(buffer,0,size0);
               shared_ptr<nj::Alloc> l_p = nj::NAlloc::create(array.v());
               nj::NAlloc *l = (nj::NAlloc*)l_p.get();

               l->add(nj::JSAlloc::create(dest));
               return dest;
            }
         }
         break;
         default:
         {
            Local<ArrayBuffer> buffer = ArrayBuffer::New(I,size0*sizeof(Nv));
            Local<N> dest = N::New(buffer,0,size0);
            nj::NativeArray<Nv> arr(dest);
            V *p = array.ptr();
            Nv *dptr = arr.dptr();

            for(size_t i = 0;i < size0;i++) *dptr++ = Nv(*p++);
            return dest;
         }
         break;
      }
   }
   else
   {
      // #TODO do more than simply cast to int
      vector<size_t> strides;
      size_t size0 = array.dims()[0];
      Local<Array> dest = Array::New(I,(int)size0);

      strides.push_back(1);
      for(size_t idxNum = 1;idxNum < array.dims().size();idxNum++) strides.push_back(array.dims()[idxNum - 1]*strides[idxNum - 1]);
      connectSubArrays<V,N,Nv>(array.dims(),strides,0,0,dest,array.ptr());

      return dest;
   }
   return Array::New(I,0);
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

   Isolate *I = Isolate::GetCurrent();

   return Array::New(I,0);
}

Local<Value> createBufferRes(HandleScope &scope,const shared_ptr<nj::Value> &value)
{
   const nj::Array<unsigned char,nj::UInt8_t> &array = static_cast<const nj::Array<unsigned char,nj::UInt8_t>&>(*value);
   Isolate *I = Isolate::GetCurrent();
   size_t length = array.dims()[0];
   unsigned char *p = array.ptr();

#if V8MAJOR < 4 || V8MAJOR == 4 && V8MINOR < 4
   Local<Object> buffer = node::Buffer::New(I,length);

   memcpy(node::Buffer::Data(buffer),p,length);
   return buffer;
#else
   MaybeLocal<Object> buffer = node::Buffer::New(I,length);
   Local<Value> localized = buffer.FromMaybe<Value>(v8::Undefined(I));

   if(!buffer.IsEmpty()) memcpy(node::Buffer::Data(localized),p,length);
   return localized;
#endif
}

Local<Value> createResponse(HandleScope &scope,const shared_ptr<nj::Value> &value)
{
   Isolate *I = Isolate::GetCurrent();

   if(value->type()->id() == nj::julia_handle_type)
   {
      nj::JuliaHandle *handle = static_cast<nj::JuliaHandle*>(value.get());
      int64_t hIndex = handle->intern();
      Handle<Value> arguments[1] = { Number::New(I,(int)hIndex) };
      Local<Function> cons = Local<Function>::New(I,nj::JRef::constructor);

      return cons->NewInstance(1,arguments);
   }
   else if(value->isPrimitive())
   {
      const nj::Primitive &primitive = static_cast<const nj::Primitive&>(*value);

      return createPrimitiveRes(scope,primitive);
   }
   else
   {
      const nj::Array_t *array_type = static_cast<const nj::Array_t*>(value->type());
      const nj::Type *etype = array_type->etype();

      if(etype == nj::UInt8_t::instance()  && value->dims().size() == 1) return createBufferRes(scope,value);
      else return createArrayRes(scope,value,etype);
   }
}

int createResponse(HandleScope &scope,const shared_ptr<nj::Result> &res,int argc,Local<Value> *argv)
{
   int index = argc - (int)res->results().size();

   for(shared_ptr<nj::Value> value: res->results())
   {
      if(value.get()) argv[index++] = createResponse(scope,value);
   }
   return index;
}

Local<String> genError(HandleScope &scope,const shared_ptr<nj::Result> &res)
{
   Isolate *I = Isolate::GetCurrent();

   return String::NewFromUtf8(I,res->exceptionText().c_str());
}

void raiseException(HandleScope &scope,const shared_ptr<nj::Result> &res)
{
   Isolate *I = Isolate::GetCurrent();
   int exceptionId = res->exceptionId();
   Local<Value> ex;

   switch(exceptionId)
   {
      case nj::Exception::julia_undef_var_error_exception:
      case nj::Exception::julia_method_error_exception:
         ex = Exception::ReferenceError(String::NewFromUtf8(I,res->exceptionText().c_str()));
      break;
      default:
         ex = Exception::Error(String::NewFromUtf8(I,res->exceptionText().c_str()));
      break;
   }

   Local<Object> obj = Local<Object>::Cast(ex);
   Local<String> stackElementKey = String::NewFromUtf8(I,"stack");
   Local<Value> stack_v = obj->Get(stackElementKey);
   String::Utf8Value stackText(stack_v);
   vector<string> lines = nj::split(string(*stackText),'\n');
   string stack_s = lines[0];

   for(string line: res->exceptionStack()) stack_s += "\n    at " + line;
   for(size_t i = 1;i < lines.size();i++) stack_s += "\n" + lines[i];

   Local<String> message = String::NewFromUtf8(I,stack_s.c_str());

   obj->Set(v8::String::NewFromUtf8(I,"stack"),message);
   I->ThrowException(ex);
}

void callbackWithResult(HandleScope &scope,const Local<Function> &cb,const shared_ptr<nj::Result> &res,const Handle<Object> &recv)
{
   Isolate *I = Isolate::GetCurrent();

   if(res.get())
   {
      int exceptionId = res->exceptionId();

      if(exceptionId != nj::Exception::no_exception)
      {
         int argc = 1;
         Local<Value> *argv = new Local<Value>[argc];

         argv[0] = genError(scope,res);
         if(!recv->IsUndefined()) node::MakeCallback(I,recv,cb,argc,argv);
         else callback(I,cb,argc,argv);
      }
      else
      {
         int argc = (int)res->results().size() + 1;
         Local<Value> *argv = new Local<Value>[argc];

         argv[0] = Null(I);
         (void)createResponse(scope,res,argc,argv);
         if(!recv->IsUndefined()) node::MakeCallback(I,recv,cb,argc,argv);
         else callback(I,cb,argc,argv);
      }
   }
   else callback(I,cb,0,0);
}

Local<Value> mapResult(HandleScope &scope,shared_ptr<nj::Result> &res)
{
   Isolate *I = Isolate::GetCurrent();
   int argc = (int)res->results().size();
   Local<Value> *argv = new Local<Value>[argc];

   argc = createResponse(scope,res,argc,argv);

   if(argc != 0)
   {
      if(argc > 1)
      {
         Local<Array> rV = Array::New(I,argc);

         for(int i = 0;i < argc;i++) rV->Set(i,argv[i]);

         return rV;
      }
      else return argv[0];
   }
   else return Local<Value>();
}

void returnResult(const FunctionCallbackInfo<Value> &args,HandleScope &scope,shared_ptr<nj::Result> &res)
{
   if(res.get())
   {
      int exceptionId = res->exceptionId();

      if(exceptionId != nj::Exception::no_exception) raiseException(scope,res);
      else
      {
         args.GetReturnValue().Set(mapResult(scope,res));
         return;
      }
   }
   args.GetReturnValue().SetUndefined();
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

   JuliaExecEnv *J = JuliaExecEnv::getSingleton();
   Local<String> arg0 = args[0]->ToString();
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

         returnResult(args,scope,res);
      }
      else
      {
         nj::Callback *c = new nj::Callback(cb,args.This());

         engine->eval(*text,c);
      }
   }
   else
   {
      if(useCallback)
      {
         const unsigned argc = 1;
         Local<Value> argv[argc] = { String::NewFromUtf8(I,"missing eval string") };

         callback(I,cb,argc,argv);
      }
      else returnNull(I,args);
   }
}

void doExec(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   int numArgs = args.Length();
   int funcNameIndex = 0;
   bool useCallback = false;
   shared_ptr<nj::Value> module;

   if(numArgs == 0)
   {
      returnNull(I,args);
      return;
   }

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
      else
      {
         returnNull(I,args);
         return;
      }
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

            returnResult(args,scope,res);
         }
         else
         {
            nj::Callback *c = new nj::Callback(cb,args.This());

            if(funcNameIndex == 0) engine->exec(*funcName,req,c);
            else engine->exec(module,*funcName,req,c);
         }
      }
      catch(nj::InvalidException e)
      {
         if(!useCallback) I->ThrowException(Exception::Error(String::NewFromUtf8(I,e.what().c_str())));
         else
         {
            const unsigned argc = 1;
            Local<Value> argv[argc] = { String::NewFromUtf8(I,e.what().c_str()) };

            callback(I,cb,argc,argv);
         }
      }
   }
   else
   {
      if(useCallback)
      {
         const unsigned argc = 1;
         Local<Value> argv[argc] = { String::NewFromUtf8(I,"could not execute") };

         callback(I,cb,argc,argv);
      }
      else returnNull(I,args);
   }
}

void doImport(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   int numArgs = args.Length();

   if(numArgs  == 0 || numArgs > 2 || (numArgs == 2 && !args[1]->IsFunction()))
   {
      returnNull(I,args);
      return;
   }

   JuliaExecEnv *J = JuliaExecEnv::getSingleton();
   Local<String> arg0 = args[0]->ToString();
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

         returnResult(args,scope,res);
      }
      else
      {
         nj::Callback *c = new nj::Callback(cb,args.This());

         engine->import(*text,c);
      }
   }
   else
   {
      if(useCallback)
      {
         const unsigned argc = 1;
         Local<Value> argv[argc] = { String::NewFromUtf8(I,"missing module name") };

         callback(I,cb,argc,argv);
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
  nj::JRef::Init(exports);
  NODE_SET_METHOD(exports,"eval",doEval);
  NODE_SET_METHOD(exports,"exec",doExec);
  NODE_SET_METHOD(exports,"import",doImport);
  NODE_SET_METHOD(exports,"newScript",newScript);
  nj::dispatch_init();
}

NODE_MODULE(nj,init)
