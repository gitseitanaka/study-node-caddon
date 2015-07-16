#ifndef __ECHOSTRING_H__
#define __ECHOSTRING_H__

#include <nan.h>
#include <uv.h>

#include <map>
#include <queue>
#include <string>


//------------------------------------------
//
// AsyncWorker(notify progress)
//
class AsyncWorker {
	//----------------------
	// forward declaration
	class Request;
	//----------------------
	// argments index.
	enum AsyncMsgType{
		TimerStart = 0,
		Progress,
		Exit
	};

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
		int aInterval );					// interval
	//----------------------
	// Destructor
	// [v8 context ]
	virtual ~AsyncWorker();

	//----------------------
	// Execute loop
	// [non-v8     ]
	virtual void ExecuteLoop();

public:
	//----------------------
	// Notify progress
	//   messaging form "aProgress.Send()" in "Execute()"
	// [v8 context ]
	virtual void ProgressCallback(std::string& aString);

	//----------------------
	// Notify finish
	// [v8 context ]
	virtual void FinishedCallback();

	//----------------------
	// Get worker id
	inline int WorkerId() const { return _workerid; }

private:
	//----------------------
	// Send Message to v8 context
	//   "aRequest" will be free by internal process.
	// [non-v8     ]
	void SendAsyncMessage(Request* aRequest);
	
	//----------------------
	// Abort Request
	// [v8 context ]
	void AbortRequest();
	
	//----------------------
	// Tick
	// [v8 context ]
	void Tick();
	
	//----------------------
	// Timer Start
	// [v8 context ]
	void AsyncMsg();
	
	//----------------------
	// Start
	// [v8 context ]
	virtual void Start();
	
	//----------------------
	// Destroy
	// [v8 context ]
	virtual void Destroy();

//======================
//Static menbers.
public:
	//----------------------
	// form java script
	// [v8 context ]
	static NAN_METHOD(asyncCommand);
	
	//----------------------
	// form java script
	// [v8 context ]
	static NAN_METHOD(asyncAbortCommand);
	
private:
	//----------------------
	// Abort
	// [v8 context ]
	static void Abort(int aWorkerId);
	
	//----------------------
	// Tick cb
	// [v8 context ]
#if (NODE_MODULE_VERSION < NODE_0_12_MODULE_VERSION)
	static void Tick_timer_cb(uv_timer_t* aHandle, int/*UNUSED*/);
#else
	static void Tick_timer_cb(uv_timer_t* aHandle);
#endif

	//----------------------
	// Request cb to start timer
	// [v8 context ]
#if (NODE_MODULE_VERSION < NODE_0_12_MODULE_VERSION)
	static void RequestAsyncMsg(uv_async_t* aHandle, int/*UNUSED*/);
#else
	static void RequestAsyncMsg(uv_async_t* aHandle);
#endif
	
	//----------------------
	// Handle closed cb
	// [v8 context ]
	static void CloseCb(uv_handle_t* aHandle);
	
	//----------------------
	// Execute loop
	// [non-v8     ]
	static void DoWork(void* aObject);
	
	//----------------------
	// lines(file)->vector
	static void ReadFile(
				const std::string& aFilePath,
				std::queue<std::string>& oList);
	
	//----------------------
	// Trim
	// http://program.station.ez-net.jp/special/handbook/cpp/string/trim.asp
	static std::string Trim(const std::string& aString,
					const char* aTrimCharacterList = " \t\v\r\n");


//======================
// attributes.
private:
	NanCallback* _progress;		// progress callback
	NanCallback* _finish;		// finish call back callback
	int _interval;				// interval

								// file path
	std::string _setting_filepath;
								// string queue
	std::queue<std::string> _stringPool;

	int _workerid;				// worker id
    bool _requestedAbort;       // "abort" is requested.

	uv_sem_t _wait_tick;		// wait object for tick

	uv_mutex_t _mutex;			// mutex

	uv_timer_t _tick_handle;	// tick timer handle
	uv_async_t _async_handle;	// async handle
	int _handle_count;			// handle count

	uv_thread_t _worker_handle;	// worker thread
								
								// internal message(async) queue
	std::queue<Request*>		_msg_queue;

	//----------------------
	// internal request define
	class Request {
	public:
		Request(AsyncMsgType aMsgType, std::string& aString);
		Request(AsyncMsgType aMsgType);
		~Request();
		inline AsyncMsgType msgType() { return _msgType; }
		inline std::string& string() { return _string; }
	private:
		AsyncMsgType _msgType;
		std::string _string;
	};

	//----------------------
	// static members
								// global workerpool
	static std::map<int, AsyncWorker*> workerpool;
	static int shareworkerid;	// global workerid
};

#endif //__ECHOSTRING_H__
