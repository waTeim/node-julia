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

   const int array_type = 0;

   class Array_t:public Type
   {
      public:

         static Type *instance() {  return Type::instance<Array_t>();  }

         Array_t():Type(array_type) {}
   };
};

#endif
