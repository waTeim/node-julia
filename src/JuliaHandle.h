#ifndef __nj_JuliaHandle
#define __nj_JuliaHandle

#include "Value.h"
#include "Exception.h"

extern "C"
{
   typedef struct _jl_value_t jl_value_t;
};

namespace nj
{
   class JuliaHandle: public Value
   {
      protected:

         static std::vector<size_t> none;
         static std::map<int64_t,JuliaHandle*> htab;
         static std::map<jl_value_t*,int64_t> vtab;
         static int64_t next_htab_index;

         jl_value_t *_val;
         int64_t preserve_index;
         int64_t htab_index;
         std::map<std::string,std::shared_ptr<JuliaHandle>> element_list;

      public:

         static JuliaHandle *atIndex(int64_t htabIndex);
         static JuliaHandle *forVal(jl_value_t* val);

         JuliaHandle(jl_value_t *val,bool preserve = false) throw(JuliaException);
         virtual bool isPrimitive() const { return false; }
         virtual const Type *type() const { return JuliaHandle_t::instance(); }
         virtual const std::vector<size_t> &dims() const {  return none;  }
         virtual std::vector<std::string> properties() const;
         jl_value_t *val() const {  return _val;  }
         virtual int64_t intern();
         virtual std::shared_ptr<nj::JuliaHandle> getElement(std::string id);
         virtual ~JuliaHandle() throw(JuliaException);
   };
};

#endif
