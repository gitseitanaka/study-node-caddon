#include <nan.h>

using namespace v8;

#define _DEBUG_PRINT
#ifdef _DEBUG_PRINT
#include <iostream>
#include <thread>
static void debug_taskid( const char* aName, const char* aTab) {
	std::cout << aName << aTab << std::this_thread::get_id() << std::endl;
}
#define DBPRINT(x, y) debug_taskid((x), (y))
#else
#define DBPRINT(x, y)
#endif

//------------------------------------------
//
// 非同期Worker(経過通知版)
//
class AsyncWorker : public NanAsyncProgressWorker {
public:
	//----------------------
	// コンストラクタ
	// [v8 conntext]
	AsyncWorker(
		NanCallback* aProgressCallback,		// progress callback
		NanCallback* aCallback,				// finish callback
		int aCount,							// count
		int aInterval )						// interval
    : NanAsyncProgressWorker(aCallback), progress(aProgressCallback),
	  _interval(aInterval),
	  _count(aCount)
	{
		NanScope();
	}
	//----------------------
	// デストラクタ
	// [v8 conntext]
	virtual ~AsyncWorker() {
		DBPRINT(__FUNCTION__, "\t\t\t");
		NanScope();
	}

	//----------------------
	// 非同期処理ループ
	// [           ]
	virtual void Execute (const NanAsyncProgressWorker::ExecutionProgress& aProgress) {
		DBPRINT(__FUNCTION__, "\t\t\t\t");
		for (int i = 0; i < _count; ++i) {
			aProgress.Send(reinterpret_cast<const char*>(&i), sizeof(int));
			DBPRINT(__FUNCTION__, "\t\t\t\t");
	
			//-----------------
			Sleep(_interval);
			//-----------------
		}
	}
	
	//----------------------
	// 経過通知
	//   "Execute"内の"aProgress.Send()"によりメッセージング
	// [v8 conntext]
	virtual void HandleProgressCallback(const char *data, size_t size) {
		DBPRINT(__FUNCTION__, "\t\t");

		NanScope();
		Local<Value> argv[] = {
		    NanNew<Integer>(*reinterpret_cast<int*>(const_cast<char*>(data)))
		};
		progress->Call(1, argv);
	}
	
	//----------------------
	// 完了通知
	// [v8 conntext]
	virtual void HandleOKCallback () {
		DBPRINT(__FUNCTION__, "\t\t\t");
		NanScope();
		Local<Value> argv[] = { NanNew<Integer>(_count) };
		
		callback->Call(1, argv);
	}

private:
	NanCallback* progress;		// progress callback
	int _interval;				// interval
	int _count;					// count

};


//----------------------
// java scriptからの関数コール
// [v8 conntext]
NAN_METHOD(asyncCommand) {
	DBPRINT(__FUNCTION__, "\t\t\t\t\t");

	NanScope();

	NanCallback* progress = new NanCallback(args[2].As<v8::Function>());
	NanCallback* callback = new NanCallback(args[3].As<v8::Function>());
	NanAsyncQueueWorker(new AsyncWorker(
		progress,					// progress callback
		callback,					// finish callback
		args[1]->Uint32Value(),		// count
		args[0]->Uint32Value()));	// interval
	
	NanReturnUndefined();
}

//---------------------------
// v8へのbind
void Init(Handle<Object> exports) {
	exports->Set(NanNew("async"), NanNew<FunctionTemplate>(asyncCommand)->GetFunction());

}
NODE_MODULE(timercb, Init)
