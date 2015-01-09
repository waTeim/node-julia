#include <string.h>
#include "NativeArray.h"

using namespace std;

nj::NativeArrayType nj::toType(const string &name)
{
   if(name == "Float64Array") return NativeArrayType::Float64Array;
   else if(name == "Int32Array") return NativeArrayType::Int32Array;
   else if(name == "Float32Array") return NativeArrayType::Float32Array;
   else if(name == "Uint32Array") return NativeArrayType::Uint32Array;
   else if(name == "Uint8Array") return NativeArrayType::Uint8Array;
   else if(name == "Int8Array") return NativeArrayType::Int8Array;
   else if(name == "Int16Array") return NativeArrayType::Int16Array;
   else if(name == "Uint16Array") return NativeArrayType::Uint16Array;
   return NativeArrayType::none;
}
