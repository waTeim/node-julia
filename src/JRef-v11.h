#ifndef __nj_JRef
#define __nj_JRef

#include <string>
#include <map>
#include <memory>
#include <node.h>
#include <node_object_wrap.h>


namespace nj
{
   class JuliaHandle;

   class JRef: public node::ObjectWrap
   {
      private:

         static std::map<int64_t,JRef*> obj_chain;

         static void New(const v8::FunctionCallbackInfo<v8::Value> &args);
         static void getHIndex(const v8::FunctionCallbackInfo<v8::Value> &args);
         static void getProperty(v8::Local<v8::String> property,const v8::PropertyCallbackInfo<v8::Value> &info);


         int64_t h_index;
         JRef *next;
         JRef *previous;
         std::shared_ptr<nj::JuliaHandle> handle;

         explicit JRef(int64_t hIndex);
         ~JRef();

      public:

         static v8::Persistent<v8::Function> constructor;
         static void Init(v8::Handle<v8::Object> exports);
         static void NewInstance(const v8::FunctionCallbackInfo<v8::Value> &args);

         std::shared_ptr<nj::JuliaHandle> getHandle() const { return handle; }
   };
};

#endif
