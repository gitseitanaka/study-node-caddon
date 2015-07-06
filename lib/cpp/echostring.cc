#include <nan.h>
#include <uv.h>

#include <thread>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

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
	//----------------------
	// argments index.
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

		uv_sem_init(&_wait_obj, 0);
		uv_timer_init(uv_default_loop(), &_tick_timer);

		NanScope();

		workerpool.insert( std::make_pair( _workerid, this ) );
		DBPRINT("[Exit ]" __FUNCTION__, "\t\t\t");
	}
	//----------------------
	// デストラクタ
	// [v8 conntext]
	virtual ~AsyncWorker() {
		DBPRINT("[Enter]" __FUNCTION__, "\t\t");
		uv_sem_destroy(&_wait_obj);
#if 0
		auto dump = [](std::vector<std::string>& v) {
				for(auto i:v) {
					std::cout << i << " ";
			}
                std::cout << std::endl;
        };
        dump(_stringArray);
#endif
		NanScope();

		workerpool.erase(_workerid);
		DBPRINT("[Exit ]" __FUNCTION__, "\t\t");
	}

	//----------------------
	// 非同期処理ループ
	// [non-v8     ]
	virtual void Execute (const NanAsyncProgressWorker::ExecutionProgress& aProgress) {
		DBPRINT("[Enter]" __FUNCTION__, "--\t\t\t");

		readFile(_setting_filepath, _stringArray);

		_tick_timer.data = (void*)this;
		uv_timer_start(&_tick_timer, Tick_timer_cb, 0, _interval);

		while(!_requestedAbort) {
			uv_sem_wait(&_wait_obj);
			aProgress.Send(reinterpret_cast<const char*>(_stringArray[0].c_str()),
			 			   _stringArray[0].length());

			rotate(_stringArray.begin(), _stringArray.begin()+1, _stringArray.end());
		}
		DBPRINT("[Exit ]" __FUNCTION__, "--\t\t\t");
	}

	//----------------------
	// 経過通知
	//   "Execute"内の"aProgress.Send()"によりメッセージング
	// [v8 conntext]
	virtual void HandleProgressCallback(const char* data, size_t size) {
		DBPRINT("[Enter]" __FUNCTION__, "\t");

		NanScope();
		std::string str(data, size);
		Local<Value> argv[] = {
			NanNew<Integer>(WorkerId()),
			NanNew<String>(const_cast<char*>(str.c_str()))
		};
		progress->Call(2, argv);
		DBPRINT("[Exit ]" __FUNCTION__, "\t");
	}

	//----------------------
	// 完了通知
	// [v8 conntext]
	virtual void HandleOKCallback () {
		DBPRINT(__FUNCTION__, "\t\t\t");
		NanScope();
		Local<Value> argv[] = { NanNew<Integer>(WorkerId()) };

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
	// lines(file)->vector
	static void readFile(
				const std::string& aFilePath,
				std::vector<std::string>& oList)
	{
		std::ifstream instream( aFilePath.c_str(), std::ios::in );
		if ( instream ) {
			for (;;) {
				std::string readstring;
				getline( instream, readstring );
				if ( instream.eof() )
				{
					break;
				}
				else
				{
					std::string str = Trim(std::string(readstring));
					oList.push_back(str);
				}
			}
			instream.close();
		}
	}

	//----------------------
	// Trim
	static std::string Trim(const std::string& aString,
					const char* aTrimCharacterList = " \t\v\r\n")
	{
		std::string result;
		std::string::size_type left = aString.find_first_not_of(aTrimCharacterList);
		if (left != std::string::npos)
		{
			std::string::size_type right = aString.find_last_not_of(aTrimCharacterList);
			result = aString.substr(left, right - left + 1);
		}
		return result;
	}

	//----------------------
	// Tick処理
	// [v8 conntext]
	void Tick() {
		DBPRINT("[Enter]" __FUNCTION__, "\t\t\t");
		if (!_requestedAbort) {
			DBPRINT("[Trace]" __FUNCTION__, "\t\t\t");
			uv_timer_again(&_tick_timer);
		}
		uv_sem_post(&_wait_obj);
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
								// string array
	std::vector<std::string> _stringArray;

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
