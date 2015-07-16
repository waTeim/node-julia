#ifndef __nj_Callback
#define __nj_Callback

#include <v8.h>

namespace nj
{
   class Callback
   {
      protected:

         v8::Persistent<v8::Function> callback_persist;
         v8::Persistent<v8::Object> recv_persist;

      public:

         Callback(const v8::Local<v8::Function> &cb,const v8::Local<v8::Object> &recv);
         v8::Local<v8::Function> cb();
         v8::Local<v8::Object> recv();
         ~Callback();
   };
};

#endif
