#include <sstream>
#include "Values.h"

using namespace std;

bool nj::ASCIIString::toBoolean() const throw(InvalidException)
{
   istringstream ss(val());
   bool b;
   ss >> b;
   return b;
}

char nj::ASCIIString::toChar() const throw(InvalidException)
{
   istringstream ss(val());
   char c;
   ss >> c;
   return c;
}

int64_t nj::ASCIIString::toInt() const throw(InvalidException)
{
   istringstream ss(val());
   int64_t i;
   ss >> i;
   return  i;
}

uint64_t nj::ASCIIString::toUInt() const throw(InvalidException)
{
   istringstream ss(val());
   uint64_t i;
   ss >> i;
   return  i;
}

double nj::ASCIIString::toFloat() const throw(InvalidException)
{
   istringstream ss(val());
   double d;
   ss >> d;
   return d;
}

bool nj::UTF8String::toBoolean() const throw(InvalidException)
{
   istringstream ss(val());
   bool b;
   ss >> b;
   return b;
}

char nj::UTF8String::toChar() const throw(InvalidException)
{
   istringstream ss(val());
   char c;
   ss >> c;
   return c;
}

int64_t nj::UTF8String::toInt() const throw(InvalidException)
{
   istringstream ss(val());
   int64_t i;
   ss >> i;
   return  i;
}

uint64_t nj::UTF8String::toUInt() const throw(InvalidException)
{
   istringstream ss(val());
   uint64_t i;
   ss >> i;
   return  i;
}

double nj::UTF8String::toFloat() const throw(InvalidException)
{
   istringstream ss(val());
   double d;
   ss >> d;
   return d;
}
