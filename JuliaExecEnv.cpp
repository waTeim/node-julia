#include "JuliaExecEnv.h"

using namespace std;

JuliaExecEnv::JuliaExecEnv(const std::string &installDir)
{
   const char *argv[1];

   engine = new JMain();
   argv[0] = installDir.c_str();
   j_main_thread = new thread(&JMain::operator(),engine);
   engine->initialize(1,argv);
}
