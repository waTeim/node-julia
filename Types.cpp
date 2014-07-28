#include <sstream>
#include "Types.h"

using namespace std;

bool nj::String::toBoolean() throw(InvalidException)
{
   istringstream ss(traits.getValue());
   bool b;
   ss >> b;
   return b;
}

int64_t nj::String::toInt() throw(InvalidException)
{
   istringstream ss(traits.getValue());
   int64_t i;
   ss >> i;
   return  i;
}

double nj::String::toFloat() throw(InvalidException)
{
   istringstream ss(traits.getValue());
   double d;
   ss >> d;
   return d;
}
