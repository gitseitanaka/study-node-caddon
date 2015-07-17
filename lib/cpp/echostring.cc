#include "echostring.h"

#include <thread>
#include <fstream>
#include <algorithm>

using namespace v8;

#define _DEBUG_PRINT
#ifdef _DEBUG_PRINT

#if !(defined(_MSC_VER))
#define __FUNCTION__ __func__
#endif
#include <sstream>
#include <iostream>
#include <iomanip>
#define DEBUG_TABSPACE (50)
void debug_taskid( const char* aTag, const char* aName) {
	std::stringstream stream;
	stream << aTag << aName << std::setw(DEBUG_TABSPACE) << std::setfill(' ') << ' ';
	std::cout << stream.str().substr(0, DEBUG_TABSPACE) << std::this_thread::get_id() << std::endl;
}
#include <stdio.h>
#include <stdlib.h>
//#define FOUTDBG(...) fprintf( stderr, __VA_ARGS__ )
#define FOUTDBG(...) fprintf( stdout, __VA_ARGS__ )
#define DBPRINT(tag, name) debug_taskid((tag), (name))
#define DBPRINTA(...)  FOUTDBG(__VA_ARGS__)
#else
#define DBPRINT(tag, name)
#define DBPRINTA(...)
#endif


//----------------------
// Constructor
// [v8 context ]
AsyncWorker::AsyncWorker(
	NanCallback* aProgressCallback,		// progress callback
	NanCallback* aCallback,				// finish callback
	std::string  aSettingPath,			// setting file path
	int aInterval )						// interval
	: node::ObjectWrap(),
  _progress(aProgressCallback), _finish(aCallback),
  _interval(aInterval),
  _setting_filepath(aSettingPath),
  _workerid(AsyncWorker::shareworkerid++),
  _requestedAbort(false),
  _handle_count(0)
{
	DBPRINT("[Enter]", __FUNCTION__);
	DBPRINTA("[Trace]%s\n", __FUNCTION__);

	memset(&_wait_tick, 0, sizeof(_wait_tick));
	uv_sem_init(&_wait_tick, 0);

	memset(&_mutex, 0, sizeof(_mutex));
	uv_mutex_init(&_mutex);

	uv_loop_t* loop = uv_default_loop();
	memset(&_tick_handle, 0, sizeof(_tick_handle));
	memset(&_async_handle, 0, sizeof(_async_handle));
	uv_timer_init(loop, &_tick_handle);
	uv_async_init(loop, &_async_handle, RequestAsyncMessageCb);
	_tick_handle.data = (void*)this;		_handle_count++;
	_async_handle.data = (void*)this;	_handle_count++;

	DBPRINT("[Exit ]", __FUNCTION__);
}
//----------------------
// Destructor
// [v8 context ]
AsyncWorker::~AsyncWorker() {
	DBPRINT("[Enter]", __FUNCTION__);
	uv_mutex_destroy(&_mutex);
	uv_sem_destroy(&_wait_tick);

#if 0
	auto dump = [](std::vector<std::string>& v) {
			for(auto i:v) {
				std::cout << i << " ";
		}
            std::cout << std::endl;
    };
    dump(_stringPool);
#endif
	DBPRINT("[Exit ]", __FUNCTION__);
}

//----------------------
// Execute loop
// [non-v8     ]
void AsyncWorker::ExecuteLoop() {
	DBPRINT("[Enter]", __FUNCTION__);

	ReadFile(_setting_filepath, _stringPool);

	// require timer start
	SendAsyncMessage(new Request(AsyncMsgType::TimerStart));

	// start tick loop
	while (!_requestedAbort) {
		uv_sem_wait(&_wait_tick);
		if (_requestedAbort) { break; }

		if (!_stringPool.empty()){
			// require notify progress
			SendAsyncMessage(new Request(AsyncMsgType::Progress, _stringPool.front()));
			_stringPool.push(_stringPool.front());
			_stringPool.pop();
		}
	}

	// require handle close
	SendAsyncMessage(new Request(AsyncMsgType::Exit));
}

//----------------------
// Notify progress
// [v8 context ]
void AsyncWorker::ProgressCallback(std::string& aString) {
	DBPRINT("[Enter]", __FUNCTION__);
	NanScope();
	Local<Value> argv[] = {
		NanNew<Integer>(WorkerId()),
		NanNew<String>(const_cast<char*>(aString.c_str()))
	};
	const int argc = 2;
	_progress->Call(argc, argv);
	DBPRINT("[Exit ]", __FUNCTION__);
}

//----------------------
// Notify finish
// [v8 context ]
void AsyncWorker::FinishedCallback() {
	DBPRINT("[Enter]", __FUNCTION__);
	NanScope();
	Local<Value> argv[] = { NanNew<Integer>(WorkerId()) };
	const int argc = 1;
	_finish->Call(argc, argv);
	DBPRINT("[Exit ]", __FUNCTION__);
}

