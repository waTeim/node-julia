#ifndef __nj_Result
#define __nj_Result

#include <vector>
#include "Value.h"
#include "Exception.h"

namespace nj
{

   class Result
   {
      protected:

         std::shared_ptr<nj::Exception> _ex;
         std::vector<std::shared_ptr<nj::Value>> _results;
     
      public:

         Result(){}
         Result(std::shared_ptr<nj::Exception> &ex) { _ex = ex; }
         Result(const std::vector<std::shared_ptr<nj::Value>> &results) { _results = results; }
         const std::vector<std::shared_ptr<nj::Value>> &results() const { return _results; }
         std::string exText() const { return _ex.get()?_ex->what():""; }
         int exId() const { return _ex.get()?_ex->id():Exception::no_exception; }
   };
};


#endif
