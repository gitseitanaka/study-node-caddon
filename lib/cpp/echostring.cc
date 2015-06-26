#include <nan.h>
#include <map>

using namespace v8;

#define _DEBUG_PRINT
#ifdef _DEBUG_PRINT
#include <iostream>
#include <thread>
static void debug_taskid( const char* aName, const char* aTab) {
	std::cout << aName << aTab << std::this_thread::get_id() << std::endl;
}
#define DBPRINT(name, tab) debug_taskid((name), (tab))
#else
#define DBPRINT(name, tab)
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
	  _count(aCount),
	  _workerid(AsyncWorker::shareworkerid++),
	  _requestedAbort(false)
	{
		NanScope();
		
		workerpool.insert( std::make_pair( _workerid, this ) );
	}
	//----------------------
	// デストラクタ
	// [v8 conntext]
	virtual ~AsyncWorker() {
		DBPRINT(__FUNCTION__, "\t\t\t");
		NanScope();
		
		workerpool.erase(_workerid);
	}

	//----------------------
	// 非同期処理ループ
	// [           ]
	virtual void Execute (const NanAsyncProgressWorker::ExecutionProgress& aProgress) {
		DBPRINT(__FUNCTION__, "--\t\t\t\t");
		for (int i = 0; i < _count; ++i) {
			if (_requestedAbort) { break; }
			
			aProgress.Send(reinterpret_cast<const char*>(&i), sizeof(int));
			DBPRINT(__FUNCTION__, "\t\t\t\t");
			
			//-----------------
			Sleep(_interval);
			//-----------------
		}
		DBPRINT(__FUNCTION__, "--\t\t\t\t");
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

	//----------------------
	// id取得
	int WorkerId() const { return _workerid; }
	
	//----------------------
	// 中断指示
	//   静的メンバ関数
	// [v8 conntext]
	static void Abort(int aWorkerId) {
		DBPRINT(__FUNCTION__, "\t\t\t\t");
		std::map<int, AsyncWorker*>::iterator ite = workerpool.find(aWorkerId);
		if (workerpool.end() != ite) {
			(*ite).second->AbortRequest();
			DBPRINT(__FUNCTION__, "--\t\t\t\t");
		}
	}

protected:
	//----------------------
	// 中断要求
	// [v8 conntext]
	void AbortRequest() {
		_requestedAbort = true;
	}

private:
	NanCallback* progress;		// progress callback
	int _interval;				// interval
	int _count;					// count
	
	int _workerid;				// worker id
	bool _requestedAbort;		// "abort" is requested.
	
								// global workerpool
	static std::map<int, AsyncWorker*> workerpool;
	static int shareworkerid;	// global workerid
};
int AsyncWorker::shareworkerid = 0;
std::map<int, AsyncWorker*> AsyncWorker::workerpool;

//----------------------
// java scriptからの関数コール
// [v8 conntext]
NAN_METHOD(asyncCommand) {
	DBPRINT(__FUNCTION__, "\t\t\t\t\t");

	NanScope();

	NanCallback* progress = new NanCallback(args[2].As<v8::Function>());
	NanCallback* callback = new NanCallback(args[3].As<v8::Function>());

	// "worker" will be destroyed by v8.
	AsyncWorker* worker = new AsyncWorker(
			progress,					// progress callback
			callback,					// finish callback
			args[1]->Uint32Value(),		// count
			args[0]->Uint32Value());	// interval
	NanAsyncQueueWorker(worker);
	
	NanReturnValue(NanNew<Integer>(worker->WorkerId()));
}


//----------------------
// java scriptからの関数コール
// [v8 conntext]
NAN_METHOD(asyncAbortCommand) {
	DBPRINT(__FUNCTION__, "\t\t\t\t");

	NanScope();
	
	AsyncWorker::Abort(args[0]->Uint32Value());
	
	NanReturnUndefined();
}


//---------------------------
// v8へのbind
void Init(Handle<Object> exports) {
	exports->Set(NanNew("async"), NanNew<FunctionTemplate>(asyncCommand)->GetFunction());
	exports->Set(NanNew("abort"), NanNew<FunctionTemplate>(asyncAbortCommand)->GetFunction());
}
NODE_MODULE(echostring, Init)
