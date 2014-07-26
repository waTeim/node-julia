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

shared_ptr<nj::Expr> JMain::dequeue(list<shared_ptr<nj::Expr>> &queue,mutex &m_queue,condition_variable &c_queue)
{
   bool done = false;
   shared_ptr<nj::Expr> expr;

   while(!done)
   {
      {
         unique_lock<mutex> lock(m_queue);

         if(queue.empty()) c_queue.wait(lock);
         if(!queue.empty())
         {
            expr = queue.back();
            queue.pop_back();
            done = true;
            if(expr.get()) printf("Dequeued Expr text = %s\n",expr->getText().c_str());
            else printf("Dequeued Null text\n");
         }
      }
      {
         unique_lock<mutex> lock(m_state);

         if(deactivated) done = true;
      }
   }
   return expr;
}

void JMain::enqueue(shared_ptr<nj::Expr> &expr,list<shared_ptr<nj::Expr>> &queue,mutex &m_queue,condition_variable &c_queue)
{
   unique_lock<mutex> lock(m_queue);

   queue.push_front(expr);
   c_queue.notify_all();
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
         shared_ptr<nj::Expr> expr = dequeue(eval_queue,m_evalq,c_evalq);

printf("got an expr\n");

         if(expr.get())
         {
printf("expr text = %s\n",expr->getText().c_str());
            shared_ptr<nj::Expr> result = eval(expr);
printf("Eval expr\n");
            enqueue(result,result_queue,m_resultq,c_resultq);
         }
         else
         {
            printf("Expr to eval is null result will be null\n");
            shared_ptr<nj::Expr> result;
            
            enqueue(result,result_queue,m_resultq,c_resultq);
         }
         {
            unique_lock<mutex> lock(m_state);

            if(deactivated) done = true;
         }
      }
   }
}

void JMain::evalQueuePut(const string &expressionText)
{
   shared_ptr<nj::Expr> expr(new nj::Expr(expressionText));
printf("Received an expr from Outside%s\n",expressionText.c_str());
   enqueue(expr,eval_queue,m_evalq,c_evalq);
}

string JMain::resultQueueGet()
{
   shared_ptr<nj::Expr> expr = dequeue(result_queue,m_resultq,c_resultq); 
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
   c_evalq.notify_all();
   c_resultq.notify_all();
}


JMain::~JMain() {}
