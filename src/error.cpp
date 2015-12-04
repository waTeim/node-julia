#if defined(WIN32)
#pragma warning(disable:4200)
#endif

#include <map>
#include <iostream>
#include <julia.h>
#include <sstream>
#include "error.h"
#include "debug.h"
#include "Kernel.h"
#include "util.h"
#include "juliav.h"

extern "C"
{
#if defined(DLLEXPORT)
DLLEXPORT jl_value_t *jl_get_backtrace(void);
#elif defined(JL_DLLEXPORT)
JL_DLLEXPORT jl_value_t *jl_get_backtrace(void);
#endif
};

using namespace std;

static map<string,jl_value_t*> etypes;
static bool initialized = false;

static string errorMsg(jl_value_t *ex);

static jl_value_t *baseSymbol(const string &name)
{
   return jl_get_global(jl_base_module,jl_symbol(name.c_str()));
}

static string errorExceptionMsg(jl_value_t *ex)
{
   jl_value_t *f0 = jl_get_nth_field(ex,0);
   stringstream ss;

   ss << jl_iostr_data(f0);

   return ss.str();
}

static shared_ptr<nj::Exception> genJuliaErrorException(jl_value_t *ex)
{
   stringstream ss;

   ss << "Julia " << errorExceptionMsg(ex);

   return shared_ptr<nj::Exception>(new nj::JuliaErrorException(ss.str()));
}

static string methodErrorMsg(jl_value_t *ex)
{
   stringstream ss;
   jl_value_t *f0 = jl_get_nth_field(ex,0);
   jl_value_t *f1 = jl_get_nth_field(ex,1);

   #if defined(USES_SVEC)
   size_t numArgs = jl_nfields(f1);
   #else
   size_t numArgs = jl_tuple_len(f1);
   #endif

   #if defined(jl_symbol_name)
   ss << "unmatched method " << jl_symbol_name(jl_gf_name(f0)) << "(";
   #else
   ss << "unmatched method " << jl_gf_name(f0)->name << "(";
   #endif

   for(size_t i = 0;i < numArgs;i++)
   {

#if defined(USES_SVEC)
      jl_value_t *arg = jl_fieldref(f1,i);
#else
      jl_value_t *arg = jl_tupleref(f1,i);
#endif

      if(i != 0) ss << ",";
      ss << jl_typename_str(jl_typeof(arg));
   }

   ss << ")";

   return ss.str();
}

static shared_ptr<nj::Exception> genJuliaMethodError(jl_value_t *ex)
{
   stringstream ss;

   ss << "Julia " << methodErrorMsg(ex);

   return shared_ptr<nj::Exception>(new nj::JuliaMethodError(ss.str()));
}

static string undefVarErrorMsg(jl_value_t *ex)
{
   jl_value_t *f0 = jl_get_nth_field(ex,0);
   stringstream ss;

   #if defined(jl_symbol_name)
   ss << "undefined variable " << jl_symbol_name((jl_sym_t*)f0);
   #else
   ss << "undefined variable " << ((jl_sym_t*)f0)->name;
   #endif

   return ss.str();
}

static shared_ptr<nj::Exception> genJuliaUndefVarError(jl_value_t *ex)
{
   stringstream ss;

   ss << "Julia " << undefVarErrorMsg(ex);

   return shared_ptr<nj::Exception>(new nj::JuliaUndefVarError(ss.str()));
}

static string loadErrorMsg(jl_value_t *ex)
{
   jl_value_t *f0 = jl_get_nth_field(ex,0);
   jl_value_t *f1 = jl_get_nth_field(ex,1);
   jl_value_t *f2 = jl_get_nth_field(ex,2);
   stringstream ss;

   ss << "loading " << jl_iostr_data(f0) << " line " << jl_unbox_int64(f1) <<  " " << errorMsg(f2);

   return ss.str();
}

static shared_ptr<nj::Exception> genJuliaLoadError(jl_value_t *ex)
{
   return shared_ptr<nj::Exception>(new nj::JuliaLoadError(loadErrorMsg(ex)));
}

static string systemErrorMsg(jl_value_t *ex)
{
   jl_value_t *f0 = jl_get_nth_field(ex,0);
   jl_value_t *f1 = jl_get_nth_field(ex,1);
   stringstream ss;

   ss << jl_iostr_data(f0) << " errno = " << jl_unbox_int64(f1);

   return ss.str();
}

static shared_ptr<nj::Exception> genJuliaSystemError(jl_value_t *ex)
{
   return shared_ptr<nj::Exception>(new nj::JuliaErrorException(systemErrorMsg(ex)));
}

static string errorMsg(jl_value_t *ex)
{
   if(jl_typeis(ex,jl_errorexception_type)) return errorExceptionMsg(ex);
   else if(jl_typeis(ex,jl_methoderror_type)) return methodErrorMsg(ex);
   else if(jl_typeis(ex,jl_undefvarerror_type)) return undefVarErrorMsg(ex);
   else if(jl_typeis(ex,jl_loaderror_type)) return loadErrorMsg(ex);
   else return jl_typeof_str(ex);
}

static void initializeETypes()
{
   etypes["SystemError"] = baseSymbol("SystemError");
   initialized = true;
}

shared_ptr<nj::Exception> nj::genJuliaError(jl_value_t *ex)
{
   stringstream ss;
   shared_ptr<nj::Exception> njEx;

   if(!initialized) initializeETypes();

   try
   {
      jl_value_t *bt = jl_get_backtrace();
      nj::Kernel *kernel = nj::Kernel::getSingleton();
      jl_value_t *stack = kernel->getError(ex,bt);
      vector<string> trace = split(jl_string_data(stack),'\n');

      if(jl_typeis(ex,jl_errorexception_type)) njEx = genJuliaErrorException(ex);
      else if(jl_typeis(ex,jl_methoderror_type)) njEx = genJuliaMethodError(ex);
      else if(jl_typeis(ex,jl_undefvarerror_type)) njEx = genJuliaUndefVarError(ex);
      else if(jl_typeis(ex,jl_loaderror_type)) njEx = genJuliaLoadError(ex);
      else if(jl_typeis(ex,etypes["SystemError"])) njEx = genJuliaSystemError(ex);
      else if(jl_is_ascii_string(ex) || jl_is_utf8_string(ex)) ss << jl_string_data(ex);
      else njEx = shared_ptr<Exception>(new InvalidException(trace[0]));

      for(size_t i = 1;i < trace.size();i++)
      {
         if(trace[i].substr(0,3) == " in") njEx->push(trace[i].substr(4));
      }
   }
   catch(JuliaException e)
   {
      njEx = e.err;
   }
   return njEx;
}


nj::JuliaException nj::getJuliaException(jl_value_t *jl_ex)
{
   JL_GC_PUSH1(&jl_ex);

   shared_ptr<nj::Exception> ex = nj::genJuliaError(jl_ex);

   JL_GC_POP();
   return JuliaException(ex);
}

nj::JuliaException nj::getJuliaException(const string &msg)
{
   return JuliaException(shared_ptr<nj::Exception>(new nj::JuliaErrorException(msg)));
}
