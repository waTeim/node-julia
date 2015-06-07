#include <iostream>
#include <julia.h>
#include "NAlloc.h"
#include "JuAlloc.h"
#include "Kernel.h"

using namespace std;

nj::JuAlloc::JuAlloc(jl_value_t *val) throw(JuliaException):Alloc()
{
   Kernel *kernel = Kernel::getSingleton();

   _pindex = kernel->preserve(val);
}

std::shared_ptr<nj::Alloc> nj::JuAlloc::create(jl_value_t *val) throw(JuliaException)
{
   JuAlloc *res = new JuAlloc(val);
   int64_t index = res->store();

   return alloc_list.get(index);
}

nj::JuAlloc::~JuAlloc()
{
   Kernel *kernel = Kernel::getSingleton();

   (void)kernel->free(_pindex);
}
