#ifndef __nj_JSAlloc
#define __nj_JSAlloc

#include <v8.h>
#include "Alloc.h"

namespace nj
{
   class JSAlloc: public Alloc
   {
      protected:

         static void CollectWeak(const v8::WeakCallbackData<v8::Object,JSAlloc> &data);

         v8::UniquePersistent<v8::Object> v8_obj;

         JSAlloc(const v8::Local<v8::Object> &v8Obj);

      public:

         static std::shared_ptr<Alloc> create(const v8::Local<v8::Object> &v8Obj);

         virtual char *ptr() const { return 0; }
         virtual size_t len() const { return 0; }
         virtual ~JSAlloc();
   };

};

#endif
