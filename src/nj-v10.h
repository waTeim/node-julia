#ifndef __nj
#define __nj

#include <memory>
#include <v8.h>
#include "Result.h"

void raiseException(v8::HandleScope &scope,const std::shared_ptr<nj::Result> &res);
v8::Local<v8::Value> mapResult(v8::HandleScope &scope,std::shared_ptr<nj::Result> &res);
v8::Local<v8::String> genError(v8::HandleScope &scope,const std::shared_ptr<nj::Result> &res);
v8::Handle<v8::Value> callbackWithResult(v8::HandleScope &scope,const v8::Local<v8::Function> &cb,const std::shared_ptr<nj::Result> &res);
v8::Handle<v8::Value> returnResult(v8::HandleScope &scope,std::shared_ptr<nj::Result> &res);

#endif
