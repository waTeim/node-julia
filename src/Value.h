#ifndef __nj_Value
#define __nj_Value

#include <memory>
#include <vector>
#include "Exception.h"
#include "Type.h"
#include "NAlloc.h"
#include "VAlloc.h"
#include "JuAlloc.h"

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

         std::vector<size_t> _dimensions;
         std::shared_ptr<Alloc> _v;
         size_t _num_elements;

      public:

         Array(const std::vector<size_t> &dimensions,jl_value_t *val = 0)
         {  
            _dimensions = dimensions;
            if(_dimensions.size() == 0) _num_elements = 0;
            else
            {
               _num_elements = 1;
               for(size_t dimension: _dimensions) _num_elements *= dimension;
               if(_num_elements)
               {
                   if(val) _v = JuAlloc::create(val);
                   else _v = VAlloc<V>::create(_num_elements);
               }
            }
         }

         Array(const std::shared_ptr<Alloc> &v)
         {
            if(v->container()) _v = v->container()->loc0();
            else _v = v;
         }

         virtual bool isPrimitive() const {  return false;  }
         virtual const std::vector<size_t> &dims() const {  return _dimensions;  }
         virtual const Type *type() const {  return Array_t::instance(E::instance());  }
         virtual V *ptr() const {  return (V*)(_v.get()?_v->ptr():0);  }
         virtual size_t size() const {  return _num_elements;  }
         virtual std::shared_ptr<Alloc> v() const {  return _v;  }
         virtual ~Array() throw(JuliaException) { if(_v.get()) _v->free(); }
   };
};

#endif
