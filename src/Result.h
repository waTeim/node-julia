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
         int64_t expr_id;
     
      public:

         Result() { expr_id = -1; }
         Result(std::shared_ptr<nj::Exception> &ex,int64_t exprId) { _ex = ex; expr_id = exprId; }
         Result(const std::vector<std::shared_ptr<nj::Value>> &results,int64_t exprId) { _results = results; expr_id = exprId; }
         const std::vector<std::shared_ptr<nj::Value>> &results() const { return _results; }
         std::string exceptionText() const { return _ex.get()?_ex->what():""; }
         int exceptionId() const { return _ex.get()?_ex->id():Exception::no_exception; }
         int exprId() const { return expr_id; }
   };
};


#endif
