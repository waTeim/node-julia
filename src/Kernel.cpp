#include <iostream>
#include "Kernel.h"
#include "error.h"

using namespace std;

static const string libDir(NJ_LIB);

nj::Kernel *nj::Kernel::singleton = 0;

jl_module_t *nj::Kernel::load() throw(JuliaException)
{
   string njPath = libDir + "/nj.jl";
   jl_function_t *func = jl_get_function(jl_core_module,"include");
   jl_value_t *jl_ex = 0;

   if(!func) throw getJuliaException("unable to locate Core.include");
   jl_call1(func,jl_cstr_to_string(njPath.c_str()));
   jl_ex = jl_exception_occurred();
   if(jl_ex) throw getJuliaException(jl_ex);

   jl_sym_t *modName = jl_symbol("nj");
   jl_module_t *mod = (jl_module_t*)jl_get_global(jl_main_module,modName);

   jl_ex = jl_exception_occurred();
   if(jl_ex) throw getJuliaException(jl_ex);
   
   return mod;
}

nj::Kernel *nj::Kernel::getSingleton()
{
   if(!singleton) singleton = new Kernel();
   return singleton;
}

jl_value_t *nj::Kernel::scriptify(jl_module_t *isolatingMod,jl_value_t *filenameToInclude) throw(JuliaException)
{
   if(!njModule) njModule = load();

   jl_function_t *func = jl_get_function(njModule,"scriptify");

   if(!func) throw getJuliaException("Could not locate function scriptify");
    
   JL_GC_PUSH2(&isolatingMod,&filenameToInclude);

   jl_value_t *ast = jl_call2(func,(jl_value_t*)isolatingMod,filenameToInclude);
   jl_value_t *ex = jl_exception_occurred();

   JL_GC_POP();

   if(ex) throw getJuliaException(ex);
   return ast;
}

jl_value_t *nj::Kernel::newRegex(jl_value_t *pattern) throw(JuliaException)
{
   if(!njModule) njModule = load();

   jl_function_t *func = jl_get_function(njModule,"newRegex");

   if(!func) throw getJuliaException("Could not locate function newRegex");

   JL_GC_PUSH1(&pattern);

   jl_value_t *re = jl_call1(func,pattern);
   jl_value_t *ex = jl_exception_occurred();

   JL_GC_POP();

   if(ex) throw getJuliaException(ex);
   return re;
}


jl_value_t *nj::Kernel::getPattern(jl_value_t *re) throw(JuliaException)
{
   if(!njModule) njModule = load();

   jl_function_t *func = jl_get_function(njModule,"getPattern");

   if(!func) throw getJuliaException("Could not locate function getPattern");

   JL_GC_PUSH1(&re);

   jl_value_t *pattern = jl_call1(func,re);
   jl_value_t *ex = jl_exception_occurred();

   JL_GC_POP();

   if(ex) throw getJuliaException(ex);
   return pattern;
}

