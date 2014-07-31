#ifndef __nj_Value
#define __nj_Value

#include <vector>
#include "Exception.h"
#include "Type.h"

namespace nj
{

   template<typename V,int id> class VTraits
   {
      protected:
  
         V value;

      public:

         VTraits(const V &value) {  this->value = value;  }

         int getId() const {  return id;  }
         V getValue() const {  return value;  }
         
         ~VTraits() {}
   };

   class Value
   {
      public:

         virtual const Type *type() const = 0;
         virtual bool isPrimitive() const = 0;
         virtual const std::vector<int> &dim() const = 0;
         virtual ~Value() {}
   };

   const int null_type = 0;
   const int boolean_type = 1;
   const int int_type = 2;
   const int float_type = 3;
   const int string_type = 4;

   class Primitive: public Value
   {
        static std::vector<int> none;

      public:

         virtual bool isPrimitive() const {  return true;  }
         virtual const std::vector<int> &dim() const {  return none;  }
         virtual bool toBoolean() const throw(InvalidException) = 0;
         virtual int64_t toInt() const throw(InvalidException) = 0;
         virtual double toFloat() const throw(InvalidException) = 0;
         virtual std::string toString() const throw(InvalidException) = 0;
   };
};

#endif
