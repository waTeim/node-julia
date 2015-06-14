#include <iostream>
#include <julia.h>
#include "NAlloc.h"
#include "JuAlloc.h"
#include "Kernel.h"

using namespace std;

map<int64_t,nj::JuAlloc*> nj::JuAlloc::pindex_index;

nj::JuAlloc::JuAlloc(jl_value_t *val) throw(JuliaException):Alloc()
{
   Kernel *kernel = Kernel::getSingleton();

   _pindex = kernel->preserve(val);
   _ptr = (char*)jl_array_data(val);
   _len = jl_array_len(val);
   pindex_index[_pindex] = this;
}

std::shared_ptr<nj::Alloc> nj::JuAlloc::create(jl_value_t *val) throw(JuliaException)
{
   Kernel *kernel = Kernel::getSingleton();
   int64_t pindex = kernel->get(val);
   std::shared_ptr<nj::Alloc> res;

   if(pindex == -1)
   {
      JuAlloc *L = new JuAlloc(val);
      int64_t index = L->store();

      res = alloc_list->get(index);
   }
   else
   {
      map<int64_t,JuAlloc*>::iterator s = pindex_index.find(pindex);

      if(s != pindex_index.end())
      {
         JuAlloc *L = s->second;

         if(L->index() != -1) res = alloc_list->get(s->second->index());
         else res = s->second->container()->loc(0);
      }
   }
   return res;
}

nj::JuAlloc::~JuAlloc()
{
   Kernel *kernel = Kernel::getSingleton();

   (void)kernel->free(_pindex);
   pindex_index[_pindex] = 0;
   pindex_index.erase(_pindex);
}
