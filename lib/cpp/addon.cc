#include "AsyncWorker.h"

//---------------------------
// export to v8 world
void Init(v8::Handle<v8::Object> aExports) {

	AsyncWorker::Init(aExports);
	// xxx::Init()
	// xxx::Init()
	// xxx::Init()
}
NODE_MODULE(studyechostring, Init)
