#include <iostream>
#include "NAlloc.h"
#include "JSAlloc.h"

using namespace std;

map<int,vector<v8::UniquePersistent<v8::Object>>> *nj::JSAlloc::obj_map;
map<int,vector<shared_ptr<nj::Alloc>>> *nj::JSAlloc::alloc_map;

void nj::JSAlloc::CollectWeak(const v8::WeakCallbackData<v8::Object,JSAlloc> &data)
{
   JSAlloc *L = data.GetParameter();

   if(L)
   {
      if(L->_container) L->_container->free();
      else L->free();
   }
}

shared_ptr<nj::Alloc> nj::JSAlloc::create(const v8::Local<v8::Object> &obj)
{
   JSAlloc *L = new JSAlloc(obj);
   int64_t index = L->store();
   shared_ptr<nj::Alloc> res = alloc_list->get(index);

   if(!obj_map)
   {
      obj_map = new map<int,vector<v8::UniquePersistent<v8::Object>>>();
      alloc_map = new map<int,vector<shared_ptr<Alloc>>>();
   }
    
   (*obj_map)[obj->GetIdentityHash()].push_back(v8::UniquePersistent<v8::Object>(v8::Isolate::GetCurrent(),obj));
   (*alloc_map)[obj->GetIdentityHash()].push_back(res);

   return res;
}

shared_ptr<nj::Alloc> nj::JSAlloc::find(const v8::Local<v8::Object> &obj)
{
   shared_ptr<nj::Alloc> res;

   if(!obj_map) return res;

   v8::UniquePersistent<v8::Object> id(v8::Isolate::GetCurrent(),obj);
   map<int,vector<v8::UniquePersistent<v8::Object>>>::iterator searchResult = obj_map->find(obj->GetIdentityHash());

   if(searchResult != obj_map->end())
   {
      const vector<v8::UniquePersistent<v8::Object>> &objVec = searchResult->second;
      const vector<shared_ptr<nj::Alloc>> &allocVec = (*alloc_map)[obj->GetIdentityHash()];
      size_t index = 0;

      while(index < objVec.size())
      {
         if(objVec[index] == id) break;
         index++;
      }

      if(index < objVec.size()) res = allocVec[index];
   }
   return res;
}


nj::JSAlloc::JSAlloc(const v8::Local<v8::Object> &obj):Alloc(),_obj(v8::Isolate::GetCurrent(),obj)
{
   _obj.SetWeak(this,CollectWeak);
}

nj::JSAlloc::~JSAlloc()
{
   _obj.Reset();
}
