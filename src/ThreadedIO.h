#ifndef __ThreadedIO
#define __ThreadedIO

#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>

class ThreadedIO
{
   protected:

      std::condition_variable c_state;
      std::mutex m_state;
      bool deactivated;

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

      ThreadedIO() {  deactivated = false; }

};

#endif
