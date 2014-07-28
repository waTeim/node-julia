#include <julia.h>
#include "JMain.h"
#include "Immediate.h"

using namespace std;

shared_ptr<vector<shared_ptr<nj::Value>>> JMain::eval(const shared_ptr<nj::Expr> &expr)
{
   if(expr.get()) return shared_ptr<vector<shared_ptr<nj::Value>>>(new vector<shared_ptr<nj::Value>>(expr->eval()));
   return shared_ptr<vector<shared_ptr<nj::Value>>>(new vector<shared_ptr<nj::Value>>());
}

JMain::JMain()
{
   initialized = false;
   deactivated = false;
}

void JMain::initialize(int argc,const char *argv[]) throw(nj::InitializationException)
{
   unique_lock<mutex> lock(m_state);

   if(argc >= 1) install_directory = argv[0];
   initialized = true;
}

void JMain::operator()()
{
   bool done = false;

   while(!done)
   {
      unique_lock<mutex> lock(m_state);

      if(!initialized) c_state.wait(lock);
      if(initialized || deactivated) done = true;
   }

   if(!deactivated)
   {
      if(install_directory == "") jl_init(0);
      else jl_init((char*)install_directory.c_str());
      JL_SET_STACK_BASE;

      done = false;

      while(!done)
      {
         shared_ptr<nj::Expr> expr = dequeue(eval_queue,m_evalq,c_evalq);

printf("Dequeued an Expr\n");
         if(expr.get())
         {
            shared_ptr<vector<shared_ptr<nj::Value>>> result = eval(expr);
printf("Evaled an Expr\n");
            enqueue<vector<shared_ptr<nj::Value>>>(result,result_queue,m_resultq,c_resultq);
printf("Enqueued a Result\n");
         }
         else
         {
            shared_ptr<vector<shared_ptr<nj::Value>>> result;
            
            enqueue<vector<shared_ptr<nj::Value>>>(result,result_queue,m_resultq,c_resultq);
         }
         {
            unique_lock<mutex> lock(m_state);

            if(deactivated) done = true;
         }
      }
   }
}

void JMain::evalQueuePut(const string &text)
{
   shared_ptr<nj::Expr> expr(new nj::Expr());
   expr->args.push_back(shared_ptr<nj::Value>(new nj::String(text)));
   expr->F = shared_ptr<nj::EvalFunc>(new nj::Immediate);
   
   enqueue(expr,eval_queue,m_evalq,c_evalq);
}

shared_ptr<vector<shared_ptr<nj::Value>>> JMain::resultQueueGet()
{
   return dequeue<vector<shared_ptr<nj::Value>>>(result_queue,m_resultq,c_resultq); 
}

void JMain::stop()
{
   unique_lock<mutex> lock(m_state);

   deactivated = true;
   c_state.notify_all();
   c_evalq.notify_all();
   c_resultq.notify_all();
}


JMain::~JMain() {}
