#include <node_version.h>
#include "Callback.h"

using namespace v8;

nj::Callback::Callback(const Local<Function> &cb,const Local<Object> &recv)
{
   callback_persist = Persistent<Function>::New(cb);
   recv_persist = Persistent<Object>::New(recv);
}

Local<Function> nj::Callback::cb()
{
   return *callback_persist;
}

Local<Object> nj::Callback::recv()
{
   return *recv_persist;
}

nj::Callback::~Callback()
{
   callback_persist.Dispose();
   callback_persist.Clear();
   recv_persist.Dispose();
   recv_persist.Clear();
}
