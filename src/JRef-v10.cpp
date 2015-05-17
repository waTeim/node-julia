#include <iostream>
#include <node.h>
#include "nj-v10.h"
#include "JRef-v10.h"
#include "JuliaHandle.h"
#include "JuliaExecEnv.h"
#include "JMain.h"

using namespace std;
using namespace v8;

Persistent<Function> nj::JRef::constructor;
map<int64_t,nj::JRef*> nj::JRef::obj_chain;

Handle<Value> nj::JRef::getProperty(Local<String> property,const AccessorInfo &info)
{
   HandleScope scope;

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
            else return mapResult(scope,res);
         }
      }
   }
   return Undefined();
}

Handle<Value> nj::JRef::New(const Arguments& args)
{
   HandleScope scope;

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
               JRef *source = ObjectWrap::Unwrap<JRef>(arg0);
              
               hIndex = source->h_index;
            }
         }
         else if(args[0]->IsNumber()) hIndex = args[0]->IntegerValue();
      }

      JRef *unwrapped = new JRef(hIndex);
      vector<string> properties = unwrapped->handle->properties();

      for(string property: properties)
      {
         args.This()->SetAccessor(String::NewSymbol(property.c_str()),&getProperty);
      }

      unwrapped->Wrap(args.This());
      return args.This();
   }
   else
   {
      const int argc = 1;
      Local<v8::Value> argv[argc] = { args[0] };

      return scope.Close(constructor->NewInstance(argc,argv));
   }
}

Handle<Value> nj::JRef::getHIndex(const Arguments &args)
{
   HandleScope scope;
   JRef *obj = ObjectWrap::Unwrap<JRef>(args.This());

   return scope.Close(Number::New((int)obj->h_index));
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
   Local<FunctionTemplate> T = FunctionTemplate::New(New);

   T->SetClassName(String::NewSymbol("JRef"));
   T->InstanceTemplate()->SetInternalFieldCount(1);
   T->PrototypeTemplate()->Set(String::NewSymbol("getHIndex"),FunctionTemplate::New(getHIndex)->GetFunction());
   constructor = Persistent<Function>::New(T->GetFunction());
   exports->Set(String::NewSymbol("JRef"),constructor);
}

Handle<Value> nj::JRef::NewInstance(const Arguments& args)
{

   HandleScope scope;
   const unsigned argc = 1;
   Handle<v8::Value> argv[argc] = { args[0] };
   Local<Object> instance = constructor->NewInstance(argc,argv);

   if(instance.IsEmpty() || instance->IsUndefined()) return scope.Close(Undefined());
   return scope.Close(instance);
}
