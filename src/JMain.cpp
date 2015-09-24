#if defined(WIN32)
#pragma warning(disable:4200)
#define IFS '\\'
#else
#define IFS '/'
#endif


#include <iostream>
#include <sstream>
#include <julia.h>
#include "util.h"
#include "JMain.h"
#include "Call.h"
#include "Script.h"
#include "Callback.h"
#include "Immediate.h"
#include "Import.h"
#include "Convert.h"
#include "JuliaHandle.h"
#include "JuliaExecEnv.h"
#include "Trampoline.h"

using namespace std;

shared_ptr<nj::Result> JMain::eval(const shared_ptr<nj::Expr> &expr)
{
   if(expr.get()) return shared_ptr<nj::Result>(new nj::Result(expr->eval()));
   return shared_ptr<nj::Result>(new nj::Result());
}

void JMain::enqueue_result(shared_ptr<nj::Result> &result,const nj::Expr::Dest &dest)
{
   switch(dest)
   {
      case nj::Expr::syncQ:
         enqueue<nj::Result>(result,sync_queue,m_syncq,c_syncq);
      break;
      case nj::Expr::asyncQ:
         enqueue<nj::Result>(result,async_queue,m_asyncq,c_asyncq);
      break;
   }
}


JMain::JMain()
{
   initialized = false;
}

void JMain::initialize(int argc,const char *argv[]) throw(nj::InitializationException)
{
   unique_lock<mutex> lock(m_state);

   if(argc >= 1) install_directory = argv[0];
   trampoline = JuliaExecEnv::getSingleton()->getTrampoline();
   initialized = true;
   c_state.notify_all();
}

void setPrecompiled(bool usePrecompiled)
{
   #if defined(JULIA_VERSION_MINOR) && JULIA_VERSION_MINOR >= 4 && !defined(JL_OPTIONS_DUMPBITCODE_ON)
   if(usePrecompiled) jl_options.use_precompiled = 1;
   else jl_options.use_precompiled = 0;
   #endif
}

string getSysImageName()
{
   #if defined(JULIA_VERSION_MINOR) && JULIA_VERSION_MINOR >= 4 && !defined(JL_OPTIONS_DUMPBITCODE_ON)
   string imageFile = jl_options.image_file;
   vector<string> pathParts = nj::split(imageFile,'/');

   return pathParts[pathParts.size() - 1];
   #else
   return "sys.ji";
   #endif
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

      #ifdef WIN32
      setPrecompiled(true);
      #endif

      #if defined(JL_OPTIONS_HANDLE_SIGNALS_OFF)
      jl_options.handle_signals = JL_OPTIONS_HANDLE_SIGNALS_OFF;
      #endif

      if(install_directory == "") jl_init(0);
      else jl_init_with_image((char*)install_directory.c_str(),(char*)getSysImageName().c_str());

      #ifdef JL_SET_STACK_BASE
      JL_SET_STACK_BASE;
      #endif

      done = false;

      while(!done)
      {
         shared_ptr<nj::Expr> expr = dequeue(eval_queue,m_evalq,c_evalq);

         if(expr.get())
         {
            shared_ptr<nj::Result> result = eval(expr);

            enqueue_result(result,expr->dest);
         }
         else
         {
            shared_ptr<nj::Result> result;

            enqueue_result(result,expr->dest);
         }
         {
            unique_lock<mutex> lock(m_state);

            if(deactivated) done = true;
         }
      }

//#if defined(JULIA_VERSION_MINOR) && JULIA_VERSION_MINOR == 4
//      jl_atexit_hook();
//#endif

   }
}

shared_ptr<nj::Result> JMain::asyncQueueGet()
{
   return dequeue<nj::Result>(async_queue,m_asyncq,c_asyncq);
}

void JMain::compileScript(const std::string &filename)
{
   shared_ptr<nj::Expr> expr(new nj::Expr());

   expr->args.push_back(shared_ptr<nj::Value>(new nj::UTF8String(filename)));
   expr->F = shared_ptr<nj::EvalFunc>(new nj::Script);
   enqueue(expr,eval_queue,m_evalq,c_evalq);
}

void JMain::convert(const std::shared_ptr<nj::JuliaHandle> &val)
{
   shared_ptr<nj::Expr> expr(new nj::Expr(nj::Expr::syncQ));

   expr->args.push_back(val);
   expr->F = shared_ptr<nj::EvalFunc>(new nj::Convert);
   enqueue(expr,eval_queue,m_evalq,c_evalq);
}

void JMain::eval(const string &text,nj::Callback *c)
{
   shared_ptr<nj::Expr> expr(new nj::Expr(c?nj::Expr::asyncQ:nj::Expr::syncQ));

   expr->args.push_back(shared_ptr<nj::Value>(new nj::UTF8String(text)));
   expr->F = shared_ptr<nj::EvalFunc>(new nj::Immediate);
   if(c) trampoline->addJump(expr->id,shared_ptr<nj::Callback>(c));
   enqueue(expr,eval_queue,m_evalq,c_evalq);
}

void JMain::exec(const string &funcName,const vector<shared_ptr<nj::Value>> &argv,nj::Callback *c)
{
   shared_ptr<nj::Expr> expr(new nj::Expr(c?nj::Expr::asyncQ:nj::Expr::syncQ));

   expr->args.push_back(shared_ptr<nj::Value>(new nj::UTF8String(funcName)));
   for(shared_ptr<nj::Value> arg: argv) expr->args.push_back(arg);
   expr->F = shared_ptr<nj::EvalFunc>(new nj::Call);
   if(c) trampoline->addJump(expr->id,shared_ptr<nj::Callback>(c));
   enqueue(expr,eval_queue,m_evalq,c_evalq);
}

void JMain::exec(const shared_ptr<nj::Value> &module,const string &funcName,const vector<shared_ptr<nj::Value>> &argv,nj::Callback *c)
{
   shared_ptr<nj::Expr> expr(new nj::Expr(c?nj::Expr::asyncQ:nj::Expr::syncQ));

   expr->args.push_back(module);
   expr->args.push_back(shared_ptr<nj::Value>(new nj::UTF8String(funcName)));
   for(shared_ptr<nj::Value> arg: argv) expr->args.push_back(arg);
   expr->F = shared_ptr<nj::EvalFunc>(new nj::Call);
   if(c) trampoline->addJump(expr->id,shared_ptr<nj::Callback>(c));
   enqueue(expr,eval_queue,m_evalq,c_evalq);
}

void JMain::import(const string &text,nj::Callback *c)
{
   shared_ptr<nj::Expr> expr(new nj::Expr(c?nj::Expr::asyncQ:nj::Expr::syncQ));

   expr->args.push_back(shared_ptr<nj::Value>(new nj::UTF8String(text)));
   expr->F = shared_ptr<nj::EvalFunc>(new nj::Import);
   if(c) trampoline->addJump(expr->id,shared_ptr<nj::Callback>(c));
   enqueue(expr,eval_queue,m_evalq,c_evalq);
}

void JMain::stop()
{
   unique_lock<mutex> lock(m_state);

   deactivated = true;
   c_state.notify_all();
   c_evalq.notify_all();
   c_asyncq.notify_all();
   c_syncq.notify_all();
}

shared_ptr<nj::Result> JMain::syncQueueGet()
{
   return dequeue<nj::Result>(sync_queue,m_syncq,c_syncq);
}

JMain::~JMain() {}
