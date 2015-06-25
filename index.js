// hello.js
//require('v8-profiler');

var addon = require('./build/Release/addon');

console.log('Hello', addon.hello()); // 'world'


var timercb = require('./build/Release/timercb');

//timercb.async(100, 100, function(result) {
//	console.log('console++', result);
//});
//	console.log('+++', process.memoryUsage());

var id = timercb.async(100/*interval*/, 5/*count*/,
	function(result) {
		console.log('@@@@@@@', result);
	},
	function(result) {
		console.log('*******', result);
	//	console.log('console++', result);
	//		console.log('+++', process.memoryUsage());

	}
);

	setTimeout(function () {
		timercb.abort(id);
	}, 200);


//var timer = setInterval(function () {
//	console.log('----');
//
//if(global.gc) {
//	console.log('+++', process.memoryUsage());
// global.gc();
//}
//
//}, 100);

		setTimeout(function () {
//clearInterval(timer);
			console.log('-------------------------------');
			timercb.abort(id);
			if(global.gc) {
				 global.gc();
				console.log('+++', process.memoryUsage());
			}
		}, 2000);