//----------------------
// Send Message to v8 context
// [non-v8     ]
void AsyncWorker::SendAsyncMessage(Request* aRequest) {
	DBPRINT("[Enter]", __FUNCTION__);
	uv_mutex_lock(&_mutex);
	_msg_queue.push(aRequest);
	uv_async_send(&_async_handle);
	uv_mutex_unlock(&_mutex);

	DBPRINTA(" @@@@@ %s send msg [%d]\n", __FUNCTION__, aRequest->msgType());

	DBPRINT("[Exit ]", __FUNCTION__);
}
//----------------------
// Abort Request
// [v8 context ]
void AsyncWorker::AbortRequest() {
	DBPRINT("[Enter]", __FUNCTION__);
	_requestedAbort = true;
	uv_timer_stop(&_tick_handle);
	uv_unref((uv_handle_t*)&_tick_handle);
	uv_close((uv_handle_t*)&_tick_handle, CloseCb);
	uv_sem_post(&_wait_tick);
	DBPRINT("[Exit ]", __FUNCTION__);
}
//----------------------
// Tick
// [v8 context ]
void AsyncWorker::Tick() {
	DBPRINT("[Enter]", __FUNCTION__);
	if (!_requestedAbort) {
		DBPRINT("[Trace]", __FUNCTION__);
		uv_timer_again(&_tick_handle);
	}
	uv_sem_post(&_wait_tick);
	DBPRINT("[Exit ]", __FUNCTION__);
}
//----------------------
// Timer Start
// [v8 context ]
void AsyncWorker::OnAsyncMessage() {
	DBPRINT("[Enter]", __FUNCTION__);
	uv_mutex_lock(&_mutex);
	DBPRINTA(" @@@@@ %s queue size is [%d]\n", __FUNCTION__, _msg_queue.size());
	while (!_msg_queue.empty()) {
		Request* req = _msg_queue.front();
		_msg_queue.pop();
		DBPRINTA(" @@@@@ %s msg is [%d]\n", __FUNCTION__, req->msgType());
		switch (req->msgType())
		{
		case AsyncMsgType::TimerStart:
			if (!_requestedAbort) {
				uv_timer_start(&_tick_handle, TickTimerCb, _interval, _interval);
			}
			break;
		case AsyncMsgType::Progress:
			this->ProgressCallback(req->string());
			break;
		case AsyncMsgType::Exit:
			this->FinishedCallback();
			uv_close((uv_handle_t*)&_async_handle, CloseCb);
			break;
		default:
			assert(false);
			break;
		}
		delete req;
	}
	uv_mutex_unlock(&_mutex);
	DBPRINT("[Exit ]", __FUNCTION__);
}
//----------------------
// Start
// [v8 context ]
void AsyncWorker::Start() {
	NanAssignPersistent(_myself, NanObjectWrapHandle(this));// for GB.
	uv_thread_create(&_worker_handle, DoWork, this);
}
//----------------------
// Destroy
// [v8 context ]
void AsyncWorker::Destroy() {
	uv_thread_join(&_worker_handle);
	NanDisposePersistent(_myself);
}

