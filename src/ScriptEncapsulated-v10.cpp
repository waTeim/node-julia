#include <iostream>
#include <node.h>
#include "ScriptEncapsulated-v10.h"
#include "JuliaExecEnv.h"
#include "request.h"
#include "nj-v10.h"

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

Handle<Value> nj::ScriptEncapsulated::New(const Arguments& args)
{
   HandleScope scope;

   if(args.IsConstructCall())
   {
      // Invoked as constructor: `new julia.script(...)`
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

         if(exId != nj::Exception::no_exception) return raiseException(scope,cruw);
         else
         {  
            unwrapped->Wrap(args.This());
            return args.This();
         }
      }
      return Undefined();
   }
   else
   {
      // Invoked as plain function `julia.Script(...)`, turn into construct call.
      const int argc = 1;
      Local<v8::Value> argv[argc] = { args[0] };

      return scope.Close(constructor->NewInstance(argc,argv));
   }
}

Handle<Value> nj::ScriptEncapsulated::getPath(const Arguments &args)
{
   HandleScope scope;
   ScriptEncapsulated *obj = ObjectWrap::Unwrap<ScriptEncapsulated>(args.This());

   return scope.Close(String::New(obj->path.c_str()));
}

Handle<Value> nj::ScriptEncapsulated::getModuleName(const Arguments &args)
{
   HandleScope scope;
   ScriptEncapsulated *obj = ObjectWrap::Unwrap<ScriptEncapsulated>(args.This());

   return scope.Close(String::New(obj->module_name.c_str()));
}

Handle<Value> nj::ScriptEncapsulated::exec(const Arguments &args)
{
   HandleScope scope;
   ScriptEncapsulated *obj = ObjectWrap::Unwrap<ScriptEncapsulated>(args.This());
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
         shared_ptr<nj::Value> reqElement = buildRequest(args[i]);

         if(reqElement.get()) req.push_back(reqElement);
      }
      engine->exec(obj->compile_res->results()[1],funcName,req);
      shared_ptr<nj::Result> res = engine->resultQueueGet();

      if(useCallback) return callbackWithResult(scope,cb,res);
      else return returnResult(scope,res);
   }
   return scope.Close(Undefined());
}

void nj::ScriptEncapsulated::Init(Handle<Object> exports)
{
   Local<FunctionTemplate> T = FunctionTemplate::New(New);

   T->SetClassName(String::NewSymbol("Script"));
   T->InstanceTemplate()->SetInternalFieldCount(1);
   T->PrototypeTemplate()->Set(String::NewSymbol("getPath"),FunctionTemplate::New(getPath)->GetFunction());
   T->PrototypeTemplate()->Set(String::NewSymbol("getModuleName"),FunctionTemplate::New(getModuleName)->GetFunction());
   T->PrototypeTemplate()->Set(String::NewSymbol("exec"),FunctionTemplate::New(exec)->GetFunction());
   constructor = Persistent<Function>::New(T->GetFunction());
   exports->Set(String::NewSymbol("Script"),constructor);
}

Handle<Value> nj::ScriptEncapsulated::NewInstance(const Arguments& args)
{
   HandleScope scope;
   const unsigned argc = 1;
   Handle<v8::Value> argv[argc] = { args[0] };
   Local<Object> instance = constructor->NewInstance(argc,argv);

   if(instance.IsEmpty() || instance->IsUndefined()) return scope.Close(Undefined());
   return scope.Close(instance);
}
