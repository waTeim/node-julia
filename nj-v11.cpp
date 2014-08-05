#include <stdio.h>
#include <node.h>
#include <v8.h>
#include <string>
#include "Types.h"
#include "JuliaExecEnv.h"

using namespace std;
using namespace v8;

static JuliaExecEnv *J = 0;

void returnNull(const FunctionCallbackInfo<Value> &args,Isolate *I)
{
   args.GetReturnValue().SetNull();
}

void returnString(const FunctionCallbackInfo<Value> &args,Isolate *I,const string &s)
{
   args.GetReturnValue().Set(String::NewFromUtf8(I,s.c_str()));
}

void callback(const FunctionCallbackInfo<Value>& args,Isolate *I,const Local<Function> &cb,int argc,Local<Value> *argv)
{
  cb->Call(I->GetCurrentContext()->Global(),argc,argv);
}

void buildPrimitive(Isolate *I,const nj::Primitive &primitive,int index,Local<Value> *argv)
{
   switch(primitive.type()->getId())
   {
      case nj::null_type:
printf("arg is null\n");
         argv[index] = Null(I);
      break;
      case nj::boolean_type:
printf("arg is %d\n",primitive.toBoolean());
         argv[index] = Boolean::New(I,primitive.toBoolean());
      break;
      case nj::char_type:
printf("arg is %d\n",primitive.toChar());
         argv[index] = String::NewFromUtf8(I,primitive.toString().c_str());
      break;
      case nj::int64_type:
      case nj::int32_type:
      case nj::int16_type:
printf("arg is %lld\n",primitive.toInt());
         argv[index] = Number::New(I,primitive.toInt());
      break;
      case nj::uint64_type:
      case nj::uint32_type:
      case nj::uint16_type:
      case nj::uchar_type:
printf("arg is %lld\n",primitive.toUInt());
         argv[index] = Number::New(I,primitive.toUInt());
      break;
      case nj::float64_type:
      case nj::float32_type:
printf("arg is %f\n",primitive.toFloat());
         argv[index] = Number::New(I,primitive.toFloat());
      break;
      case nj::string_type:
printf("arg is %s\n",primitive.toString().c_str());
         argv[index] = String::NewFromUtf8(I,primitive.toString().c_str());
      break;
   }
}

Local<Array> buildArray(const shared_ptr<nj::Value> &value)
{
   const nj::Array_t *array_type = static_cast<const nj::Array_t*>(value->type());
   const nj::Type *element_type = array_type->getElementType();
   Isolate *I = Isolate::GetCurrent();
   EscapableHandleScope scope(I);

   switch(element_type->getId())
   {
      case nj::float64_type:
      {
         const nj::Array<double,nj::Float64_t> &array = static_cast<const nj::Array<double,nj::Float64_t>&>(*value);

         if(array.size() == 0) return Local<Array>();
         if(array.dims().size() == 1)
         {
            size_t size0 = array.dims()[0];
            double *p = array.ptr();
            Local<Array> dest = Array::New(I,size0);

            for(size_t i = 0;i < size0;i++) dest->Set(i,Number::New(I,p[i]));
            return scope.Escape(dest);
         }
         else if(array.dims().size() == 2)
         {
            size_t size0 = array.dims()[0];
            size_t size1 = array.dims()[1];
            double *p = array.ptr();
            Local<Array> dest = Array::New(I,size0);

            for(size_t i = 0;i < size0;i++)
            {
               Local<Array> row  = Array::New(I,size1);

               dest->Set(i,row);
               for(size_t j = 0;j < size1;j++) row->Set(j,Number::New(I,p[size0*j + i]));
            }
            return scope.Escape(dest);
         }
         return Local<Array>();
      }
      break;
      default: return Local<Array>(); break;
   }
}


int buildArgs(Isolate *I,const shared_ptr<vector<shared_ptr<nj::Value>>> &res,int argc,Local<Value> *argv)
{
   int index = 0;

   for(shared_ptr<nj::Value> value: *res)
   {
      if(value.get())
      {
         if(value->isPrimitive())
         {
            const nj::Primitive &primitive = static_cast<const nj::Primitive&>(*value);

            buildPrimitive(I,primitive,index++,argv);
         }
         else
         {
            argv[index++] = buildArray(value);
         }
      }
   }
   return index;
}


void doEval(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   int numArgs = args.Length();

   if(numArgs < 2 || !J)
   {
      returnNull(args,I);
      return;
   }

   Local<String> arg0 = args[0]->ToString();
   String::Utf8Value text(arg0);
   Local<Function> cb = Local<Function>::Cast(args[1]);
   JMain *engine;

   if(text.length() > 0 && (engine = J->getEngine()))
   {
      engine->evalQueuePut(*text);
      shared_ptr<vector<shared_ptr<nj::Value>>> res = engine->resultQueueGet();
  
      if(res.get())
      {
         int argc = res->size();
         Local<Value> *argv = new Local<Value>[argc];
         argc = buildArgs(I,res,argc,argv);
         callback(args,I,cb,argc,argv);
      }
   }
   else
   {
      const unsigned argc = 1;
      Local<Value> argv[argc] = { String::NewFromUtf8(I,"") };
      callback(args,I,cb,argc,argv);
   }
}


void doStart(const FunctionCallbackInfo<Value> &args)
{
   Isolate *I = Isolate::GetCurrent();
   HandleScope scope(I);
   int numArgs = args.Length();

   if(numArgs == 0)
   {
      returnString(args,I,"");
      return;
   }

   Local<String> arg0 = args[0]->ToString();
   String::Utf8Value plainText_av(arg0);

   if(plainText_av.length() > 0)
   {
      if(!J) J = new JuliaExecEnv(*plainText_av);

      returnString(args,I,"Julia Started");
   }
   else returnString(args,I,"");
}

void init(Handle<Object> exports)
{
  NODE_SET_METHOD(exports,"start",doStart);
  NODE_SET_METHOD(exports,"eval",doEval);
}

NODE_MODULE(nj,init)
