#ifndef __Trampoline
#define __Trampoline

#include <vector>
#include <map>
#include <list>
#include "ThreadedIO.h"
#include "Binding.h"
#include "Exception.h"

class JMain;

class Trampoline: public ThreadedIO
{
   protected:

      std::list<std::shared_ptr<nj::Binding>> call_queue;
      std::condition_variable c_callq;
      std::map<int64_t,std::shared_ptr<nj::Callback>> jump_map;
      JMain *engine;
      bool initialized;
      std::mutex m_callq;

   public:

      Trampoline();

      void initialize(int argc,const char *argv[]) throw(nj::InitializationException);
      void operator()();
      void addJump(int64_t,const std::shared_ptr<nj::Callback> &dest);
      std::list<std::shared_ptr<nj::Binding>> getCallList();
      void stop();
      ~Trampoline();
};

#endif
