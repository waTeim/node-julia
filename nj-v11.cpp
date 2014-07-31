#include <stdio.h>
#include <node.h>
#include <v8.h>
#include <string>
#include "Types.h"
#include "JuliaExecEnv.h"

using namespace std;
using namespace v8;

static JuliaExecEnv *J = 0;

void returnNull(const FunctionCallbackInfo<Value> &args,Isolate *I)
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

void buildArgs(Isolate *I,const std::shared_ptr<std::vector<std::shared_ptr<nj::Value>>> &res,int argc,Local<Value> *argv)
{
   int index = 0;
   for(std::shared_ptr<nj::Value> value: *res)
   {
printf("building arg %d\n",index);
      if(value.get() && value->isPrimitive())
      {
         const nj::Primitive &primitive = static_cast<const nj::Primitive&>(*value);

         switch(primitive.typeAtom())
         {
            case nj::null_type:
printf("arg is null\n");
               argv[index++] = Null(I);
            break;
            case nj::boolean_type:
printf("arg is %d\n",primitive.toBoolean());
               argv[index++] = Boolean::New(I,primitive.toBoolean());
            break;
            case nj::int_type:
printf("arg is %lld\n",primitive.toInt());
               argv[index++] = Number::New(I,primitive.toInt());
            break;
            case nj::float_type:
printf("arg is %f\n",primitive.toFloat());
               argv[index++] = Number::New(I,primitive.toFloat());
            break;
            case nj::string_type:
printf("arg is %s\n",primitive.toString().c_str());
               argv[index++] = String::NewFromUtf8(I,primitive.toString().c_str());
            break;
         }
      }
   }
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
   String::Utf8Value text(arg0);
   Local<Function> cb = Local<Function>::Cast(args[1]);
   JMain *engine;

   if(text.length() > 0 && (engine = J->getEngine()))
   {
      engine->evalQueuePut(*text);
      std::shared_ptr<std::vector<std::shared_ptr<nj::Value>>> res = engine->resultQueueGet();
  
      if(res.get())
      {
         int argc = res->size();
         Local<Value> *argv = new Local<Value>[argc];
         buildArgs(I,res,argc,argv);
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


void doStart(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   int numArgs = args.Length();

   if(numArgs == 0)
   {
      returnString(args,I,"");
      return;
   }

   Local<String> arg0 = args[0]->ToString();
   String::Utf8Value plainText_av(arg0);

   if(plainText_av.length() > 0)
   {
      if(!J) J = new JuliaExecEnv(*plainText_av);

      returnString(args,I,"Julia Started");
   }
   else returnString(args,I,"");
}

void init(Handle<Object> exports)
{
  NODE_SET_METHOD(exports,"start",doStart);
  NODE_SET_METHOD(exports,"eval",doEval);
}

NODE_MODULE(nj,init)
