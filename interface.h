#ifndef __nj_interface
#define __nj_interface

#include <string>

namespace nj
{
   class Expr
   {
      protected:

         std::string text;

      public:

         Expr(std::string text) {  this->text = text;  }

         std::string getText() const {  return text;  }
   };
};

#endif
