// hello.js
//require('v8-profiler');

var echostring = require('./index.js');

//echostring.start(100, 100, function(result) {
//	console.log('console++', result);
//});
//	console.log('+++', process.memoryUsage());

//

//var id = echostring.start("PathX", 500/*interval*/,
//var id = echostring.start("C:\\Users\\tanakahi\\nodeschool\\caddon\\test.txt", 100/*interval*/,
var id = echostring.start("test/teststrings.txt", 100/*interval*/,
	function(id, hoge) {	// progress
		console.log('@@@@@@@', id, hoge);
	},
	function(id) {	// finish
		console.log('*******', id);
	//	console.log('console++', result);
	//		console.log('+++', process.memoryUsage());

	}
);
echostring.stop(id);
console.log('++++++');

//var id2 = echostring.start("C:\\Users\\tanakahi\\nodeschool\\caddon\\test.txt", 200/*interval*/,
//	function(id, hoge) {	// progress
//		console.log('@@@@@@@', id, hoge);
//	},
//	function(id) {	// finish
//		console.log('*******', id);
//
//	}
//);

//	setTimeout(function () {
//		echostring.stop(id);
//	}, 500);

//	setTimeout(function () {
//		echostring.stop(id);
//	}, 600);


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
			console.log('-');
//			echostring.stop(id);
//			if(global.gc) {
//				 global.gc();
//				console.log('+++', process.memoryUsage());
//			}
		}, 600);


		setTimeout(function () {
			console.log('------------------------------+');
		}, 1000);
		setTimeout(function () {
			console.log('------------------------------+');
		}, 1000);
		setTimeout(function () {
			console.log('------------------------------+');
		}, 1000);
