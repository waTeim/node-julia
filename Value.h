#ifndef __nj_Value
#define __nj_Value

#include <vector>
#include "Exception.h"
#include "Type.h"

namespace nj
{
   class Value
   {
      public:

         virtual const Type *type() const = 0;
         virtual bool isPrimitive() const = 0;
         virtual const std::vector<int> &dim() const = 0;
         virtual ~Value() {}
   };

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
