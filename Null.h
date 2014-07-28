#ifndef __nj_Null
#define __nj_Null

#include "Value.h"

namespace nj
{
   class Null:public Value
   {
      public:

         Null(){}
         virtual int getTypeId() {  return null_type;  }
         virtual std::string toString() throw(InvalidException) {  throw(InvalidException("taking value of null"");  }
         virtual int64_t toInt()  throw(InvalidException)  {  throw(InvalidException("taking value of null"");  }
         virtual double toFloat()  throw(InvalidException) {  throw(InvalidException("taking value of null"");  }
         virtual ~Null() {}
   };
};

#endif
