#include "Alloc.h"

using namespace std;

nj::FreeList<nj::Alloc> *nj::Alloc::alloc_list = 0;

int64_t nj::Alloc::store()
{
   if(_index == -1)
   {
       if(!alloc_list) alloc_list = new FreeList<Alloc>();
       _index = alloc_list->store(this);
   }
   return _index;
}

shared_ptr<nj::Alloc> nj::Alloc::free()
{
   shared_ptr<Alloc> res;

   if(_index != -1)
   {
      res = alloc_list->free(_index);
      _index = -1;
   }
   return res;
}
