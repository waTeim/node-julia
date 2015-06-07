#include <iostream>
#include "NAlloc.h"
#include "JSAlloc.h"

using namespace std;

void nj::JSAlloc::CollectWeak(const v8::WeakCallbackData<v8::Object,JSAlloc> &data)
{
   JSAlloc *L = data.GetParameter();

   if(L)
   {
      if(L->_container) L->_container->free();
      else L->free();
   }
}

std::shared_ptr<nj::Alloc> nj::JSAlloc::create(const v8::Local<v8::Object> &v8Obj)
{
   JSAlloc *res = new JSAlloc(v8Obj);
   int64_t index = res->store();

   return alloc_list.get(index);
}

nj::JSAlloc::JSAlloc(const v8::Local<v8::Object> &v8Obj):Alloc(),v8_obj(v8::Isolate::GetCurrent(),v8Obj)
{
   v8_obj.SetWeak(this,CollectWeak);
}

nj::JSAlloc::~JSAlloc()
{
   v8_obj.Reset();
}
