#include <nan.h>
#include <uv.h>

#include <thread>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

using namespace v8;

//#define _DEBUG_PRINT
#ifdef _DEBUG_PRINT

#if !(defined(_MSC_VER))
#define __FUNCTION__ __func__
#endif
#include <sstream>
#include <iostream>
#include <iomanip>
#define DEBUG_TABSPACE (50)
static void debug_taskid( const char* aTag, const char* aName) {
	std::stringstream stream;
	stream << aTag << aName << std::setw(DEBUG_TABSPACE) << std::setfill(' ') << ' ';
	std::cout << stream.str().substr(0, DEBUG_TABSPACE) << std::this_thread::get_id() << std::endl;
}
#define DBPRINT(tag, name) debug_taskid((tag), (name))
#else
#define DBPRINT(tag, name)
#endif


//------------------------------------------
//
// AsyncWorker(notify progress)
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
	// Constructor
	// [v8 context ]
	AsyncWorker(
		NanCallback* aProgressCallback,		// progress callback
		NanCallback* aCallback,				// finish callback
		std::string  aSettingPath,			// setting file path
		int aInterval )						// interval
    : NanAsyncProgressWorker(aCallback), _progress(aProgressCallback),
	  _interval(aInterval),
	  _setting_filepath(aSettingPath),
	  _workerid(AsyncWorker::shareworkerid++),
	  _requestedAbort(false)
	{
		DBPRINT("[Enter]", __FUNCTION__);

		memset(&_wait_tick, 0, sizeof(_wait_tick));
		memset(&_wait_handle_close, 0, sizeof(_wait_handle_close));
		uv_sem_init(&_wait_tick, 0);
		uv_sem_init(&_wait_handle_close, 0);

		memset(&_tick_timer, 0, sizeof(_tick_timer));
		memset(&_async_handle, 0, sizeof(_async_handle));
		uv_loop_t* loop = uv_default_loop();
		uv_timer_init(loop, &_tick_timer);
		uv_async_init(loop, &_async_handle, RequestAsyncMsg);

		_handle_pool.push_back((uv_handle_t*)(&_tick_timer));
		_handle_pool.push_back((uv_handle_t*)(&_async_handle));

		std::vector<uv_handle_t*>::iterator ite = _handle_pool.begin();
		while (ite != _handle_pool.end()) {
			(*ite)->data = (void*)this;
			ite++;
		}

		workerpool.insert( std::make_pair( _workerid, this ) );
		DBPRINT("[Exit ]", __FUNCTION__);
	}
	//----------------------
	// Destructor
	// [v8 context ]
	virtual ~AsyncWorker() {
		DBPRINT("[Enter]", __FUNCTION__);
		uv_sem_destroy(&_wait_tick);
		uv_sem_destroy(&_wait_handle_close);
		workerpool.erase(_workerid);
#if 0
		auto dump = [](std::vector<std::string>& v) {
				for(auto i:v) {
					std::cout << i << " ";
			}
                std::cout << std::endl;
        };
        dump(_stringArray);
#endif
		DBPRINT("[Exit ]", __FUNCTION__);
	}

	//----------------------
	// Execute loop
	// [non-v8     ]
	virtual void Execute (const NanAsyncProgressWorker::ExecutionProgress& aProgress) {
		DBPRINT("[Enter]", __FUNCTION__);

		readFile(_setting_filepath, _stringArray);

		uv_async_send(&_async_handle);	// send timer start
		
		// start tick loop
		while(!_requestedAbort) {
			uv_sem_wait(&_wait_tick);
			if (_requestedAbort) { break; }

			if (!_stringArray.empty()){
				aProgress.Send(reinterpret_cast<const char*>(_stringArray[0].c_str()),
							_stringArray[0].length());

				rotate(_stringArray.begin(), _stringArray.begin() + 1, _stringArray.end());
			}
		}

		// wait handles close after exited tick loop.
		DBPRINT("[Trace]", __FUNCTION__);
		std::vector<uv_handle_t*>::iterator ite = _handle_pool.begin();
		while (ite != _handle_pool.end()) {
			uv_sem_wait(&_wait_handle_close);
			ite++;
		}
		DBPRINT("[Exit ]", __FUNCTION__);
	}

	//----------------------
	// Notify progress
	//   messaging form "aProgress.Send()" in "Execute()"
	// [v8 context ]
	virtual void HandleProgressCallback(const char* data, size_t size) {
		DBPRINT("[Enter]", __FUNCTION__);

		NanScope();
		std::string str(data, size);
		Local<Value> argv[] = {
			NanNew<Integer>(WorkerId()),
			NanNew<String>(const_cast<char*>(str.c_str()))
		};
		const int argc = 2;
		_progress->Call(argc, argv);
		DBPRINT("[Exit ]", __FUNCTION__);
	}

	//----------------------
	// Notify finish
	// [v8 context ]
	virtual void HandleOKCallback () {
		DBPRINT("[Enter]", __FUNCTION__);
		NanScope();
		Local<Value> argv[] = { NanNew<Integer>(WorkerId()) };
		const int argc = 1;
		callback->Call(argc, argv);
		DBPRINT("[Exit ]", __FUNCTION__);
	}

	//----------------------
	// Get worker id
	int WorkerId() const { return _workerid; }

