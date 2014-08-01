#ifndef __nj_Types
#define __nj_Types

#include "Type.h"

namespace nj
{
   const int null_type = 0;
   const int boolean_type = 1;
   const int int_type = 2;
   const int float_type = 3;
   const int string_type = 4;

   class Null_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Null_t>();  }
      
         Null_t():Type(null_type,0) {}
   };

   class Boolean_t:public Type
   {
      public:

         static Type *instance(int card) {  return Type::instance<Boolean_t>(card);  }
      
         Boolean_t(int card):Type(boolean_type,card) {}
   };

   class Int_t:public Type
   {
      public:

         static Type *instance(int card) {  return Type::instance<Int_t>(card);  }

         Int_t(int card):Type(int_type,card) {}
   };

   template class Float_t:public Type
   {
      protected:

         int card;

      public:

         static Type *instance(int card) {  return Type::instance<Float_t>(card);  }

         Float_t(int card):Type(float_type,card) {}
   };

   template class String_t:public Type
   {
      protected:

         int card;

      public:

         static Type *instance(int card) {  return Type::instance<String_t>(card);  }

         String_t(int card):Type(string_type,card) {}
   };
};

#endif
