#include <unistd.h>
#include <iostream>
#include <uv.h>
#include "nj-v10.h"
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

void doDispatch(uv_async_t *handle,int status)
{
   Trampoline *trampoline = JuliaExecEnv::getSingleton()->getTrampoline();
   list<shared_ptr<nj::Binding>> work = trampoline->getCallList();

   for(shared_ptr<nj::Binding> binding: work)
   {
      HandleScope scope;

      callbackWithResult(scope,binding->target()->cb(),binding->res(),true);
   }
}

void nj::dispatch_init()
{
  dchan.data = 0;
  uv_async_init(uv_default_loop(),&dchan,doDispatch);
  uv_unref((uv_handle_t*)&dchan);
}
