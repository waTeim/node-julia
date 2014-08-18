#include "JuliaExecEnv.h"

using namespace std;

JuliaExecEnv::JuliaExecEnv(const std::string &installDir)
{
   const char *argv[1];

   engine = new JMain();
   if(installDir != "") argv[0] = installDir.c_str();
   else argv[0] = JULIA_DIR "/lib";
   j_main_thread = new thread(&JMain::operator(),engine);
   engine->initialize(1,argv);
}
