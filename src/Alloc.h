#ifndef __nj_Alloc
#define __nj_Alloc

#include "FreeList.h"

namespace nj
{
   class NAlloc;

   class Alloc
   {
      protected:

         static FreeList<Alloc> *alloc_list;

         int64_t _index;
         NAlloc *_container;

      public:

         Alloc()
         { 
            _index = -1;
            _container = 0;
         }

         void setContainer(NAlloc *container) { _container = container; }
         virtual char *ptr() const = 0;
         virtual size_t len() const = 0;
         virtual std::shared_ptr<Alloc> free();
         virtual int64_t store();
         virtual ~Alloc() {}
   };
};

#endif
