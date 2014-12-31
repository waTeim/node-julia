#include <julia.h>
#include <iostream>
#include "Kernel.h"
#include "Script.h"
#include "JuliaHandle.h"
#include "error.h"

using namespace std;

static nj::Result exceptionResult(jl_value_t *jl_ex,int64_t exprId)
{
   JL_GC_PUSH1(&jl_ex);

   shared_ptr<nj::Exception> ex = nj::genJuliaError(jl_ex);

   JL_GC_POP();
   return nj::Result(ex,exprId);
}
 
static nj::Result loadErrorResult(string msg,int64_t exprId)
{
   shared_ptr<nj::Exception> ex = shared_ptr<nj::Exception>(new nj::JuliaLoadError(msg));

   return nj::Result(ex,exprId);
}

nj::Result nj::Script::eval(vector<shared_ptr<nj::Value>> &args,int64_t exprId)
{
   vector<shared_ptr<nj::Value>> res;
   static int modNum = 0;

   if(args.size() != 1 || !args[0]->isPrimitive()) return Result(res,exprId);

   Primitive &text = static_cast<Primitive&>(*args[0]);

   jl_value_t *jl_ex = 0;
   Kernel *kernel = Kernel::getSingleton();

   try
   {
      string isolatingModName = string("njIsoMod") + to_string(modNum++);
      jl_value_t *filenameToInclude = jl_cstr_to_string(text.toString().c_str());
      jl_sym_t *isolatingModSym = jl_symbol(isolatingModName.c_str());
      jl_module_t *isolatingMod = jl_new_module(isolatingModSym);

      jl_ex = jl_exception_occurred();
      if(jl_ex) return exceptionResult(jl_ex,exprId);

      (void)jl_add_standard_imports(isolatingMod);

      jl_ex = jl_exception_occurred();
      if(jl_ex) return exceptionResult(jl_ex,exprId);

      jl_value_t *ast = kernel->scriptify(isolatingMod,filenameToInclude);
      jl_function_t *func = jl_get_function(jl_core_module,"eval");

      if(!func) return loadErrorResult("unable to locate Core.eval",exprId);

      (void)jl_call2(func,(jl_value_t*)isolatingMod,ast);
      jl_ex = jl_exception_occurred();
      if(jl_ex) return exceptionResult(jl_ex,exprId);

      res.push_back(shared_ptr<nj::Value>(new nj::ASCIIString(isolatingModName)));
      res.push_back(shared_ptr<nj::Value>(new nj::JuliaHandle((jl_value_t*)isolatingMod)));

      return Result(res,exprId);
   }
   catch(JuliaException je)
   {
      return nj::Result(je.err,exprId);
   }
}
