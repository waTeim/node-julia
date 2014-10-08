#ifndef __JuliaExecEnv
#define __JuliaExecEnv

#include <string>
#include <thread>
#include "JMain.h"

class JuliaExecEnv
{
   protected:
  
      std::thread *j_main_thread;
      JMain *engine;

   public:

      JuliaExecEnv(const std::string &installDir = "");
  
      JMain *getEngine() {  return engine;  }
};

extern JuliaExecEnv *J;

#endif
