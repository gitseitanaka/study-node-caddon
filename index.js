// hello.js
require('v8-profiler');

//var addon = require('./build/Release/addon');

//console.log(addon.hello()); // 'world'


var timercb = require('./build/Release/timercb');

//timercb.async(100, 100, function(result) {
//	console.log('console++', result);
//});
		console.log('+++', process.memoryUsage());

timercb.async(100, 110, function(result) {
//	console.log('console++', result);
//		console.log('+++', process.memoryUsage());

});

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
			if(global.gc) {
				 global.gc();
				console.log('+++', process.memoryUsage());
			}
		}, 20000);
