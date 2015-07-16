#include <node_version.h>
#include "Callback.h"

using namespace v8;

nj::Callback::Callback(const Local<Function> &cb,const Local<Object> &recv):callback_persist(),recv_persist()
{
   Isolate *I = Isolate::GetCurrent();

   callback_persist.Reset(I,cb);
   recv_persist.Reset(I,recv);
}

Local<Function> nj::Callback::cb()
{
   Isolate *I = Isolate::GetCurrent();

   return Local<Function>::New(I,callback_persist);
}

Local<Object> nj::Callback::recv()
{
   Isolate *I = Isolate::GetCurrent();

   return Local<Object>::New(I,recv_persist);
}

nj::Callback::~Callback()
{
   callback_persist.Reset();
   recv_persist.Reset();
}
