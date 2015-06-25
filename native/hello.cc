#include <node.h>
#include <nan.h>

using namespace v8;

NAN_METHOD(Method) {
	NanScope();

	NanReturnValue(NanNew<String>("world"));
}

void init(Handle<Object> exports) {

  exports->Set(NanNew("hello"), NanNew<FunctionTemplate>(Method)->GetFunction());

}

NODE_MODULE(addon, init)
