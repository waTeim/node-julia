#include <iostream>
#include <julia.h>
#include <sstream>
#include "error.h"
#include "debug.h"

using namespace std;

static string getMethodErrorText(jl_value_t *ex)
{
   stringstream ss;
   jl_value_t *f0 = jl_get_nth_field(ex,0);
   jl_value_t *f1 = jl_get_nth_field(ex,1);
   size_t numArgs = jl_tuple_len(f1);

   ss << "Unmatched method: " << jl_gf_name(f0)->name << "(";

   for(size_t i = 0;i < numArgs;i++) 
   {
      jl_value_t *ti = jl_tupleref(f1,i);
      jl_datatype_t *t = (jl_datatype_t*)jl_typeof(ti);

      if(i != 0) ss << ",";
      ss << t->name->name->name;
   }

   ss << ")";

   return ss.str();
}

string nj::getErrorText(jl_value_t *ex)
{
   stringstream ss;

   if(jl_typeis(ex,jl_methoderror_type)) return getMethodErrorText(ex);
   else
   {
      if(jl_typeis(ex,jl_errorexception_type)) ss << "ex is jl_errorexception_type";
      else if(jl_typeis(ex,jl_loaderror_type)) ss << "ex is jl_loaderror_type";
      else if(jl_typeis(ex,jl_typeerror_type)) ss << "ex is jl_typeerror_type";
      else if(jl_typeis(ex,jl_undefvarerror_type)) ss << "ex is jl_undefvarerror_type";
 
      return ss.str();
   }
}
