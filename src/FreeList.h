#ifndef __nj_FreeList
#define __nj_FreeList

#include <memory>

namespace nj
{
   template <typename T> class FreeList
   {
      protected:

         int64_t freelist_start;
         std::vector<int64_t> freelist;
         std::map<int64_t,std::shared_ptr<T>> objs;
   
       public:

         FreeList() { freelist_start = -1; }

         std::shared_ptr<T> free(int64_t index)
         {
            freelist[index] = freelist_start;
            freelist_start = index;

            std::shared_ptr<T> res = objs[index];
            objs.erase(index);
            return res;
         }

         int64_t store(T *obj)
         {
            int64_t free_index;
         
            if(freelist_start == -1)
            {
               freelist.push_back(-1);
               free_index = freelist.size() - 1;
            }
            else
            {
               free_index = freelist_start;
               freelist_start = freelist[free_index];
               freelist[free_index] = -1;
            }

            objs[free_index] = std::shared_ptr<T>(obj);
            return free_index;
         }

         std::shared_ptr<T> get(int64_t index) const
         {
            std::shared_ptr<T> res;
            typename std::map<int64_t,std::shared_ptr<T>>::const_iterator i = objs.find(index);

            if(i != objs.end()) return i->second;
            return res;
         }
   };
};

#endif
