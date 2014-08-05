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
         virtual char toChar() const throw(InvalidException)  {  throw InvalidException("is null");  }
         virtual int64_t toInt() const throw(InvalidException)  {  throw InvalidException("is null");  }
         virtual uint64_t toUInt() const throw(InvalidException)  {  throw InvalidException("is null");  }
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
         virtual char toChar() const throw(InvalidException) {  return b?1:0;  }
         virtual int64_t toInt() const throw(InvalidException) {  return b?1:0;  }
         virtual uint64_t toUInt() const throw(InvalidException) {  return b?1:0;  }
         virtual double toFloat() const throw(InvalidException) {  return b?1:0;  }
         virtual std::string toString() const throw(InvalidException) {  return b?"true":"false";  }
         virtual ~Boolean() {}
   };

   class Char:public Primitive
   {
      protected:

         char c;

      public:

         Char(char c) {  this->c = c;  }
         virtual const Type *type() const {  return Char_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return c != 0;  }
         virtual char toChar() const throw(InvalidException) {  return c;  }
         virtual int64_t toInt() const throw(InvalidException) {  return c;  }
         virtual uint64_t toUInt() const throw(InvalidException) {  return c;  }
         virtual double toFloat() const throw(InvalidException) {  return c;  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(c);  }
         virtual ~Char() {}
   }; 

   class UChar:public Primitive
   {
      protected:

         unsigned char c;

      public:

         UChar(unsigned int c) {  this->c = c;  }
         virtual const Type *type() const {  return UChar_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return c != 0;  }
         virtual char toChar() const throw(InvalidException) {  return c;  }
         virtual int64_t toInt() const throw(InvalidException) {  return c;  }
         virtual uint64_t toUInt() const throw(InvalidException) {  return c;  }
         virtual double toFloat() const throw(InvalidException) {  return c;  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(c);  }
         virtual ~UChar() {}
   }; 

   class Int64:public Primitive
   {
      protected:

         int64_t i;

      public:

         Int64(int64_t i) {  this->i = i;  }
         virtual const Type *type() const {  return Int64_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return i != 0;  }
         virtual char toChar() const throw(InvalidException) {  return i;  }
         virtual int64_t toInt() const throw(InvalidException) {  return i;  }
         virtual uint64_t toUInt() const throw(InvalidException) {  return i;  }
         virtual double toFloat() const throw(InvalidException) {  return i;  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(i);  }
         virtual ~Int64() {}
   }; 

   class UInt64:public Primitive
   {
      protected:

         uint64_t i;

      public:

         UInt64(uint64_t i) {  this->i = i;  }
         virtual const Type *type() const {  return UInt64_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return i != 0;  }
         virtual char toChar() const throw(InvalidException) {  return i;  }
         virtual int64_t toInt() const throw(InvalidException) {  return i;  }
         virtual uint64_t toUInt() const throw(InvalidException) {  return i;  }
         virtual double toFloat() const throw(InvalidException) {  return i;  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(i);  }
         virtual ~UInt64() {}
   }; 

   class Int32:public Primitive
   {
      protected:

         int i;

      public:

         Int32(int i) {  this->i = i;  }
         virtual const Type *type() const {  return Int32_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return i != 0;  }
         virtual char toChar() const throw(InvalidException) {  return i;  }
         virtual int64_t toInt() const throw(InvalidException) {  return i;  }
         virtual uint64_t toUInt() const throw(InvalidException) {  return i;  }
         virtual double toFloat() const throw(InvalidException) {  return i;  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(i);  }
         virtual ~Int32() {}
   }; 

   class UInt32:public Primitive
   {
      protected:

         unsigned int i;

      public:

         UInt32(unsigned int i) {  this->i = i;  }
         virtual const Type *type() const {  return UInt32_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return i != 0;  }
         virtual char toChar() const throw(InvalidException) {  return i;  }
         virtual int64_t toInt() const throw(InvalidException) {  return i;  }
         virtual uint64_t toUInt() const throw(InvalidException) {  return i;  }
         virtual double toFloat() const throw(InvalidException) {  return i;  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(i);  }
         virtual ~UInt32() {}
   }; 

   class Int16:public Primitive
   {
      protected:

         short s;

      public:

         Int16(short s) {  this->s = s;  }
         virtual const Type *type() const {  return Int16_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return s != 0;  }
         virtual char toChar() const throw(InvalidException) {  return s;  }
         virtual int64_t toInt() const throw(InvalidException) {  return s;  }
         virtual uint64_t toUInt() const throw(InvalidException) {  return s;  }
         virtual double toFloat() const throw(InvalidException) {  return s;  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(s);  }
         virtual ~Int16() {}
   }; 

   class UInt16:public Primitive
   {
      protected:

         unsigned short s;

      public:

         UInt16(unsigned int s) {  this->s = s;  }
         virtual const Type *type() const {  return UInt16_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return s != 0;  }
         virtual char toChar() const throw(InvalidException) {  return s;  }
         virtual int64_t toInt() const throw(InvalidException) {  return s;  }
         virtual uint64_t toUInt() const throw(InvalidException) {  return s;  }
         virtual double toFloat() const throw(InvalidException) {  return s;  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(s);  }
         virtual ~UInt16() {}
   }; 

   class Float64:public Primitive
   {
      protected:
   
         double d;
   
      public:

         Float64(double d) {  this->d = d;  }
         virtual const Type *type() const {  return Float64_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return d != 0;  }
         virtual char toChar() const throw(InvalidException)  {  throw InvalidException("is float");  }
         virtual int64_t toInt() const throw(InvalidException) {  return d;  }
         virtual uint64_t toUInt() const throw(InvalidException) {  return d;  }
         virtual double toFloat() const throw(InvalidException) {  return d;  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(d);  }
         virtual ~Float64() {}
   };    

   class Float32:public Primitive
   {
      protected:
   
         float f;
   
      public:

         Float32(float f) {  this->f = f;  }
         virtual const Type *type() const {  return Float32_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException) {  return f != 0;  }
         virtual char toChar() const throw(InvalidException)  {  throw InvalidException("is float");  }
         virtual int64_t toInt() const throw(InvalidException) {  return f;  }
         virtual uint64_t toUInt() const throw(InvalidException) {  return f;  }
         virtual double toFloat() const throw(InvalidException) {  return f;  }
         virtual std::string toString() const throw(InvalidException) {  return std::to_string(f);  }
         virtual ~Float32() {}
   };    

   class String:public Primitive
   {
      protected:
    
         std::string s;

      public:

         String(const std::string &s) {  this->s = s;  }
         virtual const Type *type() const {  return String_t::instance();  }
         virtual bool toBoolean() const throw(InvalidException);
         virtual char toChar() const throw(InvalidException);
         virtual int64_t toInt() const throw(InvalidException);
         virtual uint64_t toUInt() const throw(InvalidException);
         virtual double toFloat() const throw(InvalidException);
         virtual std::string toString() const throw(InvalidException) {  return s;  }
         virtual ~String() {}
   };
};

#endif
