#include <iostream>
#include <sstream>
#include <julia.h>
#include "Call.h"
#include "JuliaHandle.h"
#include "error.h"
#include "rvalue.h"
#include "lvalue.h"
#include "util.h"

using namespace std;

jl_function_t *findFunction(jl_module_t *start,const vector<string> &funcNamePath)
{
   jl_module_t *mod = 0;
   jl_function_t *func = 0;

   if(funcNamePath.size() > 1)
   {
      jl_sym_t *sym = jl_symbol(funcNamePath[0].c_str());

      if(!start)
      {
         mod = (jl_module_t*)jl_get_global(jl_core_module,sym);
         if(mod && !jl_is_module(mod)) mod = 0;
         if(!mod) mod = (jl_module_t*)jl_get_global(jl_base_module,sym);
         if(mod && !jl_is_module(mod)) mod = 0;
         if(!mod) mod = (jl_module_t*)jl_get_global(jl_main_module,sym);
      }
      else mod = (jl_module_t*)jl_get_global(start,sym);
      if(mod && !jl_is_module(mod)) mod = 0;
      if(mod)
      {
         for(size_t i = 1;mod && i < funcNamePath.size() - 1;i++)
         {
            sym = jl_symbol(funcNamePath[i].c_str());
            mod  = (jl_module_t*)jl_get_global(mod,sym);
            if(mod && !jl_is_module(mod)) mod = 0;
         }
         if(mod) func = jl_get_function(mod,funcNamePath[funcNamePath.size() - 1].c_str());
      }
   }
   else
   {
      if(!start)
      {
         func = jl_get_function(jl_core_module,funcNamePath[0].c_str());
         if(!func) func = jl_get_function(jl_base_module,funcNamePath[0].c_str());
         if(!func) func = jl_get_function(jl_main_module,funcNamePath[0].c_str());
      }
      else func = jl_get_function(start,funcNamePath[0].c_str());
   }
   return func;
}

string getFuncName(const vector<string> &funcNamePath)
{
   stringstream ss;

   ss << funcNamePath[0];
   for(size_t i = 1;i < funcNamePath.size();i++) ss << "." << funcNamePath[i];
   return ss.str();
}

nj::Result nj::Call::eval(vector<shared_ptr<nj::Value>> &args,int64_t exprId)
{
   vector<shared_ptr<nj::Value>> res;
   jl_module_t *start = 0;
   int numArgs;
   int argOffset;
   vector<string> funcNamePath;

   if(args.size() == 0) return Result(res,exprId);

   switch(args[0]->type()->id())
   {
      case ascii_string_type:
      case utf8_string_type:
      {
         Primitive &id = static_cast<Primitive&>(*args[0]);

         funcNamePath = split(id.toString(),'.');
         numArgs = args.size() - 1;
         argOffset = 1;
      }
      break;
      case julia_handle_type:
      {
         if(args.size() < 2 || !args[1]->isPrimitive()) return Result(res,exprId);

         JuliaHandle &mod_h = static_cast<JuliaHandle&>(*args[0]);
         Primitive &id = static_cast<Primitive&>(*args[1]);

         start = (jl_module_t*)mod_h.val();
         funcNamePath = split(id.toString(),'.');
         numArgs = args.size() - 2;
         argOffset = 2;
      }
      break;
      default: return Result(res,exprId);
   }

   jl_value_t *jl_res = 0;
   jl_function_t *func = findFunction(start,funcNamePath);

   if(!func)
   {
      shared_ptr<Exception> ex = shared_ptr<Exception>(new JuliaMethodError(string("Julia method ") + getFuncName(funcNamePath) + " is undefined"));

      return Result(ex,exprId);
   }

   bool rvalue_error = false;

   try
   {
      if(numArgs <= 3)
      {
         switch(numArgs)
         {
            case 0: jl_res = jl_call0(func); break;
            case 1:
            {
               jl_value_t *arg1 = rvalue(args[argOffset]);

               if(!arg1) rvalue_error = true;
               else jl_res = jl_call1(func,arg1);
            }
            break;
            case 2:
            {
	            jl_value_t *arg1 = rvalue(args[argOffset]);
               jl_value_t *arg2 = rvalue(args[argOffset + 1]);

               if(!arg1 || !arg2) rvalue_error = true;
               else jl_res = jl_call2(func,arg1,arg2);
            }
            break;
            case 3:
            {
               jl_value_t *arg1 = rvalue(args[argOffset]);
               jl_value_t *arg2 = rvalue(args[argOffset + 1]);
               jl_value_t *arg3 = rvalue(args[argOffset + 2]);

               if(!arg1 || !arg2 || !arg3) rvalue_error = true;
               else jl_res = jl_call3(func,arg1,arg2,arg3);
            }
            break;
         }
      }
      else
      {
         jl_value_t **jl_args = new jl_value_t*[numArgs];

         for(int i = 0;i < numArgs && !rvalue_error;i++)
         {
            jl_args[i] = rvalue(args[i + argOffset]);
            if(!jl_args[i]) rvalue_error = true;
         }
         if(!rvalue_error) jl_res = jl_call(func,jl_args,numArgs);
         delete jl_args;
      }

      if(rvalue_error)
      {
         shared_ptr<Exception> ex = shared_ptr<Exception>(new InvalidException("can not convert RHS to a Julia value"));

         return Result(ex,exprId);
      }
   }
   catch(JuliaException e)
   {
      return Result(e.err,exprId);
   }

   jl_value_t *jl_ex = jl_exception_occurred();

   if(jl_ex)
   {
     JL_GC_PUSH1(&jl_ex);

     shared_ptr<Exception> ex = genJuliaError(jl_ex);

     JL_GC_POP();
     return Result(ex,exprId);
   }
   else
   {
      try
      {
         JL_GC_PUSH1(&jl_res);
         res = lvalue(jl_res);
         JL_GC_POP();
         return Result(res,exprId);
      }
      catch(JuliaException e)
      {
        JL_GC_POP();
        return Result(e.err,exprId);
      }
   }
}
