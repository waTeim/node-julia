#ifndef __nj_Value
#define __nj_Value

#include <memory>
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
         virtual const std::vector<size_t> &dims() const = 0;
         virtual ~Value() throw(JuliaException) {}
   };

   class Primitive: public Value
   {
      protected:

        static std::vector<size_t> none;

      public:

         virtual bool isPrimitive() const {  return true;  }
         virtual const std::vector<size_t> &dims() const {  return none;  }
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

         std::vector<size_t> dimensions;
         std::shared_ptr<std::vector<V>> data;
         size_t num_elements;
         bool _owned;

         struct D
         {
            Array *a;

            D(Array *a) { this->a = a; }

            void operator()(std::vector<V> *p) const
            {
               if(a->owned()) delete p;
            }
         };
     
      public:

         Array(const std::vector<size_t> &dimensions,bool owned = true)
         {  
            _owned = owned;
            this->dimensions = dimensions;
            if(dimensions.size() == 0) num_elements = 0;
            else
            {
               num_elements = 1;
               for(size_t dimension: dimensions) num_elements *= dimension;
               if(num_elements) data = std::shared_ptr<std::vector<V>>(new std::vector<V>(num_elements),D(this));
            }
         }

         virtual bool isPrimitive() const {  return false;  }
         virtual const std::vector<size_t> &dims() const {  return dimensions;  }
         virtual const Type *type() const {  return Array_t::instance(E::instance());  }
         virtual V *ptr() const {  return data.get()?data.get()->data():0;  }
         virtual size_t size() const {  return num_elements;  }
         virtual bool owned() { return _owned; }
         virtual void relinguish() { _owned = false; }
         virtual ~Array() throw(JuliaException) {}
   };
};

#endif
