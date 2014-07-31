#ifndef __nj_Types
#define __nj_Types

#include "Value.h"

namespace nj
{
   class Null:public Primitive
   {
      public:

         Null(){}
         virtual int typeAtom() const {  return null_type;  }
         virtual bool toBoolean() const throw(InvalidException)  {  throw InvalidException("is null");  }
         virtual int64_t toInt() const throw(InvalidException)  {  throw InvalidException("is null");  }
         virtual double toFloat() const throw(InvalidException) {  throw InvalidException("is null");  }
         virtual std::string toString() const throw(InvalidException) {  throw InvalidException("is null");  }
         virtual ~Null() {}
   };

   class Boolean:public Primitive
   {
      protected:

         VTraits<bool,boolean_type> traits;

      public:

         Boolean(bool b):traits(b) {}
         virtual int typeAtom() const {  return traits.getId();  }
         virtual bool toBoolean() const throw(InvalidException) {  return traits.getValue();  }
         virtual int64_t toInt() const throw(InvalidException) {  return traits.getValue()?1:0;  }
         virtual double toFloat() const throw(InvalidException) {  return traits.getValue()?1:0;  }
         virtual std::string toString() const throw(InvalidException) {  return traits.getValue()?"true":"false";  }
         virtual ~Boolean() {}
   };

   class Int:public Primitive
   {
      protected:

         VTraits<int64_t,int_type> traits;

      public:

         Int(int64_t i):traits(i) {}
         virtual int typeAtom() const {  return traits.getId();  }
         virtual bool toBoolean() const throw(InvalidException) {  return traits.getValue() != 0;  }
         virtual int64_t toInt() const throw(InvalidException) {  return traits.getValue();  }
         virtual double toFloat() const throw(InvalidException) {  return traits.getValue();  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(traits.getValue());  }
         virtual ~Int() {}
   }; 

   class Float:public Primitive
   {
      protected:
   
         VTraits<double,float_type> traits;
   
      public:

         Float(double d):traits(d) {}
         virtual int typeAtom() const {  return traits.getId();  }
         virtual bool toBoolean() const throw(InvalidException) {  return traits.getValue() != 0;  }
         virtual int64_t toInt() const throw(InvalidException) {  return traits.getValue();  }
         virtual double toFloat() const throw(InvalidException) {  return traits.getValue();  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(traits.getValue());  }
         virtual ~Float() {}
   };    

   class String:public Primitive
   {
      protected:
    
         VTraits<std::string,string_type> traits;

      public:

         String(const std::string &s):traits(s) {}
         virtual int typeAtom() const {  return traits.getId();  }
         virtual bool toBoolean() const throw(InvalidException);
         virtual int64_t toInt() const throw(InvalidException);
         virtual double toFloat() const throw(InvalidException);
         virtual std::string toString() const throw(InvalidException) {  return traits.getValue();  }
         virtual ~String() {}
   };
};

#endif
