#ifndef __nj_Type
#define __nj_Type

#include <map>
#include "Exception.h"

namespace nj
{
   class Type
   {
      protected:

         int id;
         int card;
   
      public:
 
         template<class T> static Type *instance()
         {
            static Type *i = 0;

            if(!i) i = new T();
            return i;
         }

         template<class T> static Type *instance(int card)
         {
            static std::map<int,Type*> instances;

            if(instances.find(card) == instances.end()) instances[card] = new T(card);
            return instances[card]; 
         }

         Type(int id,int card)
         {  
            this->id = id;
            this->card = card;
         }

         int getId() const {  return id;  }
         int getCard() const {  return card;  }
   };
};

#endif
