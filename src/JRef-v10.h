#ifndef __nj_JRef
#define __nj_JRef

#include <string>
#include <map>
#include <memory>
#include <node.h>

namespace nj
{
   class JuliaHandle;

   class JRef: public node::ObjectWrap
   {
      private:

         static std::map<int64_t,JRef*> obj_chain;

         static v8::Handle<v8::Value> New(const v8::Arguments& args);
         static v8::Handle<v8::Value> getHIndex(const v8::Arguments& args);

         int64_t h_index;
         JRef *next;
         JRef *previous;
         std::shared_ptr<nj::JuliaHandle> handle;

         explicit JRef(int64_t hIndex);
         ~JRef();

      public:

         static v8::Persistent<v8::Function> constructor;
         static void Init(v8::Handle<v8::Object> exports);
         static v8::Handle<v8::Value> NewInstance(const v8::Arguments &args);

         std::shared_ptr<nj::JuliaHandle> getHandle() const { return handle; }
   };
};

#endif
