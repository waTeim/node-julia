#include <julia.h>
#include "JMain.h"

using namespace std;

void JMain::eval(const std::shared_ptr<nj::Expr> &expr)
{
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
      shared_ptr<nj::Expr> expr;

      while(!done)
      {
         {
            unique_lock<mutex> lock(m_state);

            while(expr_queue.empty() && !done)
            {
               c_state.wait(lock);
               if(deactivated) done = true;
            }

            if(!deactivated)
            {
               expr = expr_queue.back();
               expr_queue.pop_back();
            }
         }

         if(!done && expr.get()) eval(expr);
      }
   }
}

void JMain::addExpr(const string &text)
{
   unique_lock<mutex> lock(m_state);
   shared_ptr<nj::Expr> expr(new nj::Expr(text));

   expr_queue.insert(expr_queue.begin(),expr);
   c_state.notify_all();
}

void JMain::stop()
{
   unique_lock<mutex> lock(m_state);

   deactivated = true;
   c_state.notify_all();
}


JMain::~JMain() {}
