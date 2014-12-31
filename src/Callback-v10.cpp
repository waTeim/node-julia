#include <node_version.h>
#include "Callback.h"

using namespace v8;

nj::Callback::Callback(Local<Function> cb)
{ 
   resource = Persistent<Function>::New(cb);
}

Local<Function> nj::Callback::cb()
{
   return *resource;
}

nj::Callback::~Callback()
{
   resource.Dispose();
   resource.Clear();
}
