#include <stdio.h>
#include <node.h>
#include <v8.h>
#include <mutex>
#include <thread>
#include <chrono>

using namespace std;
using namespace v8;


class JuliaEnv
{
   protected:

      mutex state_m;
      string s;

   public:

      JuliaEnv(string s) {  this->s = s; }
  
      string getS()
      { 
         unique_lock<mutex> lock(state_m);
 
         return s;
      }
};

static JuliaEnv *J = 0;

void init(const FunctionCallbackInfo<Value> &args)
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
      if(!J) J = new JuliaEnv(*plainText_av);

      args.GetReturnValue().Set(String::NewFromUtf8(I,J->getS().c_str()));
   }
   else args.GetReturnValue().Set(String::NewFromUtf8(I,""));
}
