// hello.js
//require('v8-profiler');

var echostring = require('./index.js');

//echostring.echoStringCyclic(100, 100, function(result) {
//	console.log('console++', result);
//});
//	console.log('+++', process.memoryUsage());

//

//var id = echostring.echoStringCyclic("PathX", 500/*interval*/,
//var id = echostring.echoStringCyclic("C:\\Users\\tanakahi\\nodeschool\\caddon\\test.txt", 100/*interval*/,
var id = echostring.echoStringCyclic("test/teststrings.txt", 100/*interval*/,
	function(id, hoge) {	// progress
		console.log('@@@@@@@', id, hoge);
	},
	function(id) {	// finish
		console.log('*******', id);
	//	console.log('console++', result);
	//		console.log('+++', process.memoryUsage());

	}
);
echostring.echoStringCyclicAbort(id);
console.log('++++++');

//var id2 = echostring.echoStringCyclic("C:\\Users\\tanakahi\\nodeschool\\caddon\\test.txt", 200/*interval*/,
//	function(id, hoge) {	// progress
//		console.log('@@@@@@@', id, hoge);
//	},
//	function(id) {	// finish
//		console.log('*******', id);
//
//	}
//);

//	setTimeout(function () {
//		echostring.echoStringCyclicAbort(id);
//	}, 500);

//	setTimeout(function () {
//		echostring.echoStringCyclicAbort(id);
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
//			echostring.echoStringCyclicAbort(id);
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
