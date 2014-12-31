#ifndef __nj_Binding
#define __nj_Binding

#include <memory>

namespace nj
{
   class Result;
   class Callback;

   class Binding
   {
      protected:

         std::shared_ptr<Result> _res;
         std::shared_ptr<Callback> _target;

      public:
     
         Binding(const std::shared_ptr<Result> &res,const std::shared_ptr<Callback> &target):_res(res),_target(target) {}
         std::shared_ptr<Result> res() { return _res; }
         std::shared_ptr<Callback> target() { return _target; }
         ~Binding() {}
   };
};

#endif
