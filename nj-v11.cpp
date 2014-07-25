#include <stdio.h>
#include <node.h>
#include <v8.h>
#include <string>
#include "JuliaExecEnv.h"

using namespace std;
using namespace v8;

static JuliaExecEnv *J = 0;

void returnString(const FunctionCallbackInfo<Value> &args,Isolate *I,const string &s)
{
   args.GetReturnValue().Set(String::NewFromUtf8(I,""));
}

void doEval(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   int numArgs = args.Length();

   if(numArgs == 0 || !J)
   {
      returnString(args,I,"");
      return;
   }

   Local<String> arg0 = args[0]->ToString();
   String::Utf8Value expressionText(arg0);
   JMain *engine;

   if(expressionText.length() > 0 && (engine = J->getEngine()))
   {
      engine->evalQueuePut(*expressionText);
      returnString(args,I,engine->resultQueueGet());
   }
   else returnString(args,I,"");
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
