#include "Types.h"

bool nj::operator<(const nj::Type &t1,const nj::Type &t2)
{
   bool res = false;

   if(t1.getId() == nj::boolean_type && t2.getId() != nj::boolean_type) res = true;
   else
   {
      switch(t1.getId())
      {
         case int8_type:
         {
            switch(t2.getId())
            {
               case int16_type:
               case int32_type:
               case int64_type:
               case float32_type:
               case float64_type:
                  res = true;
               break;
            }
         }
         break;
         case uint8_type:
         {
            switch(t2.getId())
            {
               case int16_type:
               case uint16_type:
               case int32_type:
               case uint32_type:
               case int64_type:
               case uint64_type:
               case float32_type:
               case float64_type:
                  res = true;
               break;
            }
         }
         break;
         case int16_type:
         {
            switch(t2.getId())
            {
               case int32_type:
               case int64_type:
               case float32_type:
               case float64_type:
                  res = true;
               break;
            }
         }
         break;
         case uint16_type:
         {
            switch(t2.getId())
            {
               case int32_type:
               case uint32_type:
               case int64_type:
               case uint64_type:
               case float32_type:
               case float64_type:
                  res = true;
               break;
            }
         }
         break;
         case int32_type:
         {
            switch(t2.getId())
            {
               case int64_type:
               case float64_type:
                  res = true;
               break;
            }
         }
         break;
         case uint32_type:
         {
            switch(t2.getId())
            {
               case int64_type:
               case uint64_type:
               case float64_type:
                  res = true;
               break;
            }
         }
         break;
         case float32_type:
         {
            switch(t2.getId())
            {
               case float64_type:
                  res = true;
               break;
            }
         }
         break;
         case ascii_string_type:
         {
            switch(t2.getId())
            {
               case utf8_string_type:
                  res = true;
               break;
            }
         }
         break;
      }
   }
 
   return res;
}
