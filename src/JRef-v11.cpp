#include <iostream>
#include <node.h>
#include "nj-v11.h"
#include "JRef-v11.h"
#include "JuliaHandle.h"
#include "JuliaExecEnv.h"
#include "JMain.h"

using namespace std;
using namespace v8;

Persistent<Function> nj::JRef::constructor;
map<int64_t,nj::JRef*> nj::JRef::obj_chain;

void nj::JRef::getProperty(Local<String> property,const PropertyCallbackInfo<v8::Value> &info)
{
   Isolate *I = info.GetIsolate();
   HandleScope scope(I);

   String::Utf8Value text(property);
   JuliaExecEnv *J = JuliaExecEnv::getSingleton();
   JMain *engine;

   if(text.length() > 0 && (engine = J->getEngine()))
   {
      JRef *obj = ObjectWrap::Unwrap<JRef>(info.This());
      shared_ptr<JuliaHandle> element = obj->handle->getElement(*text);

      if(element.get())
      {
         engine->convert(element);

         shared_ptr<nj::Result> res = engine->syncQueueGet();

         if(res.get())
         {
            int exceptionId = res->exceptionId();

            if(exceptionId != nj::Exception::no_exception) raiseException(scope,res);
            else
            {
               info.GetReturnValue().Set(mapResult(scope,res));
               return;
            }
         }
         info.GetReturnValue().SetUndefined();
      }
   }
   info.GetReturnValue().SetUndefined();
}

void nj::JRef::New(const FunctionCallbackInfo<v8::Value>& args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);

   if(args.IsConstructCall())
   {
      int64_t hIndex = -1;

      if(!args[0]->IsUndefined())
      {
         if(args[0]->IsObject())
         {
            Local<Object> arg0 = args[0]->ToObject();
            String::Utf8Value utf(arg0->GetConstructorName());
            string cname(*utf);

            if(cname == "JRef") 
            {
               JRef *obj = ObjectWrap::Unwrap<JRef>(arg0);
              
               hIndex = obj->h_index;
            }
         }
         else if(args[0]->IsNumber()) hIndex = args[0]->IntegerValue();
      }

      JRef *unwrapped = new JRef(hIndex);
      vector<string> properties = unwrapped->handle->properties();

      for(string property: properties)
      {
         Local<String> propertyName = String::NewFromUtf8(I,property.c_str());

         args.This()->SetAccessor(propertyName,&getProperty);
      }

      unwrapped->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
   }
   else
   {
      const int argc = 1;
      Local<v8::Value> argv[argc] = { args[0] };
      Local<Function> cons = Local<Function>::New(I,constructor);

      args.GetReturnValue().Set(cons->NewInstance(argc,argv));
   }
}

void nj::JRef::getHIndex(const FunctionCallbackInfo<v8::Value>& args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   JRef *obj = ObjectWrap::Unwrap<JRef>(args.This());

   args.GetReturnValue().Set(Number::New(I,obj->h_index));
}

nj::JRef::JRef(int64_t hIndex)
{
   h_index = hIndex;
   next = previous = 0;
   if(obj_chain.find(h_index) != obj_chain.end())
   {
      JRef *source = obj_chain[h_index];

      source->previous = this;
      next = source;
      handle = source->handle;
   }
   else handle.reset(JuliaHandle::atIndex(h_index));
   obj_chain[h_index] = this;
}

nj::JRef::~JRef()
{
   if(obj_chain[h_index] == this) obj_chain[h_index] = next;
   if(next) next->previous = previous;
   if(previous) previous->next = next;
}

void nj::JRef::Init(Handle<Object> exports)
{
   Isolate *I = Isolate::GetCurrent();
   Local<FunctionTemplate> T = FunctionTemplate::New(I,New);

   T->SetClassName(String::NewFromUtf8(I,"JRef"));
   T->InstanceTemplate()->SetInternalFieldCount(1);

   NODE_SET_PROTOTYPE_METHOD(T,"getHIndex",getHIndex);

   constructor.Reset(I,T->GetFunction());
   exports->Set(String::NewFromUtf8(I,"JRef"),T->GetFunction());
}

void nj::JRef::NewInstance(const FunctionCallbackInfo<v8::Value>& args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   const unsigned argc = 1;
   Handle<v8::Value> argv[argc] = { args[0] };
   Local<Function> cons = Local<Function>::New(I,constructor);
   Local<Object> instance = cons->NewInstance(argc,argv);

   if(instance.IsEmpty() || instance->IsUndefined()) args.GetReturnValue().SetUndefined();
   else args.GetReturnValue().Set(instance);
}
