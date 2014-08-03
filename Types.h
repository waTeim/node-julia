#ifndef __nj_Types
#define __nj_Types

#include "Type.h"

namespace nj
{
   const int null_type = 1;
   const int boolean_type = 2;
   const int int_type = 3;
   const int float_type = 4;
   const int string_type = 5;

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

   class Int_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Int_t>();  }

         Int_t():Type(int_type) {}
   };

   class Float_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Float_t>();  }

         Float_t():Type(float_type) {}
   };

   class String_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<String_t>();  }

         String_t():Type(string_type) {}
   };
};

#endif
