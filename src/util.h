#ifndef __nj_util
#define __nj_util

#include <string>
#include <vector>

namespace nj
{
   std::vector<std::string> &split(const std::string &s,char delim,std::vector<std::string> &elems);
   std::vector<std::string> split(const std::string &s,char delim);
};

#endif
