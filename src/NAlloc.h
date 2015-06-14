#ifndef __nj_NAlloc
#define __nj_NAlloc

#include <iostream>
#include "Alloc.h"

namespace nj
{

   class NAlloc: public Alloc
   {
      protected:

         std::vector<std::shared_ptr<Alloc>> locus;

         NAlloc() {}
         NAlloc(const std::shared_ptr<Alloc> &loc0);

      public:

         static std::shared_ptr<Alloc> create(const std::shared_ptr<Alloc> &loc0);

         virtual std::shared_ptr<Alloc> loc(size_t i) const { return locus.size() > i?locus[i]:std::shared_ptr<Alloc>(); }
         virtual char *ptr() const { return locus.size() > 0?locus[0]->ptr():0; }
         virtual size_t len() const { return locus.size() > 0?locus[0]->len():0; }
         virtual std::shared_ptr<Alloc> free();
         virtual int64_t store();
         virtual void add(const std::shared_ptr<Alloc> &loc);
         virtual ~NAlloc() {}
   };
};

#endif
