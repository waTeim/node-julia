#ifndef __nj_Types
#define __nj_Types

#include "Type.h"

namespace nj
{
   const int boolean_type = 64;
   const int int64_type = 65;
   const int uint64_type = 66;
   const int int32_type = 67;
   const int uint32_type = 68;
   const int int16_type = 69;
   const int uint16_type = 70;
   const int int8_type = 71;
   const int uint8_type = 72;
   const int float64_type = 73;
   const int float32_type = 74;
   const int ascii_string_type = 75;
   const int utf8_string_type = 76;

   class Boolean_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Boolean_t>();  }
      
         Boolean_t():Type(boolean_type) {}
   };

   class Int64_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Int64_t>();  }

         Int64_t():Type(int64_type) {}
   };

   class UInt64_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<UInt64_t>();  }

         UInt64_t():Type(uint64_type) {}
   };

   class Int32_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Int32_t>();  }

         Int32_t():Type(int32_type) {}
   };

   class UInt32_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<UInt32_t>();  }

         UInt32_t():Type(uint32_type) {}
   };

   class Int16_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Int16_t>();  }

         Int16_t():Type(int16_type) {}
   };

   class UInt16_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<UInt16_t>();  }

         UInt16_t():Type(uint16_type) {}
   };

   class Int8_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Int8_t>();  }

         Int8_t():Type(int8_type) {}
   };

   class UInt8_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<UInt8_t>();  }

         UInt8_t():Type(uint8_type) {}
   };

   class Float64_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Float64_t>();  }

         Float64_t():Type(float64_type) {}
   };

   class Float32_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Float32_t>();  }

         Float32_t():Type(float32_type) {}
   };

   class ASCIIString_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<ASCIIString_t>();  }

         ASCIIString_t():Type(ascii_string_type) {}
   };

   class UTF8String_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<UTF8String_t>();  }

         UTF8String_t():Type(utf8_string_type) {}
   };

   bool operator<(const nj::Type &t1,const nj::Type &t2);
};

#endif
