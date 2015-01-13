#ifndef __nj_NativeArray
#define __nj_NativeArray

#include <string>
#include <v8.h>
#include <node_version.h>

namespace nj
{
   template <typename V> class NativeArray
   {
      public:


      protected:

         V *_dptr;
         unsigned int _len;

      public:

#if NODE_MINOR_VERSION == 10

         NativeArray(const v8::Local<v8::Object> &array)
         {
            v8::Local<v8::Value> buffer = array->Get(v8::String::New("buffer"));
            v8::Local<v8::Value> byteOffset = array->Get(v8::String::New("byteOffset"));
            v8::Local<v8::Value> byteLength = array->Get(v8::String::New("byteLength"));

            _dptr = 0;
            _len = 0;

            if(buffer->IsUndefined() || byteOffset->IsUndefined() || byteLength->IsUndefined()) return;

            v8::Local<v8::Object> bufferObject = buffer->ToObject();
            unsigned int offset = byteOffset->Uint32Value();

            _len = byteLength->Uint32Value()/sizeof(V);
            _dptr = (V*)(((char*)bufferObject->GetIndexedPropertiesExternalArrayData()) + offset);
         }

#else

         NativeArray(const v8::Local<v8::Object> &array)
         {
            v8::Isolate *I = v8::Isolate::GetCurrent();
            v8::Local<v8::Value> buffer = array->Get(v8::String::NewFromUtf8(I,"buffer"));
            v8::Local<v8::Value> byteOffset = array->Get(v8::String::NewFromUtf8(I,"byteOffset"));
            v8::Local<v8::Value> byteLength = array->Get(v8::String::NewFromUtf8(I,"byteLength"));

            _dptr = 0;
            _len = 0;

            if(byteOffset->IsUndefined() || byteLength->IsUndefined() || buffer->IsUndefined()) return;

            unsigned int offset = byteOffset->Uint32Value();

            _len = byteLength->Uint32Value()/sizeof(V);
            _dptr = (V*)(((char*)array->GetIndexedPropertiesExternalArrayData()) + offset);
         }

#endif

         V *dptr() const { return _dptr; }
         unsigned int len() { return _len; }

   };

   enum NativeArrayType
   {
      none,
      Int8Array,
      Uint8Array,
      Int16Array,
      Uint16Array,
      Int32Array,
      Uint32Array,
      Float32Array,
      Float64Array
   };

   NativeArrayType toType(const std::string &name);
};

#endif
