#include <sstream>
#include <iostream>
#include "util.h"

using namespace std;

vector<string> &nj::split(const string &s,char delim,vector<string> &elems)
{
   stringstream ss(s);
   string item;

   while(getline(ss,item,delim)) elems.push_back(item);
   return elems;
}


vector<string> nj::split(const string &s,char delim)
{
   vector<string> elems;

   split(s,delim,elems);
   return elems;
}
