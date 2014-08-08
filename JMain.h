#ifndef __JMain
#define __JMain

#include <string>
#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "Exception.h"
#include "Expr.h"


class JMain
{
   protected:

      std::condition_variable c_state;
      std::condition_variable c_evalq;
      std::condition_variable c_resultq;
      bool deactivated;
      std::list<std::shared_ptr<nj::Expr>> eval_queue;
      bool initialized;
      std::string install_directory;
      std::mutex m_state;
      std::mutex m_evalq;
      std::mutex m_resultq;
      std::list<std::shared_ptr<std::vector<std::shared_ptr<nj::Value>>>> result_queue;

      std::shared_ptr<std::vector<std::shared_ptr<nj::Value>>> eval(const std::shared_ptr<nj::Expr> &expr);

      template <typename T> std::shared_ptr<T> dequeue(std::list<std::shared_ptr<T>> &queue,std::mutex &m_queue,std::condition_variable &c_queue)
      {
         bool done = false;
         std::shared_ptr<T> element;

         while(!done)
         {
            {
               std::unique_lock<std::mutex> lock(m_queue);

               if(queue.empty()) c_queue.wait(lock);
               if(!queue.empty())
               {
                  element = queue.back();
                  queue.pop_back();
                  done = true;
               }
            }
            {
               std::unique_lock<std::mutex> lock(m_state);

               if(deactivated) done = true;
            }
         }
         return element;
      }

      template <typename T> void enqueue(std::shared_ptr<T> &element,std::list<std::shared_ptr<T>> &queue,std::mutex &m_queue,std::condition_variable &c_queue)
      {
         std::unique_lock<std::mutex> lock(m_queue);

         queue.push_front(element);
         c_queue.notify_all();
      }

   public:

      JMain();

      void initialize(int argc,const char *argv[]) throw(nj::InitializationException);
      void operator()();
      void evalQueuePut(const std::string &expr);
      void evalQueuePut(const std::string &funcName,const std::vector<std::shared_ptr<nj::Value>> &argv);
      std::shared_ptr<std::vector<std::shared_ptr<nj::Value>>> resultQueueGet();
      void stop();
      ~JMain();
};

#endif
