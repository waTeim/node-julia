#if defined(WIN32)
#pragma warning(disable:4200)
#endif

#if !win
#include <dlfcn.h>
#endif
#include "dispatch.h"
#include "Trampoline.h"
#include "JMain.h"
#include "JuliaExecEnv.h"

using namespace std;

JuliaExecEnv *JuliaExecEnv::singleton = 0;

JuliaExecEnv *JuliaExecEnv::getSingleton()
{
   if(!singleton)
   {
      const char *argv[1];

      singleton = new JuliaExecEnv();
      argv[0] = JULIA_LIB;
      singleton->engine->initialize(1,argv);
      singleton->trampoline->initialize(0,0);
   }
   return singleton;
}

JuliaExecEnv::JuliaExecEnv()
{

// The workaround for sys.so needing all those libjulia symbols.  On OS/X
// this is not an issue as the dynamic linker resolves differently.  Call
// dlopen explicitly and cause resolution immediately.  Force all symbols
// to global scope.

#if linux
   (void)dlopen(JULIA_LIB "/libjulia.so",RTLD_GLOBAL|RTLD_NOW);
#endif

   engine = new JMain();
   trampoline = new Trampoline();
   j_main_thread = new thread(&JMain::operator(),engine);
   trampoline_thread = new thread(&Trampoline::operator(),trampoline);
}
