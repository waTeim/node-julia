#if defined(WIN32)
#pragma warning(disable:4200)
#endif

#include <iostream>
#include <julia.h>
#include "Import.h"
#include "Kernel.h"
#include "error.h"
#include "lvalue.h"
#include "JuliaHandle.h"

using namespace std;

nj::Result nj::Import::eval(vector<shared_ptr<nj::Value>> &args,int64_t exprId)
{
   vector<shared_ptr<Value>> res;

   if(args.size() != 1 || !args[0]->isPrimitive()) return Result(res,exprId);

   try
   {
      Primitive &text = static_cast<Primitive&>(*args[0]);
      string modulePath = text.toString();
      Kernel *kernel = Kernel::getSingleton();
      jl_value_t *val = kernel->import(modulePath);
      vector<shared_ptr<Value>> unfiltered = lvalue(val);
      JuliaHandle &handle = static_cast<JuliaHandle&>(*unfiltered[0]);
      jl_module_t *module = (jl_module_t*)handle.val();
      Array<string,ASCIIString_t> &unfilteredNames = static_cast<Array<string,ASCIIString_t>&>(*unfiltered[1]);
      vector<string> filteredNames;

      res.push_back(unfiltered[0]);
      for(size_t i = 0;i < unfilteredNames.dims()[0];i++)
      {
         string *s = unfilteredNames.ptr() + i;
         jl_value_t *val = jl_get_global(module,jl_symbol(s->c_str()));

         if(val && jl_is_function(val)) filteredNames.push_back(*s);
      }

      vector<size_t> dims;

      dims.push_back(filteredNames.size());

      Array<string,ASCIIString_t> *res1 = new Array<string,ASCIIString_t>(dims);
      string *p = res1->ptr();

      for(size_t i = 0;i < dims[0];i++) *p++ = filteredNames[i];
      res.push_back(shared_ptr<Value>(res1));
      
      return Result(res,exprId);
   }
   catch(JuliaException e)
   {
     return Result(e.err,exprId);
   }
}
