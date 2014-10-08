#ifndef __nj
#define __nj

#include <memory>
#include <v8.h>
#include "Result.h"

v8::Handle<v8::Value> callbackWithResult(v8::HandleScope &scope,v8::Local<v8::Function> &cb,std::shared_ptr<nj::Result> &res);

#endif
