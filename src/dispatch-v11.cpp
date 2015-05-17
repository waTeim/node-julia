#include <iostream>
#include <uv.h>
#include "nj-v11.h"
#include "Callback.h"
#include "Trampoline.h"
#include "JuliaExecEnv.h"
#include "dispatch.h"

using namespace std;
using namespace v8;

static uv_async_t dchan;

void nj::async_dispatch()
{
   uv_async_send(&dchan);
}

void doDispatch(uv_async_t *handle)
{
   Trampoline *trampoline = JuliaExecEnv::getSingleton()->getTrampoline();
   list<shared_ptr<nj::Binding>> work = trampoline->getCallList();
   Isolate *I = Isolate::GetCurrent();

   for(shared_ptr<nj::Binding> binding: work)
   {
      HandleScope scope(I);

      callbackWithResult(scope,binding->target()->cb(),binding->res());
   }
}

void nj::dispatch_init()
{
  dchan.data = 0;
  uv_async_init(uv_default_loop(),&dchan,doDispatch);
  uv_unref((uv_handle_t*)&dchan);
}
