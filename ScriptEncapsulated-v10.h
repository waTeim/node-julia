#ifndef __nj_ScriptEncapsulated
#define __nj_ScriptEncapsulated

#include <string>
#include <memory>
#include <node.h>
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

         static v8::Handle<v8::Value> New(const v8::Arguments& args);
         static v8::Handle<v8::Value> getPath(const v8::Arguments& args);
         static v8::Handle<v8::Value> getModuleName(const v8::Arguments& args);
         static v8::Handle<v8::Value> exec(const v8::Arguments& args);

      public:

         static void Init(v8::Handle<v8::Object> exports);
         static v8::Handle<v8::Value> NewInstance(const v8::Arguments &args);
   };
};

#endif