protected:
		//----------------------
		// Abort Request
		// [v8 context ]
		void AbortRequest() {
			DBPRINT("[Enter]", __FUNCTION__);
			_requestedAbort = true;
			uv_timer_stop(&_tick_timer);
			uv_unref((uv_handle_t*)&_tick_timer);
			uv_close((uv_handle_t*)&_tick_timer, close_cb);
			
			uv_sem_post(&_wait_tick);
			DBPRINT("[Exit ]", __FUNCTION__);
		}

private:
		//----------------------
		// Tick
		// [v8 context ]
		void Tick() {
			DBPRINT("[Enter]", __FUNCTION__);
			if (!_requestedAbort) {
				DBPRINT("[Trace]", __FUNCTION__);
				uv_timer_again(&_tick_timer);
			}
			uv_sem_post(&_wait_tick);
			DBPRINT("[Exit ]", __FUNCTION__);
		}
		//----------------------
		// Timer Start
		// [v8 context ]
		void AsyncMsg() {
			DBPRINT("[Enter]", __FUNCTION__);
			if (!_requestedAbort) {
				uv_timer_start(&_tick_timer, Tick_timer_cb, _interval, _interval);
			}
			uv_close((uv_handle_t*)&_async_handle, close_cb);
			DBPRINT("[Exit ]", __FUNCTION__);
		}

//======================
//Static menbers.
public:
	//----------------------
	// Abort
	// [v8 context ]
	static void Abort(int aWorkerId) {
		DBPRINT("[Enter]", __FUNCTION__);
		std::map<int, AsyncWorker*>::iterator ite = AsyncWorker::workerpool.find(aWorkerId);
		if (workerpool.end() != ite) {
			(*ite).second->AbortRequest();
			DBPRINT("[Trace]", __FUNCTION__);
		}
		DBPRINT("[Exit ]", __FUNCTION__);
	}


private:
	//----------------------
	// Tick cb
	// [v8 context ]
#if (NODE_MODULE_VERSION < NODE_0_12_MODULE_VERSION)
	static void Tick_timer_cb(uv_timer_t* aHandle, int/*UNUSED*/) {
#else
	static void Tick_timer_cb(uv_timer_t* aHandle) {
#endif
		DBPRINT("[Enter]", __FUNCTION__);
		if (0 != uv_is_active((uv_handle_t*) aHandle)) { // non-zero is active
			AsyncWorker* instance = reinterpret_cast<AsyncWorker*>(aHandle->data);
			instance->Tick();
		}
		DBPRINT("[Exit ]", __FUNCTION__);
	}
	//----------------------
	// Request cb to start timer
	// [v8 context ]
#if (NODE_MODULE_VERSION < NODE_0_12_MODULE_VERSION)
	static void RequestAsyncMsg(uv_async_t* aHandle, int/*UNUSED*/) {
#else
	static void RequestAsyncMsg(uv_async_t* aHandle) {
#endif
		DBPRINT("[Enter]", __FUNCTION__);
		if (0 != uv_is_active((uv_handle_t*)aHandle)) { // non-zero is active
			AsyncWorker* instance = reinterpret_cast<AsyncWorker*>(aHandle->data);
			instance->AsyncMsg();
		}
		DBPRINT("[Exit ]", __FUNCTION__);
	}

	//----------------------
	// Handle closed cb
	static void close_cb(uv_handle_t* aHandle) {
		DBPRINT("[Enter]", __FUNCTION__);
		AsyncWorker* instance = reinterpret_cast<AsyncWorker*>(aHandle->data);
		uv_sem_post(&(instance->_wait_handle_close));
		DBPRINT("[Exit ]", __FUNCTION__);
	}

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
					if (0 != str.length()) {
						oList.push_back(str);
					}
				}
			}
			instream.close();
		}
	}

	//----------------------
	// Trim
	// http://program.station.ez-net.jp/special/handbook/cpp/string/trim.asp
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

//======================
// attributes.
private:
	NanCallback* _progress;		// progress callback
	int _interval;				// interval

								// file path
	std::string _setting_filepath;
								// string array
	std::vector<std::string> _stringArray;

	int _workerid;				// worker id
    bool _requestedAbort;       // "abort" is requested.

	uv_sem_t _wait_tick;		// wait object for tick
	uv_sem_t _wait_handle_close;// wait object for handles close

	uv_timer_t _tick_timer;		// tick timer
	uv_async_t _async_handle;	// async handle

								// for handles close
	std::vector<uv_handle_t*> _handle_pool;

								// global workerpool
	static std::map<int, AsyncWorker*> workerpool;
	static int shareworkerid;	// global workerid
};
int AsyncWorker::shareworkerid = 0;
std::map<int, AsyncWorker*> AsyncWorker::workerpool;

//----------------------
// form java script
// [v8 context ]
NAN_METHOD(asyncCommand) {
	DBPRINT("[V8   ]", __FUNCTION__);

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
// form java script
// [v8 context ]
NAN_METHOD(asyncAbortCommand) {
	DBPRINT("[V8   ]", __FUNCTION__);

	NanScope();

	AsyncWorker::Abort(args[0]->Uint32Value());

	NanReturnUndefined();
}


//---------------------------
// bind to v8
void Init(Handle<Object> exports) {
	exports->Set(NanNew("echoStringCyclic"),
				NanNew<FunctionTemplate>(asyncCommand)->GetFunction());
	exports->Set(NanNew("echoStringCyclicAbort"),
				NanNew<FunctionTemplate>(asyncAbortCommand)->GetFunction());
}
NODE_MODULE(echostring, Init)
