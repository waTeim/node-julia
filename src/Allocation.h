#ifndef __nj_Allocation
#define __nj_Allocation

#include <v8.h>
#include <map>
#include <vector>
#include <memory>
#include "FreeList.h"
#include "Exception.h"

extern "C"
{
   typedef struct _jl_value_t jl_value_t;
};

namespace nj
{
   class Allocation
   {
      protected:

         static FreeList<Allocation> alloc_list;

         static void CollectWeak(const v8::WeakCallbackData<v8::Object,Allocation> &data);

         int64_t index;
         int64_t pindex;
         jl_value_t *julia_val;
         v8::UniquePersistent<v8::Object> v8_val;
         char *buffer;

      public:

         static void free(size_t index);
         static std::shared_ptr<Allocation> get(int64_t index);
         static int64_t store(jl_value_t *juliaVal = 0) throw(JuliaException);
         static int64_t store(const v8::Local<v8::Object> &v8Val,char *buffer);
         static int64_t store(const v8::Local<v8::Object> &v8Val,jl_value_t *juliaVal) throw(JuliaException);

         Allocation();
         Allocation(const v8::Local<v8::Object> &v8Val);
         ~Allocation();
   };

};

#endif
