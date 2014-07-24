#ifndef __Exception
#define __Exception

#include <string>

namespace nj
{
   class Exception
   {
      protected:

         std::string _what;

      public:

         Exception() {}
         Exception(const std::string &what):_what(what) {}

         std::string what() const {  return _what;  }
   };

   class SystemException:public Exception {  public: SystemException(const std::string &what);  };
   class InitializationException:public Exception {  public: InitializationException(std::string what):Exception(what) {} };
}

#endif

