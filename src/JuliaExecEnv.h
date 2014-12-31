#ifndef __JuliaExecEnv
#define __JuliaExecEnv

#include <string>
#include <thread>

class Trampoline;
class JMain;

class JuliaExecEnv
{
   protected:
  
      static JuliaExecEnv *singleton;

      std::thread *j_main_thread;
      std::thread *trampoline_thread;
      JMain *engine;
      Trampoline *trampoline;

      JuliaExecEnv();
  
   public:

      static JuliaExecEnv *getSingleton();

      JMain *getEngine() { return engine; }
      Trampoline *getTrampoline() { return trampoline; }
};

#endif
