#include <iostream>
#include <node.h>
#include "ScriptEncapsulated-v11.h"
#include "JuliaExecEnv.h"
#include "request.h"
#include "nj-v11.h"

using namespace std;
using namespace v8;

Persistent<Function> nj::ScriptEncapsulated::constructor;

nj::ScriptEncapsulated::ScriptEncapsulated(string path):path(path)
{
   if(!J) J = new JuliaExecEnv();
   if(J)
   {
      JMain *engine = J->getEngine();

      if(engine)
      {
         engine->compileScript(path);
         compile_res = engine->resultQueueGet();

         int exId = compile_res->exId();

         if(exId == nj::Exception::no_exception)
         {
            Primitive &moduleName_r = static_cast<Primitive&>(*compile_res->results()[0]);

            module_name = moduleName_r.toString();
         }
      }
   }
}

nj::ScriptEncapsulated::~ScriptEncapsulated(){}

void nj::ScriptEncapsulated::New(const FunctionCallbackInfo<v8::Value>& args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);

   if(args.IsConstructCall())
   {
      // Invoked as constructor: `new julia.Script(...)`
      string path = "";

      if(!args[0]->IsUndefined() && args[0]->IsString())
      {
         Local<String> s = Local<String>::Cast(args[0]);
         String::Utf8Value text(s);
         path = *text;
      }

      ScriptEncapsulated *unwrapped = new ScriptEncapsulated(path);

      if(unwrapped->compile_res.get())
      {
         shared_ptr<nj::Result> cruw = unwrapped->compile_res;
         int exId = cruw->exId();

         if(exId != nj::Exception::no_exception) raiseException(args,scope,cruw);
         else
         {  
            unwrapped->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
         }
      }
      else args.GetReturnValue().SetUndefined();
   }
   else
   {
      // Invoked as plain function `julia.Script(...)`, turn into construct call.
      const int argc = 1;
      Local<v8::Value> argv[argc] = { args[0] };
      Local<Function> cons = Local<Function>::New(I,constructor);

      args.GetReturnValue().Set(cons->NewInstance(argc,argv));
   }
}

void nj::ScriptEncapsulated::getPath(const FunctionCallbackInfo<v8::Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   ScriptEncapsulated *obj = ObjectWrap::Unwrap<ScriptEncapsulated>(args.Holder());

   args.GetReturnValue().Set(String::NewFromUtf8(I,obj->path.c_str()));
}

void nj::ScriptEncapsulated::getModuleName(const FunctionCallbackInfo<v8::Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   ScriptEncapsulated *obj = ObjectWrap::Unwrap<ScriptEncapsulated>(args.Holder());

   args.GetReturnValue().Set(String::NewFromUtf8(I,obj->module_name.c_str()));
}

void nj::ScriptEncapsulated::exec(const FunctionCallbackInfo<v8::Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   ScriptEncapsulated *obj = ObjectWrap::Unwrap<ScriptEncapsulated>(args.Holder());
   JMain *engine = J->getEngine();

   if(engine)
   {
      vector<shared_ptr<nj::Value>> req;
      string funcName = "_";
      Local<Function> cb;
      bool useCallback = false;
      int numArgs = args.Length();

      if(numArgs != 0 && args[numArgs - 1]->IsFunction())
      {
         useCallback = true;
         cb = Local<Function>::Cast(args[args.Length() - 1]);
         numArgs--;
      }

      for(int i = 0;i < numArgs;i++)
      {
         shared_ptr<nj::Value> reqElement = createRequest(args[i]);

         if(reqElement.get()) req.push_back(reqElement);
      }
      engine->exec(obj->compile_res->results()[1],funcName,req);
      shared_ptr<nj::Result> res = engine->resultQueueGet();

      if(useCallback) callbackWithResult(args,scope,cb,res);
      else returnResult(args,scope,res);
   }
   else args.GetReturnValue().SetUndefined();
}

void nj::ScriptEncapsulated::Init(Handle<Object> exports)
{
   Isolate *I = Isolate::GetCurrent();
   Local<FunctionTemplate> T = FunctionTemplate::New(I,New);

   T->SetClassName(String::NewFromUtf8(I,"Script"));
   T->InstanceTemplate()->SetInternalFieldCount(1);

   NODE_SET_PROTOTYPE_METHOD(T,"getPath",getPath);
   NODE_SET_PROTOTYPE_METHOD(T,"getModuleName",getModuleName);
   NODE_SET_PROTOTYPE_METHOD(T,"exec",exec);

   constructor.Reset(I,T->GetFunction());
   exports->Set(String::NewFromUtf8(I,"Script"),T->GetFunction());
}

void nj::ScriptEncapsulated::NewInstance(const FunctionCallbackInfo<v8::Value> &args)
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
