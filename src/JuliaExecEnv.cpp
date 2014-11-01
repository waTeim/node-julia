#if !win
#include <dlfcn.h>
#endif
#include "JuliaExecEnv.h"

using namespace std;

JuliaExecEnv *J = 0;

JuliaExecEnv::JuliaExecEnv(const std::string &installDir)
{

// The workaround for sys.so needing all those libjulia symbols.  On OS/X
// this is not an issue as the dynamic linker resolves differently.  Call
// dlopen explicitly and cause resolution immediately.  Force all symbols
// to global scope.

#if linux
   (void)dlopen(JULIA_LIB "/libjulia.so",RTLD_GLOBAL|RTLD_NOW);
#endif

   const char *argv[1];

   engine = new JMain();
   if(installDir != "") argv[0] = installDir.c_str();
   else argv[0] = JULIA_LIB;
   j_main_thread = new thread(&JMain::operator(),engine);
   engine->initialize(1,argv);
}
