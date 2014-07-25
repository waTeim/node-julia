#include <julia.h>
#include "JMain.h"

using namespace std;

shared_ptr<nj::Expr> JMain::eval(const shared_ptr<nj::Expr> &expr)
{
   shared_ptr<nj::Expr> result;

   return result;
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

shared_ptr<nj::Expr> JMain::de_queue(list<shared_ptr<nj::Expr>> &queue)
{
   bool done = false;
   shared_ptr<nj::Expr> expr;

   while(!done)
   {
      {
         unique_lock<mutex> lock(m_state);

         while(eval_queue.empty() && !done)
         {
printf("Waiting for an expression\n");
            c_state.wait(lock);
            if(deactivated) done = true;
         }

         if(!deactivated)
         {
printf("Checking end of queue\n");
            if((expr = queue.back()))
            {
printf("Got expr\n");
               queue.pop_back();
printf("Poped end of queue\n");
               if(expr.get())
               {
                  done = true;
                  printf("Expr text = %s\n",expr->getText().c_str());
               }
            }
         }
         else done = true;
      }
   }
   return expr;
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
printf("entering de_queue\n");
         shared_ptr<nj::Expr> expr = de_queue(eval_queue);

printf("got an expr\n");

         if(expr.get())
         {
printf("expr text = %s\n",expr->getText().c_str());
            shared_ptr<nj::Expr> result = eval(expr);

printf("Got a result\n");
            if(result.get())
            {
printf("Expr result = %s\n",result->getText().c_str());
              unique_lock<mutex> lock(m_state); 

              if(!deactivated)
              {
printf("Pushing result\n");
                 result_queue.push_front(result);
                 c_state.notify_all();
              }
              else done = true;
            }
            else result_queue.push_front(result);
         }
         else printf("Expr was null\n");
      }
   }
}

void JMain::evalQueuePut(const string &expressionText)
{
   unique_lock<mutex> lock(m_state);
   shared_ptr<nj::Expr> expr(new nj::Expr(expressionText));
printf("Received an expr %s\n",expressionText.c_str());
   eval_queue.push_front(expr);
   c_state.notify_all();
}

string JMain::resultQueueGet()
{
   shared_ptr<nj::Expr> expr = de_queue(result_queue); 
printf("Got Result\n");
   if(expr.get())
   {
printf("Result text = %s\n",expr->getText().c_str());
    return expr->getText();
   }
printf("Result text = ""\n");
   return "";
}

void JMain::stop()
{
   unique_lock<mutex> lock(m_state);

   deactivated = true;
   c_state.notify_all();
}


JMain::~JMain() {}
