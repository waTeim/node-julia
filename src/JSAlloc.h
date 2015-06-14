#ifndef __nj_JSAlloc
#define __nj_JSAlloc

#include <map>
#include <v8.h>
#include "Alloc.h"

namespace nj
{
   class JSAlloc: public Alloc
   {
      protected:

         static std::map<int,std::vector<v8::UniquePersistent<v8::Object>>> *obj_map;
         static std::map<int,std::vector<std::shared_ptr<Alloc>>> *alloc_map;

         static void CollectWeak(const v8::WeakCallbackData<v8::Object,JSAlloc> &data);

         v8::UniquePersistent<v8::Object> _obj;

         JSAlloc(const v8::Local<v8::Object> &obj);

      public:

         static std::shared_ptr<Alloc> create(const v8::Local<v8::Object> &obj);
         static std::shared_ptr<Alloc> find(const v8::Local<v8::Object> &obj);

         virtual char *ptr() const { return 0; }
         virtual size_t len() const { return 0; }
         virtual v8::Local<v8::Object> obj() { return v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(),_obj); }
         virtual ~JSAlloc();
   };

};

#endif
