#ifndef __nj_ScriptEncapsulated
#define __nj_ScriptEncapsulated

#include <string>
#include <memory>
#include <node.h>
#include <node_object_wrap.h>
#include "Result.h"

namespace nj
{
   class ScriptEncapsulated: public node::ObjectWrap
   {
      private:

         std::string path;
         std::string module_name;
         std::shared_ptr<nj::Result> compile_res;

         static v8::Persistent<v8::Function> constructor;

         explicit ScriptEncapsulated(std::string path = "");
         ~ScriptEncapsulated();

         static void New(const v8::FunctionCallbackInfo<v8::Value> &args);
         static void getPath(const v8::FunctionCallbackInfo<v8::Value> &args);
         static void getModuleName(const v8::FunctionCallbackInfo<v8::Value> &args);
         static void exec(const v8::FunctionCallbackInfo<v8::Value> &args);

      public:

         static void Init(v8::Handle<v8::Object> exports);
         static void NewInstance(const v8::FunctionCallbackInfo<v8::Value> &args);
   };
};

#endif
