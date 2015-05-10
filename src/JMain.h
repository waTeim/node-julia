#ifndef __JMain
#define __JMain

#include <string>
#include "ThreadedIO.h"
#include "Exception.h"
#include "Expr.h"

namespace nj
{
   class Callback;
   class JuliaHandle;
};

class Trampoline;

class JMain:public ThreadedIO
{
   protected:

      std::list<std::shared_ptr<nj::Result>> async_queue;
      std::condition_variable c_evalq;
      std::condition_variable c_syncq;
      std::condition_variable c_asyncq;
      Trampoline *trampoline;
      std::list<std::shared_ptr<nj::Expr>> eval_queue;
      bool initialized;
      std::string install_directory;
      std::mutex m_evalq;
      std::mutex m_syncq;
      std::mutex m_asyncq;
      std::list<std::shared_ptr<nj::Result>> sync_queue;

      std::shared_ptr<nj::Result> eval(const std::shared_ptr<nj::Expr> &expr);

      void enqueue_result(std::shared_ptr<nj::Result> &result,const nj::Expr::Dest &dest);

   public:

      JMain();

      void initialize(int argc,const char *argv[]) throw(nj::InitializationException);
      void operator()();
      std::shared_ptr<nj::Result> asyncQueueGet();
      void compileScript(const std::string &filename);
      void convert(const std::shared_ptr<nj::JuliaHandle> &val);
      void eval(const std::string &expr,nj::Callback *c = 0);
      void exec(const std::string &funcName,const std::vector<std::shared_ptr<nj::Value>> &argv,nj::Callback *c = 0);
      void exec(const std::shared_ptr<nj::Value> &module,const std::string &funcName,const std::vector<std::shared_ptr<nj::Value>> &argv,nj::Callback *c = 0);
      void import(const std::string &expr,nj::Callback *c = 0);
      void stop();
      std::shared_ptr<nj::Result> syncQueueGet();
      ~JMain();
};

#endif
