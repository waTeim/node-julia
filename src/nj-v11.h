#ifndef __nj
#define __nj

#include <memory>
#include <v8.h>
#include "Result.h"

void raiseException(const v8::FunctionCallbackInfo<v8::Value> &args,v8::HandleScope &scope,std::shared_ptr<nj::Result> &res);
void callbackWithResult(const v8::FunctionCallbackInfo<v8::Value> &args,v8::HandleScope &scope,v8::Local<v8::Function> &cb,std::shared_ptr<nj::Result> &res);
void returnResult(const v8::FunctionCallbackInfo<v8::Value> &args,v8::HandleScope &scope,std::shared_ptr<nj::Result> &res);

#endif
