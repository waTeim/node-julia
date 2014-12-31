#include <node_version.h>
#include "Callback.h"

using namespace v8;

nj::Callback::Callback(Local<Function> cb):resource()
{ 
   Isolate *I = Isolate::GetCurrent();

   resource.Reset(I,cb);
}

Local<Function> nj::Callback::cb()
{
   Isolate *I = Isolate::GetCurrent();

   return Local<Function>::New(I,resource);
}

nj::Callback::~Callback()
{
   resource.Reset();
}