//----------------------
// [v8 context ]
void AsyncWorker::Init(Handle<Object> aExports)
{
	DBPRINT("[V8   ]", __FUNCTION__);
	// NanScope();  <- no need
	
	// Prepare constructor template
	Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(CmdNew);
    tpl->SetClassName(NanNew("AsyncWorker"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
    // Prototype
    NanSetPrototypeTemplate(tpl, "start", NanNew<FunctionTemplate>(CmdStart));
    NanSetPrototypeTemplate(tpl, "stop", NanNew<FunctionTemplate>(CmdStop));
	NanSetPrototypeTemplate(tpl, "id", NanNew<FunctionTemplate>(CmdGetId));

	// Return created object via the arg.
	NanAssignPersistent<Function>(_constructor, tpl->GetFunction());
    aExports->Set(NanNew("AsyncWorker"), tpl->GetFunction());
}

//----------------------
// [v8 context ]
NAN_METHOD(AsyncWorker::CmdNew) {
	DBPRINT("[V8   ]", __FUNCTION__);
	NanScope();
	if (!args[AsyncWorker::ArgSettingFilePath]->IsString()) {
		NanThrowError("param error : 'setting file' is not string.");
	}
	if (args[AsyncWorker::ArgSettingFilePath].As<String>()->Length() == 0) {
		NanThrowError("param error : 'setting file' is length 0.");
	}
	if (!args[AsyncWorker::ArgInterval]->IsNumber()) {
		NanThrowError("param error : 'interval' is not number.");
	}
	if (args[AsyncWorker::ArgInterval]->Int32Value() <= 0) {
		NanThrowError("param error : 'interval' is '0' or less.");
	}
	if (!args[AsyncWorker::ArgCbProgress]->IsFunction()) {
		NanThrowError("param error : 'progress cb' is not function.");
	}
	if (!args[AsyncWorker::ArgCbFinish]->IsFunction()) {
		NanThrowError("param error : 'finished cb' is not function.");
	}

	if (args.IsConstructCall()) {
		// Invoked as constructor: `new MyObject(...)`
		NanCallback* progress = new NanCallback(
			args[AsyncWorker::ArgCbProgress].As<Function>());
		NanCallback* callback = new NanCallback(
			args[AsyncWorker::ArgCbFinish].As<Function>());
		NanUtf8String* filename = new NanUtf8String(
			args[AsyncWorker::ArgSettingFilePath]);

		// "worker" will be destroyed by itself.
		AsyncWorker* obj = new AsyncWorker(
			progress,					// progress callback
			callback,					// finish callback
			// filename
			std::string(filename->operator*()),
			// interval
			args[AsyncWorker::ArgInterval]->Int32Value());
		
		obj->Wrap(args.This());
		NanReturnThis();
	}
	else {
		// Invoked as constructor: `MyObject(...)`
		const int argc = 4;
		Local<Value> argv[argc] = {
				args[ArgSettingFilePath],
				args[ArgInterval       ],
				args[ArgCbProgress     ],
				args[ArgCbFinish       ] };
		Local<Function> cons = NanNew<Function>(_constructor);
		NanReturnValue(cons->NewInstance(argc, argv));
	}
	NanReturnUndefined();
}

//----------------------
// [v8 context ]
NAN_METHOD(AsyncWorker::CmdStart) {
	DBPRINT("[V8   ]", __FUNCTION__);

	NanScope();
	AsyncWorker* obj = ObjectWrap::Unwrap<AsyncWorker>(args.Holder());

	obj->Start();
	NanReturnValue(NanNew<Int32>(obj->WorkerId()));
}

//----------------------
// [v8 context ]
NAN_METHOD(AsyncWorker::CmdStop) {
	DBPRINT("[V8   ]", __FUNCTION__);

	NanScope();
	AsyncWorker* obj = ObjectWrap::Unwrap<AsyncWorker>(args.Holder());
	obj->AbortRequest();
	NanReturnUndefined();
}
//----------------------
// [v8 context ]
NAN_METHOD(AsyncWorker::CmdGetId) {
	DBPRINT("[V8   ]", __FUNCTION__);

	NanScope();
	AsyncWorker* obj = ObjectWrap::Unwrap<AsyncWorker>(args.Holder());
	NanReturnValue(NanNew<Int32>(obj->WorkerId()));
}

//----------------------
// Tick cb
// [v8 context ]
#if (NODE_MODULE_VERSION < NODE_0_12_MODULE_VERSION)
void AsyncWorker::TickTimerCb(uv_timer_t* aHandle, int/*UNUSED*/) {
#else
void AsyncWorker::TickTimerCb(uv_timer_t* aHandle) {
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
void AsyncWorker::RequestAsyncMessageCb(uv_async_t* aHandle, int/*UNUSED*/) {
#else
void AsyncWorker::RequestAsyncMessageCb(uv_async_t* aHandle) {
#endif
	DBPRINT("[Enter]", __FUNCTION__);
	if (0 != uv_is_active((uv_handle_t*)aHandle)) { // non-zero is active
		AsyncWorker* instance = reinterpret_cast<AsyncWorker*>(aHandle->data);
		instance->OnAsyncMessage();
	}
	DBPRINT("[Exit ]", __FUNCTION__);
}
//----------------------
// Handle closed cb
// [v8 context ]
void AsyncWorker::CloseCb(uv_handle_t* aHandle) {
	DBPRINT("[Enter]", __FUNCTION__);
	AsyncWorker* instance = reinterpret_cast<AsyncWorker*>(aHandle->data);
	instance->_handle_count--;
	if (0 == instance->_handle_count) {
		instance->Destroy();
	}
	DBPRINT("[Exit ]", __FUNCTION__);
}
//----------------------
// Execute loop
// [non-v8     ]
void AsyncWorker::DoWork(void* aObject) {
	DBPRINT("[Enter]", __FUNCTION__);
	reinterpret_cast<AsyncWorker*>(aObject)->ExecuteLoop();
	DBPRINT("[Exit ]", __FUNCTION__);
}
//----------------------
// lines(file)->vector
void AsyncWorker::ReadFile(
			const std::string& aFilePath,
			std::queue<std::string>& oList)
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
					oList.push(str);
				}
			}
		}
		instream.close();
	}
}

//----------------------
// Trim
// http://program.station.ez-net.jp/special/handbook/cpp/string/trim.asp
std::string AsyncWorker::Trim(const std::string& aString,
				const char* aTrimCharacterList)
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
// internal request define
AsyncWorker::Request::Request(AsyncMsgType aMsgType, std::string& aString)
	:_msgType(aMsgType), _string(aString)
{
}
AsyncWorker::Request::Request(AsyncMsgType aMsgType)
	:_msgType(aMsgType)
{
}
AsyncWorker::Request::~Request(){

}

int AsyncWorker::shareworkerid = 0;

v8::Persistent<v8::Function> AsyncWorker::_constructor;

//EOF
