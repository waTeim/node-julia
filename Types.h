#ifndef __nj_Types
#define __nj_Types

#include "Value.h"

namespace nj
{
   class Null:public Value
   {
      public:

         Null(){}
         virtual int getTypeId() {  return null_type;  }
         virtual bool toBoolean() throw(InvalidException)  {  throw InvalidException("taking value of null");  }
         virtual int64_t toInt() throw(InvalidException)  {  throw InvalidException("taking value of null");  }
         virtual double toFloat() throw(InvalidException) {  throw InvalidException("taking value of null");  }
         virtual std::string toString() throw(InvalidException) {  throw InvalidException("taking value of null");  }
         virtual ~Null() {}
   };

   class Boolean:public Value
   {
      protected:

         VTraits<bool,boolean_type> traits;

      public:

         Boolean(bool b):traits(b) {}
         virtual int getTypeId() {  return traits.getTypeId();  }
         virtual bool toBoolean() throw(InvalidException) {  return traits.getValue();  }
         virtual int64_t toInt() throw(InvalidException) {  return traits.getValue()?1:0;  }
         virtual double toFloat() throw(InvalidException) {  return traits.getValue()?1:0;  }
         virtual std::string toString() throw(InvalidException) {  return traits.getValue()?"true":"false";  }
         virtual ~Boolean() {}
   };

   class Int:public Value
   {
      protected:

         VTraits<int64_t,int_type> traits;

      public:

         Int(int64_t i):traits(i) {}
         virtual int getTypeId() {  return traits.getTypeId();  }
         virtual bool toBoolean() throw(InvalidException) {  return traits.getValue() != 0;  }
         virtual int64_t toInt() throw(InvalidException) {  return traits.getValue();  }
         virtual double toFloat() throw(InvalidException) {  return traits.getValue();  }
         virtual std::string toString() throw(InvalidException) {  return std::to_string(traits.getValue());  }
         virtual ~Int() {}
   }; 

   class Float:public Value
   {
      protected:
   
         VTraits<double,float_type> traits;
   
      public:

         Float(double d):traits(d) {}
         virtual int getTypeId() {  return traits.getTypeId();  }
         virtual bool toBoolean() throw(InvalidException) {  return traits.getValue() != 0;  }
         virtual int64_t toInt() throw(InvalidException) {  return traits.getValue();  }
         virtual double toFloat() throw(InvalidException) {  return traits.getValue();  }
         virtual std::string toString() throw(InvalidException) {  return std::to_string(traits.getValue());  }
         virtual ~Float() {}
   };    

   class String:public Value
   {
      protected:
    
         VTraits<std::string,string_type> traits;

      public:

         String(const std::string &s):traits(s) {}
         virtual int getTypeId() {  return traits.getTypeId();  }
         virtual bool toBoolean() throw(InvalidException);
         virtual int64_t toInt() throw(InvalidException);
         virtual double toFloat() throw(InvalidException);
         virtual std::string toString() throw(InvalidException) {  return traits.getValue();  }
         virtual ~String() {}
   };
};

#endif
