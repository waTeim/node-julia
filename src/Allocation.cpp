#include <iostream>
#include "Allocation.h"
#include "Kernel.h"

using namespace std;

nj::FreeList<nj::Allocation> nj::Allocation::alloc_list;

void nj::Allocation::CollectWeak(const v8::WeakCallbackData<v8::Object,Allocation> &data)
{
   Allocation *L = data.GetParameter();

   L->v8_val.Reset();
   if(L->index != -1) alloc_list.free(L->index);
}

shared_ptr<nj::Allocation> nj::Allocation::get(int64_t index)
{
   return alloc_list.get(index);
}

int64_t nj::Allocation::store(jl_value_t *juliaVal) throw(JuliaException)
{
   Allocation *res = new Allocation();

   res->julia_val = juliaVal;
   res->buffer = 0;
   if(res->julia_val)
   {
      Kernel *kernel = Kernel::getSingleton();

      res->pindex = kernel->preserve(res->julia_val);
   }

   res->index = alloc_list.store(res);
   return res->index;
}

int64_t nj::Allocation::store(const v8::Local<v8::Object> &v8Val,char *buffer)
{
   Allocation *res = new Allocation(v8Val);

   res->julia_val = 0;
   res->buffer = buffer;
   res->index = alloc_list.store(res);
   return res->index;
}

int64_t nj::Allocation::store(const v8::Local<v8::Object> &v8Val,jl_value_t *juliaVal) throw(JuliaException)
{
   Allocation *res = new Allocation(v8Val);

   res->julia_val = juliaVal;
   res->buffer = 0;
   if(res->julia_val)
   {
      Kernel *kernel = Kernel::getSingleton();

      res->pindex = kernel->preserve(res->julia_val);
   }

   res->index = alloc_list.store(res);
   return res->index;
}

nj::Allocation::Allocation():v8_val()
{
   index = -1;
   pindex = -1;
   julia_val = 0;
}

nj::Allocation::Allocation(const v8::Local<v8::Object> &v8Obj):v8_val(v8::Isolate::GetCurrent(),v8Obj)
{
   v8_val.SetWeak(this,CollectWeak);
   index = -1;
   pindex = -1;
   julia_val = 0;
}

nj::Allocation::~Allocation()
{
   if(pindex != -1)
   {
      Kernel *kernel = Kernel::getSingleton();

      kernel->free(pindex);
   }
   if(buffer) delete buffer;
}

