#include <stdio.h>
#include <node.h>
#include <v8.h>
#include <string>
#include "JuliaExecEnv.h"

using namespace std;
using namespace v8;

static JuliaExecEnv *J = 0;

void doStart(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   int numArgs = args.Length();

   if(numArgs == 0)
   {
      args.GetReturnValue().Set(String::NewFromUtf8(I,""));
      return;
   }

   Local<String> arg0 = args[0]->ToString();
   String::Utf8Value plainText_av(arg0);

   if(plainText_av.length() > 0)
   {
      if(!J) J = new JuliaExecEnv(*plainText_av);

      args.GetReturnValue().Set(String::NewFromUtf8(I,"Julia Started"));
   }
   else args.GetReturnValue().Set(String::NewFromUtf8(I,""));
}

void init(Handle<Object> exports)
{
  NODE_SET_METHOD(exports,"start",doStart);
}

NODE_MODULE(nj,init)
