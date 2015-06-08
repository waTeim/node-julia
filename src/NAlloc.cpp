#include <iostream>
#include "NAlloc.h"

using namespace std;

nj::NAlloc::NAlloc(const shared_ptr<Alloc> &loc0):Alloc()
{
   locus.push_back(loc0);
   loc0->setContainer(this);
}

std::shared_ptr<nj::Alloc> nj::NAlloc::create(const shared_ptr<Alloc> &loc0)
{
   NAlloc *res = new NAlloc(loc0);
   int64_t index = res->store();

   return alloc_list.get(index);
}

shared_ptr<nj::Alloc> nj::NAlloc::free()
{
   for(shared_ptr<Alloc> loc: locus) (void)loc->free();
   return Alloc::free();
}

int64_t nj::NAlloc::store()
{
   int64_t i = Alloc::store();

   for(shared_ptr<Alloc> loc: locus) (void)loc->store();
   return i;
}

void nj::NAlloc::add(const shared_ptr<Alloc> &loc)
{
   locus.push_back(loc);
   loc->setContainer(this);
}
