#ifndef __nj_vi
#define __nj_vi

#include <node.h>
#include <v8.h>
#include "Types.h"
#include "JuliaExecEnv.h"

extern JuliaExecEnv *J;

std::shared_ptr<nj::Value> buildRequest(const v8::Local<v8::Value> &value);

int buildResponse(const std::shared_ptr<std::vector<std::shared_ptr<nj::Value>>> &res,int argc,v8::Local<v8::Value> *argv);

#endif
