#include <sstream>
#include "Values.h"

using namespace std;

bool nj::String::toBoolean() const throw(InvalidException)
{
   istringstream ss(val());
   bool b;
   ss >> b;
   return b;
}

char nj::String::toChar() const throw(InvalidException)
{
   istringstream ss(val());
   char c;
   ss >> c;
   return c;
}

int64_t nj::String::toInt() const throw(InvalidException)
{
   istringstream ss(val());
   int64_t i;
   ss >> i;
   return  i;
}

uint64_t nj::String::toUInt() const throw(InvalidException)
{
   istringstream ss(val());
   uint64_t i;
   ss >> i;
   return  i;
}

double nj::String::toFloat() const throw(InvalidException)
{
   istringstream ss(val());
   double d;
   ss >> d;
   return d;
}
