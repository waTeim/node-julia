#include <iostream>
#include <julia.h>
#include "JMain.h"
#include "Call.h"
#include "Script.h"
#include "Immediate.h"

using namespace std;

shared_ptr<nj::Result> JMain::eval(const shared_ptr<nj::Expr> &expr)
{
   if(expr.get()) return shared_ptr<nj::Result>(new nj::Result(expr->eval()));
   return shared_ptr<nj::Result>(new nj::Result());
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
      else jl_init_with_image((char*)install_directory.c_str(),(char*)"sys.ji");
      JL_SET_STACK_BASE;
      done = false;

      while(!done)
      {
         shared_ptr<nj::Expr> expr = dequeue(eval_queue,m_evalq,c_evalq);

         if(expr.get())
         {
            shared_ptr<nj::Result> result = eval(expr);
            enqueue<nj::Result>(result,result_queue,m_resultq,c_resultq);
         }
         else
         {
            shared_ptr<nj::Result> result;
            
            enqueue<nj::Result>(result,result_queue,m_resultq,c_resultq);
         }
         {
            unique_lock<mutex> lock(m_state);

            if(deactivated) done = true;
         }
      }
   }
}

void JMain::eval(const string &text)
{
   shared_ptr<nj::Expr> expr(new nj::Expr());
   expr->args.push_back(shared_ptr<nj::Value>(new nj::UTF8String(text)));
   expr->F = shared_ptr<nj::EvalFunc>(new nj::Immediate);
   
   enqueue(expr,eval_queue,m_evalq,c_evalq);
}

void JMain::exec(const string &funcName,const vector<shared_ptr<nj::Value>> &argv)
{
   shared_ptr<nj::Expr> expr(new nj::Expr());
   expr->args.push_back(shared_ptr<nj::Value>(new nj::UTF8String(funcName)));
   for(shared_ptr<nj::Value> arg: argv) expr->args.push_back(arg);
   expr->F = shared_ptr<nj::EvalFunc>(new nj::Call);

   enqueue(expr,eval_queue,m_evalq,c_evalq);
}

void JMain::exec(const shared_ptr<nj::Value> &module,const string &funcName,const vector<shared_ptr<nj::Value>> &argv)
{
   shared_ptr<nj::Expr> expr(new nj::Expr());
   expr->args.push_back(module);
   expr->args.push_back(shared_ptr<nj::Value>(new nj::UTF8String(funcName)));
   for(shared_ptr<nj::Value> arg: argv) expr->args.push_back(arg);
   expr->F = shared_ptr<nj::EvalFunc>(new nj::Call);

   enqueue(expr,eval_queue,m_evalq,c_evalq);
}

void JMain::compileScript(const std::string &filename)
{
   shared_ptr<nj::Expr> expr(new nj::Expr());
   expr->args.push_back(shared_ptr<nj::Value>(new nj::UTF8String(filename)));
   expr->F = shared_ptr<nj::EvalFunc>(new nj::Script);

   enqueue(expr,eval_queue,m_evalq,c_evalq);
}


shared_ptr<nj::Result> JMain::resultQueueGet()
{
   return dequeue<nj::Result>(result_queue,m_resultq,c_resultq); 
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
