#include <iostream>
#include <thread>
#include <uv.h>
#include <node.h>

// C:\Users\tanakahi\.node-gyp\0.12.2

// I know this question is a bit old, but there has been a pretty major update in nodejs v0.10 to v0.12.
//http://stackoverflow.com/questions/13826803/calling-javascript-function-from-a-c-callback-in-v8
//https://github.com/felixge/node-memory-leak-tutorial

//http://tips.hecomi.com/entry/20121021/1350819390

// http://izs.me/v8-docs/main.html 非公式？ 古い。 


// https://developers.google.com/v8/embed



using namespace v8;

// スレッド間でやりとりするデータ（任意）
struct my_struct
{
	int interval;
	int count;
	Persistent<Function> callback;
};

// 別スレッドで実行する非同期プロセス
// この中で v8 の世界に入ることは出来ない
void AsyncWork(uv_work_t* req) {
	std::cout << "AsyncWork\t: " << std::this_thread::get_id() << std::endl;

	// データを非同期で処理したりする
	my_struct* data = static_cast<my_struct*>(req->data);
	
//	Isolate* isolate = Isolate::GetCurrent();
	{
//		HandleScope scope(isolate);
//		for (int i = 0; i < 2; i++) {
			std::this_thread::sleep_for( std::chrono::milliseconds(data->interval) );
		
			std::cout << "-- AsyncWork\t: " << std::this_thread::get_id() << std::endl;
			
//			const unsigned argc = 1;
//			Local<Value> argv[argc] = { Number::New(isolate, data->result) };
//			Local<Function>::New(isolate, data->callback)->Call(isolate->GetCurrentContext()->Global(), argc, argv);
			
//		}
	}
}

// 非同期プロセスが実行した後に呼ばれる
void AsyncAfter(uv_work_t* req, int ) {
	std::cout << "AsyncAfter\t: " << std::this_thread::get_id() << std::endl;

	my_struct* data = static_cast<my_struct*>(req->data);
	
	Isolate* isolate = Isolate::GetCurrent();
	{
		HandleScope scope(isolate);
		const unsigned argc = 1;
		Local<Value> argv[argc] = { Number::New(isolate, data->count) };
		Local<Function>::New(isolate, data->callback)->Call(isolate->GetCurrentContext()->Global(), argc, argv);
	}

	if ( 0 >= data->count-- ) {
		data->callback.Reset();
		
		delete data; data = NULL;
		delete req; req = NULL;
	} else {
		uv_queue_work(uv_default_loop(), req, AsyncWork, AsyncAfter);
	}
	
}

// JavaScript の世界から呼ばれる
void Async(const FunctionCallbackInfo<Value>& args)
{
	std::cout << "Async\t\t: " << std::this_thread::get_id() << std::endl;

	// 別スレッドへ渡すデータを作る
	my_struct* data = new my_struct;
	// 別スレッドで処理を行う
	uv_work_t *req = new uv_work_t;
	
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	
	data->count = args[0]->Int32Value();
	data->interval = args[1]->Int32Value();
	data->callback.Reset(isolate, args[2].As<Function>());

	
	req->data = data;
	uv_queue_work(uv_default_loop(), req, AsyncWork, AsyncAfter);
	
}

// Async 関数を JavaScript の世界へ送り出す
void Init(Handle<Object> target) {
	NODE_SET_METHOD(target, "async", Async);
}
NODE_MODULE(addon, Init)
