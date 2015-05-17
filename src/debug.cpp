#if defined(WIN32)
#pragma warning(disable:4200)
#endif

#include <sstream>
#include <julia.h>
#include "debug.h"

using namespace std;

string nj::getTypeName(jl_value_t *v)
{
   string res = "none";
   stringstream ss;

   if(v)
   {
     if(jl_is_structtype(v)) ss << "value is struct_type";
     if(jl_is_datatype(v)) ss << "value is type datatype";
     if(jl_is_expr(v)) ss << "value is expr";
     if(jl_is_tuple(v)) ss << "value is tuple";
     if(jl_is_vararg_type(v)) ss << "value is vararg";
     if(jl_is_func(v)) ss << "value is func";
     if(jl_is_byte_string(v)) ss << "value is string";
     if(jl_is_uniontype(v)) ss << "value is union";
     if(jl_is_typector(v)) ss << "value is ctor";
     if(jl_is_symbol(v)) ss << "value is jl_is_symbol";
     if(jl_is_lambda_info(v)) ss << "value is jl_is_lambda_info";
     if(jl_is_vararg_type(v)) ss << "value is jl_is_vararg_type";
     //if(jl_is_getfieldnode(v)) ss << "value is jl_is_getfieldnode";
     if(jl_is_datatype(jl_typeof(v))) ss << "value is a data type";

     string tmp = ss.str();
   
     if(tmp != "") res = tmp;
   }

   return res;
}
