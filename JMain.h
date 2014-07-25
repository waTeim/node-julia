#ifndef __JMain
#define __JMain

#include <string>
#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "Exception.h"
#include "interface.h"


class JMain
{
   protected:

      std::condition_variable c_state;
      bool deactivated;
      bool initialized;
      std::string install_directory;
      std::mutex m_state;
      std::list<std::shared_ptr<nj::Expr>> eval_queue;
      std::list<std::shared_ptr<nj::Expr>> result_queue;

      std::shared_ptr<nj::Expr> eval(const std::shared_ptr<nj::Expr> &expr);
      std::shared_ptr<nj::Expr> de_queue(std::list<std::shared_ptr<nj::Expr>> &queue);


   public:

      JMain();

      void initialize(int argc,const char *argv[]) throw(nj::InitializationException);
      void operator()();
      void evalQueuePut(const std::string &expressionText);
      std::string resultQueueGet();
      void stop();
      ~JMain();
};

#endif
