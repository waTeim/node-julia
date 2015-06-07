#ifndef __nj_VAlloc
#define __nj_VAlloc

#include <iostream>
#include <v8.h>
#include "Alloc.h"

namespace nj
{
   template <typename V> class VAlloc: public Alloc
   {
      protected:

         std::shared_ptr<std::vector<V>> _v;

         VAlloc(size_t numElements)
         {
            if(numElements) _v = std::shared_ptr<std::vector<V>>(new std::vector<V>(numElements));
         }

         VAlloc(const std::shared_ptr<std::vector<V>> &v) { _v = v; }

      public:

         static std::shared_ptr<Alloc> create(size_t numElements)
         {
            VAlloc *res = new VAlloc(numElements);
            int64_t index = res->store();

            return alloc_list.get(index);
         }

         static std::shared_ptr<Alloc> create(const std::shared_ptr<std::vector<V>> &v)
         {
            VAlloc *res = new VAlloc(v);
            int64_t index = res->store();

            return alloc_list.get(index);
         }

         virtual char *ptr() const {  return (char*)(_v.get()?_v.get()->data():0);  }
         virtual size_t len() const { return _v.get()?_v->size()*sizeof(V):0; }
         virtual ~VAlloc() {}
   };

};

#endif
