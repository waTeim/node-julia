#include <iostream>
#include "juliav.h"
#include "Kernel.h"
#include "JuliaHandle.h"

using namespace std;

vector<size_t> nj::JuliaHandle::none;
map<int64_t,nj::JuliaHandle*> nj::JuliaHandle::htab;
map<jl_value_t*,int64_t> nj::JuliaHandle::vtab;
int64_t nj::JuliaHandle::next_htab_index = 0;

nj::JuliaHandle *nj::JuliaHandle::atIndex(int64_t htabIndex)
{
   map<int64_t,JuliaHandle*>::iterator i = htab.find(htabIndex);

   if(i == htab.end()) return 0;
   return i->second;
}

nj::JuliaHandle *nj::JuliaHandle::forVal(jl_value_t* val)
{
   map<jl_value_t*,int64_t>::iterator i = vtab.find(val);
 
   if(i == vtab.end()) return 0;
   return atIndex(i->second);
}

nj::JuliaHandle::JuliaHandle(jl_value_t *val,bool preserve) throw(JuliaException)
{ 
   _val = val;
   if(preserve)
   {
      Kernel *kernel = Kernel::getSingleton();
 
      preserve_index = kernel->preserve(val);
   }
   else preserve_index = -1;
   htab_index = -1;

   jl_value_t *vtype = jl_typeof(val);

   if((jl_is_structtype(vtype) || jl_is_bitstype(vtype)) && !jl_is_module(val) && !jl_is_tuple(val) && !jl_is_expr(val) && !jl_is_function(val))
   {

#if defined(USES_SVEC)
      size_t numFields = jl_datatype_nfields(vtype);

      for(size_t i = 0;i < numFields;i++)
      {
         jl_value_t *fieldValue = jl_fieldref(val,i);
         jl_sym_t *fieldNameSym = jl_field_name(vtype,i);
         char *fieldName = fieldNameSym->name;

         if(fieldName && fieldValue) element_list[fieldName] = shared_ptr<JuliaHandle>(new JuliaHandle(fieldValue));
      }
#else
      jl_tuple_t *fieldNames = ((jl_datatype_t*)vtype)->names;

      if(fieldNames)
      {
         size_t numFields = jl_tuple_len(fieldNames);

         for(size_t i = 0;i < numFields;i++)
         {
            jl_value_t *fieldValue = jl_fieldref(val,i);
            jl_sym_t *fieldNameSym = (jl_sym_t*)jl_tupleref(fieldNames,i);
            char *fieldName = fieldNameSym->name;
 
            if(fieldName && fieldValue) element_list[fieldName] = shared_ptr<JuliaHandle>(new JuliaHandle(fieldValue));
         }
      }
#endif
   }
}

vector<string> nj::JuliaHandle::properties() const
{
   vector<string> res;

   for(pair<string,shared_ptr<JuliaHandle>> element: element_list) res.push_back(element.first);
   return res;
}

shared_ptr<nj::JuliaHandle> nj::JuliaHandle::getElement(std::string id)
{
   map<std::string,shared_ptr<JuliaHandle>>::iterator i = element_list.find(id);

   if(i != element_list.end()) return i->second;
   return 0;
}

int64_t nj::JuliaHandle::intern()
{
   if(htab_index == -1)
   {
      htab_index = next_htab_index++;
      htab[htab_index] = new JuliaHandle(*this);
      vtab[_val] = htab_index;
      preserve_index = -1;
   }
   return htab_index;
}

nj::JuliaHandle::~JuliaHandle() throw(JuliaException)
{
   if(preserve_index != -1)
   {
     Kernel *kernel = Kernel::getSingleton();

     (void)kernel->free(preserve_index);
   }
   if(htab_index != -1)
   {
      vtab.erase(_val);
      htab[htab_index] = 0;
      htab.erase(htab_index);
   }
}
