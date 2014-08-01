#include <sstream>
#include "Values.h"

using namespace std;

bool nj::String::toBoolean() const throw(InvalidException)
{
   istringstream ss(s);
   bool b;
   ss >> b;
   return b;
}

int64_t nj::String::toInt() const throw(InvalidException)
{
   istringstream ss(s);
   int64_t i;
   ss >> i;
   return  i;
}

double nj::String::toFloat() const throw(InvalidException)
{
   istringstream ss(s);
   double d;
   ss >> d;
   return d;
}
