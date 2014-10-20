#ifndef __nj_Type
#define __nj_Type

#include <map>
#include "Exception.h"

namespace nj
{
   class Type
   {
      protected:

         int id;
   
      public:
 
         template<class T> static Type *instance()
         {
            static Type *i = 0;

            if(!i) i = new T();
            return i;
         }

         Type(int id)
         {  
            this->id = id;
         }

         virtual int getId() const {  return id;  }

   };

   const int null_type = 0;
   const int any_type = 1;
   const int array_type = 2;
   const int julia_handle_type = 3;

   class Null_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Null_t>();  }

         Null_t():Type(null_type) {}
   };

   class Any_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Any_t>();  }

         Any_t():Type(any_type) {}
   };

   class Array_t:public Type
   {
      protected:
 
         static std::map<Type*,Array_t*> atypes;

         Type *element_type;

      public:

         static Type *instance(Type *elementType)
         {
            if(atypes.find(elementType) == atypes.end())
            {
               atypes[elementType] = new Array_t(elementType);
            }   
            return atypes[elementType];
         }

         Array_t(Type *elementType):Type(array_type) {  element_type = elementType;  }
 
         Type const *etype() const {  return element_type;  }
   };

   class JuliaHandle_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<JuliaHandle_t>();  }

         JuliaHandle_t():Type(julia_handle_type) {}
   };
};

#endif
