#ifndef __nj_Value
#define __nj_Value

#include "Exception.h"

namespace nj
{
   template<typename V,int id> class VTraits
   {
      protected:
  
         V value;

      public:

         VTraits(const V &value) {  this->value = value;  }

         int getTypeId() const {  return id;  }
         V getValue() const {  return value;  }
         ~VTraits() {}
   };

   class Value
   {
      public:

         virtual int getTypeId() = 0;
         virtual bool toBoolean()  throw(InvalidException) = 0;
         virtual int64_t toInt()  throw(InvalidException) = 0;
         virtual double toFloat()  throw(InvalidException) = 0;
         virtual std::string toString() throw(InvalidException) = 0;
         virtual ~Value() {}
   };

   const int null_type = 0;
   const int boolean_type = 1;
   const int int_type = 2;
   const int float_type = 3;
   const int string_type = 4;
};

#endif
