#ifndef __nj_Values
#define __nj_Values

#include "Value.h"
#include "Types.h"

namespace nj
{
   class Null:public Primitive
   {
      public:

         Null(){}
         virtual const Type *type() const {  return Null_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException)  {  throw InvalidException("is null");  }
         virtual int64_t toInt() const throw(InvalidException)  {  throw InvalidException("is null");  }
         virtual double toFloat() const throw(InvalidException) {  throw InvalidException("is null");  }
         virtual std::string toString() const throw(InvalidException) {  throw InvalidException("is null");  }
         virtual ~Null() {}
   };

   class Boolean:public Primitive
   {
      protected:

         bool b;

      public:

         Boolean(bool b) {  this->b = b;  }
         virtual const Type *type() const {  return Boolean_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return b;  }
         virtual int64_t toInt() const throw(InvalidException) {  return b?1:0;  }
         virtual double toFloat() const throw(InvalidException) {  return b?1:0;  }
         virtual std::string toString() const throw(InvalidException) {  return b?"true":"false";  }
         virtual ~Boolean() {}
   };

   class Int:public Primitive
   {
      protected:

         int64_t i;

      public:

         Int(int64_t i) {  this->i = i;  }
         virtual const Type *type() const {  return Int_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return i != 0;  }
         virtual int64_t toInt() const throw(InvalidException) {  return i;  }
         virtual double toFloat() const throw(InvalidException) {  return i;  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(i);  }
         virtual ~Int() {}
   }; 

   class Float:public Primitive
   {
      protected:
   
         double d;
   
      public:

         Float(double d) {  this->d = d;  }
         virtual const Type *type() const {  return Float_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return d != 0;  }
         virtual int64_t toInt() const throw(InvalidException) {  return d;  }
         virtual double toFloat() const throw(InvalidException) {  return d;  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(d);  }
         virtual ~Float() {}
   };    

   class String:public Primitive
   {
      protected:
    
         std::string s;

      public:

         String(const std::string &s) {  this->s = s;  }
         virtual const Type *type() const {  return String_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException);
         virtual int64_t toInt() const throw(InvalidException);
         virtual double toFloat() const throw(InvalidException);
         virtual std::string toString() const throw(InvalidException) {  return s;  }
         virtual ~String() {}
   };
};

#endif
