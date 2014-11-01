#include <errno.h>
#include <string.h>
#include <limits.h>
#include "Exception.h"

#if win
#include <Windows.h>
#define PATH_MAX MAX_PATH
#define strerror_r(errno,buf,len) strerror_s(buf,len,errno)
#endif

using namespace std;

nj::SystemException::SystemException(const string &what):Exception(system_exception,what)
{
   if(errno != 0)
   {
      char buffer[PATH_MAX + 100];

#if ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE) || __APPLE__

// If this evaluates to true then the XSI=compiliant version of strerror_r
// is made available and buffer contains the result.

      strerror_r(errno,buffer,sizeof(buffer));
      _what = what + ": " + buffer;
#else

/**********************************************************************************
 * Otherwise, the GNU specific version is supplied and buffer may or may not
 * contain the result, but the function always will return it.
 *
 * Or it's windows.
**********************************************************************************/

#if win

      strerror_r(errno,buffer,sizeof(buffer));
      _what = what + ": " + buffer;
#else
      _what = what + ": " + strerror_r(errno,buffer,sizeof(buffer));
#endif

#endif

   }
}

