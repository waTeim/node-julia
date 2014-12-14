#include <iostream>
#include "Kernel.h"
#include "JuliaHandle.h"

using namespace std;

vector<size_t> nj::JuliaHandle::none;
map<int64_t,nj::JuliaHandle*> nj::JuliaHandle::htab;
int64_t nj::JuliaHandle::next_htab_index = 0;

nj::JuliaHandle *nj::JuliaHandle::atIndex(int64_t htabIndex)
{
   map<int64_t,JuliaHandle*>::iterator i = htab.find(htabIndex);

   if(i == htab.end()) return 0;
 
   return i->second;
}

nj::JuliaHandle::JuliaHandle(jl_value_t *val,bool preserve) throw(JuliaException)
{ 
   _val = val;
  if(preserve)
  {
     Kernel *kernel = Kernel::getSingleton();

     preserve_index = kernel->preserve(val);
  }
  else preserve_index = -1;
  htab_index = -1;
}

int64_t nj::JuliaHandle::intern()
{
   if(htab_index == -1)
   {
      htab_index = next_htab_index++;
      htab[htab_index] = new JuliaHandle(*this);
      preserve_index = -1;
   }
   return htab_index;
}

nj::JuliaHandle::~JuliaHandle() throw(JuliaException)
{
   if(preserve_index != -1)
   {
     Kernel *kernel = Kernel::getSingleton();

     (void)kernel->free(preserve_index);
   }
   if(htab_index != -1)
   {
      htab[htab_index] = 0;
      htab.erase(htab_index);
   }
}
