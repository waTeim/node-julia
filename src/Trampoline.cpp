#include <iostream>
#include "Callback.h"
#include "dispatch.h"
#include "Trampoline.h"
#include "JuliaExecEnv.h"
#include "JMain.h"

using namespace std;

Trampoline::Trampoline()
{
   initialized = false;
   engine = 0;
}

void Trampoline::initialize(int argc,const char *argv[]) throw(nj::InitializationException)
{
   unique_lock<mutex> lock(m_state);
   engine = JuliaExecEnv::getSingleton()->getEngine();
   initialized = true;
   c_state.notify_all();
}

void Trampoline::operator()()
{
   bool done = false;

   while(!done)
   {
      unique_lock<mutex> lock(m_state);

      if(!initialized) c_state.wait(lock);
      if(initialized || deactivated) done = true;
   }

   if(!deactivated && engine)
   {
      done = false;

      while(!done)
      {
         std::shared_ptr<nj::Result> res = engine->asyncQueueGet();

         {
            unique_lock<mutex> lock(m_state);
         
            map<int64_t,shared_ptr<nj::Callback>>::iterator i = jump_map.find(res->exprId());

            if(i != jump_map.end())
            {
               shared_ptr<nj::Binding> binding = shared_ptr<nj::Binding>(new nj::Binding(res,i->second));
              
               enqueue<nj::Binding>(binding,call_queue,m_callq,c_callq);
               jump_map.erase(res->exprId());
               nj::async_dispatch();
            }
         }


         {
            unique_lock<mutex> lock(m_state);

            if(deactivated) done = true;
         }
      }
   }
}

void Trampoline::addJump(int64_t exprId,const shared_ptr<nj::Callback> &dest)
{
   unique_lock<mutex> lock(m_state);

   jump_map[exprId] = dest;
}

list<shared_ptr<nj::Binding>> Trampoline::getCallList()
{
   unique_lock<mutex> lock1(m_state);
   unique_lock<mutex> lock2(m_callq);
   std::list<std::shared_ptr<nj::Binding>> res = call_queue;

   call_queue.clear();
   return res;
}

void Trampoline::stop()
{
   unique_lock<mutex> lock(m_state);

   deactivated = true;
   c_state.notify_all();
}

Trampoline::~Trampoline() {}
