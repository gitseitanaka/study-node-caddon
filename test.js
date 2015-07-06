// hello.js
//require('v8-profiler');

var echostring = require('./index.js');

//echostring.async(100, 100, function(result) {
//	console.log('console++', result);
//});
//	console.log('+++', process.memoryUsage());

//

//var id = echostring.async("PathX", 500/*interval*/,
var id = echostring.async("C:\\Users\\tanakahi\\nodeschool\\caddon\\test.txt", 500/*interval*/,
	function(id, hoge) {	// progress
		console.log('@@@@@@@', id, hoge);
	},
	function(id) {	// finish
		console.log('*******', id);
	//	console.log('console++', result);
	//		console.log('+++', process.memoryUsage());

	}
);
var id2 = echostring.async("C:\\Users\\tanakahi\\nodeschool\\caddon\\test.txt", 200/*interval*/,
	function(id, hoge) {	// progress
		console.log('@@@@@@@', id, hoge);
	},
	function(id) {	// finish
		console.log('*******', id);

	}
);

	setTimeout(function () {
		echostring.abort(id);
	}, 3000);

	setTimeout(function () {
		echostring.abort(id2);
	}, 3000);


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
			echostring.abort(id);
			if(global.gc) {
				 global.gc();
				console.log('+++', process.memoryUsage());
			}
		}, 2000);
