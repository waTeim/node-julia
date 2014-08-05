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
         virtual const std::vector<int> &dims() const = 0;
         virtual ~Value() {}
   };

   class Primitive: public Value
   {
      protected:

        static std::vector<int> none;

      public:

         virtual bool isPrimitive() const {  return true;  }
         virtual const std::vector<int> &dims() const {  return none;  }
         virtual bool toBoolean() const throw(InvalidException) = 0;
         virtual char toChar() const throw(InvalidException) = 0;
         virtual int64_t toInt() const throw(InvalidException) = 0;
         virtual uint64_t toUInt() const throw(InvalidException) = 0;
         virtual double toFloat() const throw(InvalidException) = 0;
         virtual std::string toString() const throw(InvalidException) = 0;
   };
   
   template <typename V,typename E> class Array: public Value
   {
      protected:

         std::vector<int> dimensions;
         std::shared_ptr<V> data;
         size_t numElements;

      public:

         Array(const std::vector<int> &dimensions)
         {  
            this->dimensions = dimensions;
            numElements = 1;
            for(int dimension: dimensions) numElements *= dimension;
            data = std::shared_ptr<V>(new V[numElements]);
         }

         virtual bool isPrimitive() const {  return false;  }
         virtual const std::vector<int> &dims() const {  return dimensions;  }
         virtual const Type *type() const {  return Array_t::instance(E::instance());  }
         virtual V *ptr() const {  return data.get();  }
         virtual size_t size() const {  return numElements;  }
   };
};

#endif
