#include <iostream>
#include "Kernel.h"
#include "error.h"

using namespace std;

static const string libDir(NJ_LIB);

nj::Kernel *nj::Kernel::singleton = 0;

jl_value_t *nj::Kernel::get_preserve_array() throw(JuliaException)
{
   if(!nj_module) nj_module = load();

   jl_sym_t *preserveSym = jl_symbol("preserve");
   jl_value_t *preserve = jl_get_global(nj_module,preserveSym);
   jl_value_t *ex = jl_exception_occurred();

   if(ex) throw getJuliaException(ex);
   return preserve;
}

jl_value_t *nj::Kernel::invoke(const string &functionName) throw(JuliaException)
{
   if(!nj_module) nj_module = load();

   jl_function_t *func = jl_get_function(nj_module,functionName.c_str());

   if(!func) throw getJuliaException("Could not locate function nj." + functionName);

   jl_value_t *res = jl_call0(func);
   jl_value_t *ex = jl_exception_occurred();

   if(ex) throw getJuliaException(ex);
   return res;
}

jl_value_t *nj::Kernel::invoke(const string &functionName,jl_value_t *arg) throw(JuliaException)
{
   if(!nj_module) nj_module = load();

   jl_function_t *func = jl_get_function(nj_module,functionName.c_str());

   if(!func) throw getJuliaException("Could not locate function nj." + functionName);

   JL_GC_PUSH1(&arg);

   jl_value_t *res = jl_call1(func,arg);
   jl_value_t *ex = jl_exception_occurred();

   JL_GC_POP();

   if(ex) throw getJuliaException(ex);
   return res;
}

jl_value_t *nj::Kernel::invoke(const string &functionName,jl_value_t *arg1,jl_value_t *arg2) throw(JuliaException)
{
   if(!nj_module) nj_module = load();

   jl_function_t *func = jl_get_function(nj_module,functionName.c_str());

   if(!func) throw getJuliaException("Could not locate function nj." + functionName);

   JL_GC_PUSH2(&arg1,&arg2);

   jl_value_t *res = jl_call2(func,arg1,arg2);
   jl_value_t *ex = jl_exception_occurred();

   JL_GC_POP();

   if(ex) throw getJuliaException(ex);
   return res;
}

jl_module_t *nj::Kernel::load() throw(JuliaException)
{
   string njPath = libDir + "/nj.jl";
   jl_function_t *func = jl_get_function(jl_core_module,"include");

   if(!func) throw getJuliaException("unable to locate Core.include");
   jl_call1(func,jl_cstr_to_string(njPath.c_str()));

   jl_value_t *ex = jl_exception_occurred();

   if(ex) throw getJuliaException(ex);

   jl_sym_t *modName = jl_symbol("nj");
   jl_module_t *mod = (jl_module_t*)jl_get_global(jl_main_module,modName);

   ex = jl_exception_occurred();
   if(ex) throw getJuliaException(ex);

   return mod;
}

nj::Kernel *nj::Kernel::getSingleton()
{
   if(!singleton) singleton = new Kernel();
   return singleton;
}

nj::Kernel::Kernel()
{
   nj_module = 0;
   preserve_array = 0;
   freelist_start = -1;
}


jl_value_t *nj::Kernel::scriptify(jl_module_t *isolatingMod,jl_value_t *filenameToInclude) throw(JuliaException)
{
   if(!nj_module) nj_module = load();

   jl_function_t *func = jl_get_function(nj_module,"scriptify");

   if(!func) throw getJuliaException("Could not locate function nj.scriptify");

   JL_GC_PUSH2(&isolatingMod,&filenameToInclude);

   jl_value_t *ast = jl_call2(func,(jl_value_t*)isolatingMod,filenameToInclude);
   jl_value_t *ex = jl_exception_occurred();

   JL_GC_POP();

   if(ex) throw getJuliaException(ex);
   return ast;
}

jl_value_t *nj::Kernel::newRegex(jl_value_t *pattern) throw(JuliaException)
{
   if(!nj_module) nj_module = load();

   jl_function_t *func = jl_get_function(nj_module,"newRegex");

   if(!func) throw getJuliaException("Could not locate function nj.newRegex");

   JL_GC_PUSH1(&pattern);

   jl_value_t *re = jl_call1(func,pattern);
   jl_value_t *ex = jl_exception_occurred();

   JL_GC_POP();

   if(ex) throw getJuliaException(ex);
   return re;
}


jl_value_t *nj::Kernel::getPattern(jl_value_t *re) throw(JuliaException) { return invoke("getPattern",re); }
jl_datatype_t *nj::Kernel::getDateTimeType() throw(JuliaException) { return (jl_datatype_t*)invoke("getDateTimeType"); }
jl_datatype_t *nj::Kernel::getRegexType() throw(JuliaException) { return (jl_datatype_t*)invoke("getRegexType"); }
jl_value_t *nj::Kernel::getError(jl_value_t *ex,jl_value_t *bt) throw(JuliaException) { return invoke("getError",ex,bt); }

int64_t nj::Kernel::preserve(jl_value_t *val) throw(JuliaException)
{
   int64_t free_index;

   if(!preserve_array) preserve_array = get_preserve_array();

   if(freelist_start == -1)
   {
      jl_cell_1d_push((jl_array_t*)preserve_array,val);
      freelist.push_back(-1);
      free_index = freelist.size() - 1;
   }
   else
   {
      free_index = freelist_start;
      freelist_start = freelist[free_index];
      freelist[free_index] = -1;
      jl_cellset(preserve_array,free_index,val);
   }

   jl_value_t *ex = jl_exception_occurred();

   if(ex) throw getJuliaException(ex);
   return free_index;
}

jl_value_t *nj::Kernel::free(int64_t valIndex) throw(JuliaException)
{
   if(!preserve_array) preserve_array = get_preserve_array();

   freelist[valIndex] = freelist_start;
   freelist_start = valIndex;

   jl_value_t *val = jl_cellref(preserve_array,valIndex);
   jl_value_t *ex = jl_exception_occurred();

   if(ex) throw getJuliaException(ex);
   jl_cellset(preserve_array,valIndex,0);
   ex = jl_exception_occurred();
   if(ex) throw getJuliaException(ex);
   return val;
}

jl_value_t *nj::Kernel::import(const string &moduleName) throw(JuliaException)
{
   if(moduleName.length() != 0)
   {
      jl_value_t *val = jl_cstr_to_string(moduleName.c_str());

      return invoke("importModule",val);
   }
   return 0;
}
