#include <iostream>
#include <thread>
#include <uv.h>
#include <node.h>

using namespace v8;

//--------------------------------------------------
// Other info
//
// C:\Users\tanakahi\.node-gyp\0.12.2
// I know this question is a bit old, but there has been a pretty major update in nodejs v0.10 to v0.12.
//  http://stackoverflow.com/questions/13826803/calling-javascript-function-from-a-c-callback-in-v8
//  https://github.com/felixge/node-memory-leak-tutorial

//  このコードのベース
//  http://tips.hecomi.com/entry/20121021/1350819390

// 誰かが作成した旧Versionのv8リファレンス。古い。
// http://izs.me/v8-docs/main.html 

// googleのv8概略説明
// https://developers.google.com/v8/embed


//---------------------------
//
// 保存用型
//
struct my_struct
{
	int interval;
	int count;
	Persistent<Function> callback;
};

//---------------------------
//
// javascriptの裏で実行される
//   - この中で v8世界のリソースはいじれない
//
void asyncWorker(uv_work_t* req) {
	std::cout << "asyncWorker\t: " << std::this_thread::get_id() << std::endl;
	
	// v8とは別世界で sleep.
	my_struct* data = static_cast<my_struct*>(req->data);
	std::this_thread::sleep_for( std::chrono::milliseconds(data->interval) );
}

//---------------------------
//
// "asyncWorker"がEixtした後に呼ばれる
//   - javascript世界に通知するコードを記述する
//   - "uv_queue_work"が、うまく呼んでくれる。
void asyncWorkerAfter(uv_work_t* req, int ) {
	std::cout << "asyncWorkerAfter\t: " << std::this_thread::get_id() << std::endl;

	my_struct* data = static_cast<my_struct*>(req->data);
	
	Isolate* isolate = Isolate::GetCurrent();
	{
		HandleScope scope(isolate);
		const unsigned argc = 1;
		Handle<Value> argv[argc] = { Number::New(isolate, data->count) };
		Local<Function> cb = Local<Function>::New(isolate, data->callback);
		Handle<Value> result = cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
  		if (result.IsEmpty()) {
			std::cout << "asyncWorkerAfter cb error !!" << std::endl;
		}
	}

	if ( 0 >= data->count-- ) {
		data->callback.Reset();
		
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

	my_struct* data = new my_struct;
	uv_work_t *req = new uv_work_t;
	
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	data->count = args[0]->Int32Value();
	data->interval = args[1]->Int32Value();
	data->callback.Reset(isolate, args[2].As<Function>());
	req->data = data;
	
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
