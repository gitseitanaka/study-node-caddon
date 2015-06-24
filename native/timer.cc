#include <iostream>
#include <thread>
#include <uv.h>
#include <node.h>

using namespace v8;


//---------------------------
//
// 保存用型
//
class RequestBall
{
private:
	int _interval;
	int _maxCount;
	int _count;
	Persistent<v8::Function> _callback;
	
public:
	RequestBall(
		Isolate& aIsolate,
		int aInterval, int aCount,
		Local<v8::Function> aFunction)
		: _interval(aInterval),
		_maxCount(aCount),
		_count(aCount)
	{
		_callback.Reset(&aIsolate, aFunction);
	}
	~RequestBall()
	{
		_callback.Reset();
	}

	Local<Number> WrapCount(Isolate& aIsolate) {
		EscapableHandleScope handle_scope(&aIsolate);
		Local<Number> count = { Number::New(&aIsolate, Count()) };
		return handle_scope.Escape(count);
	}

	Local<Function> WrapCallback(Isolate& aIsolate) {
		EscapableHandleScope handle_scope(&aIsolate);
		Local<Function> cb = Local<Function>::New(&aIsolate, _callback);
		return handle_scope.Escape(cb);
	}

	bool IsStillValid() {
		bool retvalue = true;
		if (0 >= _count) {
			retvalue = false;
		}
		else {
			_count--; 
			retvalue = true;
		}
		return retvalue;
	}
	int Count() {
		return _count;
	}
	int MaxCount() {
		return _maxCount;
	}
	int Interval() {
		return _interval;
	}
};

//---------------------------
//
// javascriptの裏で実行される
//   - この中で v8世界のリソースはいじれない
//
void asyncWorker(uv_work_t* req) {
	std::cout << "asyncWorker\t\t: " << std::this_thread::get_id() << std::endl;
	
	// v8とは別世界で sleep.
	RequestBall* data = static_cast<RequestBall*>(req->data);
	std::this_thread::sleep_for( std::chrono::milliseconds(data->Interval()) );
}

//---------------------------
//
// "asyncWorker"がEixtした後に呼ばれる
//   - javascript世界に通知するコードを記述する
//   - "uv_queue_work"が、うまく呼んでくれる。
void asyncWorkerAfter(uv_work_t* req, int ) {
	std::cout << "asyncWorkerAfter\t: " << std::this_thread::get_id() << std::endl;

	RequestBall* data = static_cast<RequestBall*>(req->data);
	
	Isolate* isolate = Isolate::GetCurrent();
	{
		HandleScope scope(isolate);

		const int argc = 1;
		Handle<Value> argv[argc] = { data->WrapCount(*isolate) };
		Local<Function> cb = data->WrapCallback(*isolate);

		Handle<Value> result = cb->Call(
					isolate->GetCurrentContext()->Global(),
					argc,
					argv);
  		if (result.IsEmpty()) {
			std::cout << "asyncWorkerAfter cb error !!" << std::endl;
		}
	}

	if (! data->IsStillValid()) {
		delete data; data = NULL;
		delete req; req = NULL;
	} else {
		uv_queue_work(uv_default_loop(), req, asyncWorker, asyncWorkerAfter);
	}
	
}

//---------------------------
//
// javascript(node.js)上のコンテキスト
//
void asyncCommand(const FunctionCallbackInfo<Value>& args)
{
	std::cout << "asyncCommand\t\t: " << std::this_thread::get_id() << std::endl;

	uv_work_t *req = new uv_work_t;
	Isolate* isolate = Isolate::GetCurrent();
	{
		HandleScope scope(isolate);

		RequestBall* data = new RequestBall(*isolate,
			args[0]->Int32Value(),	// count
			args[1]->Int32Value(),	// interval
			args[2].As<Function>());// callback

		req->data = data;
	}
	
	// 非同期queueへの追加
	// → "asyncWorker"をキューに積み、裏で動作させる。
	//    "asyncWorker"がexitしたら、javascript上のスレッドで
	//    "asyncWorkerAfter"をコールする
	uv_queue_work(
			uv_default_loop(),
			req,				// request object
			asyncWorker,		// worker.
			asyncWorkerAfter);	// process after exist worker on node.js's thread.

}

//---------------------------
//
// javascript(node.js)への登録
//
void Init(Handle<Object> target) {
	NODE_SET_METHOD(target, "async", asyncCommand);
}
NODE_MODULE(addon, Init)
