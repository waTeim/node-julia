#ifndef __nj_Callback
#define __nj_Callback

#include <v8.h>

namespace nj
{
   class Callback
   {
      protected:

         v8::Persistent<v8::Function> resource;

      public:
     
         Callback(v8::Local<v8::Function> cb);
         v8::Local<v8::Function> cb();
         ~Callback();
   };
};

#endif
