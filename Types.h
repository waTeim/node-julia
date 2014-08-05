#ifndef __nj_Types
#define __nj_Types

#include "Type.h"

namespace nj
{
   const int null_type = 1;
   const int boolean_type = 2;
   const int char_type = 3;
   const int uchar_type = 4;
   const int int64_type = 5;
   const int uint64_type = 6;
   const int int32_type = 7;
   const int uint32_type = 8;
   const int int16_type = 9;
   const int uint16_type = 10;
   const int float64_type = 11;
   const int float32_type = 12;
   const int string_type = 13;

   class Null_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Null_t>();  }
      
         Null_t():Type(null_type) {}
   };

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

   class Char_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Char_t>();  }

         Char_t():Type(char_type) {}
   };

   class UChar_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<UChar_t>();  }

         UChar_t():Type(uchar_type) {}
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

   class String_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<String_t>();  }

         String_t():Type(string_type) {}
   };
};

#endif
