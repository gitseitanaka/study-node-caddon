#include <nan.h>
#include <uv.h>
#include <map>
#include <vector>
#include <string>
#include <thread>

using namespace v8;

#define _DEBUG_PRINT
#ifdef _DEBUG_PRINT
#include <iostream>
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
	enum ArgIndex{
		ArgSettingFilePath = 0,
		ArgInterval,
		ArgCbProgress,
		ArgCbFinish,
	};
	//----------------------
	// コンストラクタ
	// [v8 conntext]
	AsyncWorker(
		NanCallback* aProgressCallback,		// progress callback
		NanCallback* aCallback,				// finish callback
		std::string& aSettingPath,			// setting file path
		int aInterval )						// interval
    : NanAsyncProgressWorker(aCallback), progress(aProgressCallback),
	  _setting_filepath(aSettingPath),
	  _interval(aInterval),
	  _count(0),
	  _workerid(AsyncWorker::shareworkerid++),
	  _requestedAbort(false)
	{
		DBPRINT("[Enter]" __FUNCTION__, "\t\t\t");
		memset(&_wait_obj, 0, sizeof(_wait_obj));
		memset(&_tick_timer, 0, sizeof(_tick_timer));


		std::cout << _setting_filepath << std::endl;

		uv_sem_init(&_wait_obj, 0);
		uv_timer_init(uv_default_loop(), &_tick_timer);

		NanScope();
//		SaveToPersistent("path", aPathHandle);

		workerpool.insert( std::make_pair( _workerid, this ) );
		DBPRINT("[Exit ]" __FUNCTION__, "\t\t\t");
	}
	//----------------------
	// デストラクタ
	// [v8 conntext]
	virtual ~AsyncWorker() {
		DBPRINT("[Enter]" __FUNCTION__, "\t\t");
		uv_sem_destroy(&_wait_obj);
		NanScope();

		workerpool.erase(_workerid);
		DBPRINT("[Exit ]" __FUNCTION__, "\t\t");
	}

	//----------------------
	// 非同期処理ループ
	// [           ]
	virtual void Execute (const NanAsyncProgressWorker::ExecutionProgress& aProgress) {
		DBPRINT("[Enter]" __FUNCTION__, "--\t\t\t");


		_tick_timer.data = (void*)this;
		uv_timer_start(&_tick_timer, Tick_timer_cb, 0, _interval);

		while(!_requestedAbort) {
			//			aProgress.Send(reinterpret_cast<const char*>(&i), sizeof(int));
			uv_sem_wait(&_wait_obj);
		}
		DBPRINT("[Exit ]" __FUNCTION__, "--\t\t\t");
	}

	//----------------------
	// 経過通知
	//   "Execute"内の"aProgress.Send()"によりメッセージング
	// [v8 conntext]
	virtual void HandleProgressCallback(const char *data, size_t size) {
		DBPRINT("[Enter]" __FUNCTION__, "\t\t");

		NanScope();
		Local<Value> argv[] = {
		    NanNew<Integer>(*reinterpret_cast<int*>(const_cast<char*>(data)))
		};
		progress->Call(1, argv);
		DBPRINT("[Exit ]" __FUNCTION__, "\t\t");
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
		DBPRINT("[Enter]" __FUNCTION__, "\t\t\t");
		std::map<int, AsyncWorker*>::iterator ite = workerpool.find(aWorkerId);
		if (workerpool.end() != ite) {
			(*ite).second->AbortRequest();
			DBPRINT(__FUNCTION__, "--\t\t\t\t");
		}
		DBPRINT("[Exit ]" __FUNCTION__, "\t\t\t");
	}


protected:
	//----------------------
	// 中断要求
	// [v8 conntext]
	void AbortRequest() {
		DBPRINT("[Enter]" __FUNCTION__, "\t\t");
		_requestedAbort = true;
		uv_timer_stop(&_tick_timer);
		uv_unref((uv_handle_t*)&_tick_timer);
		uv_sem_post(&_wait_obj);
		DBPRINT("[Exit ]" __FUNCTION__, "\t\t");
	}
private:
	//----------------------
	// Tick処理
	// [v8 conntext]
	void Tick() {
		DBPRINT("[Enter]" __FUNCTION__, "\t\t\t");
		if (!_requestedAbort) {
			DBPRINT("[Trace]" __FUNCTION__, "\t\t\t");
			uv_timer_again(&_tick_timer);
		}
		DBPRINT("[Exit ]" __FUNCTION__, "\t\t\t");
	}
	//----------------------
	// Tick処理
	// [v8 conntext]
	static void Tick_timer_cb(uv_timer_t* aHandle) {
		DBPRINT("[Enter]" __FUNCTION__, "\t\t");
		if (0 != uv_is_active((uv_handle_t*) aHandle)) { // non-zero is active
			AsyncWorker* instance = reinterpret_cast<AsyncWorker*>(aHandle->data);
			instance->Tick();
		}
		DBPRINT("[Exit ]" __FUNCTION__, "\t\t");
	}

private:
	NanCallback* progress;		// progress callback
	int _interval;				// interval
	int _count;					// count

								// file path
	std::string _setting_filepath;
								// name array
	std::vector<std::string> _namearray;

	int _workerid;				// worker id
	bool _requestedAbort;		// "abort" is requested.

	uv_sem_t _wait_obj;			// wait object for worker
	uv_timer_t _tick_timer;		// tick timer

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
	DBPRINT("[V8   ]" __FUNCTION__, "\t\t\t\t");
	NanScope();

	NanCallback* progress = new NanCallback(
				args[AsyncWorker::ArgCbProgress].As<v8::Function>());
	NanCallback* callback = new NanCallback(
				args[AsyncWorker::ArgCbFinish].As<v8::Function>());
	NanUtf8String* filename = new NanUtf8String(
				args[AsyncWorker::ArgSettingFilePath]);

	// "worker" will be destroyed by v8.
	AsyncWorker* worker = new AsyncWorker(
			progress,					// progress callback
			callback,					// finish callback
										// filename
			std::string(filename->operator*()),
										// interval
			args[AsyncWorker::ArgInterval]->Uint32Value());
	NanAsyncQueueWorker(worker);

	NanReturnValue(NanNew<Integer>(worker->WorkerId()));
}


//----------------------
// java scriptからの関数コール
// [v8 conntext]
NAN_METHOD(asyncAbortCommand) {
	DBPRINT("[V8   ]" __FUNCTION__, "\t\t\t");

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
