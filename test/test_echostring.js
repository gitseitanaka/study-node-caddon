var assert = require("power-assert");
var stringecho = require('../index.js');


describe('string-echo', function () {
	var testfilename = 'test/teststrings.txt';
	var fs = require('fs'),
	    readline = require('readline'),
	    rs = fs.ReadStream(testfilename),
	    rl = readline.createInterface({'input': rs, 'output': {}});
	var namearray = [];
	rl.on('line', function (line) {
			var name = line.trim();
			if (name.length !== 0) {
		    namearray.push(name);
			}
	});
	rl.resume();

	describe('start-stop', function () {
		
		it ('start->stop(none call cb)', function () {
			var called = 0;
			var gid = stringecho.async(testfilename, 100,
								function(id, name) {	// process
									assert.equal(namearray[called], name);
									called++;
								},
								function(id) {				// finish
									assert(called === 0);
								}
							);
			stringecho.abort(gid);
		});

		it ('start->stop(called cb 1 time)', function (done) {
			var called = 0;
			var gid = stringecho.async(testfilename, 100,
								function(id, name) {	// process
									assert.equal(namearray[called], name);
									called++;
								},
								function(id) {				// finish
									assert(called === 1);
									done();
								}
							);
			setTimeout(function () {
				stringecho.abort(gid);
			}, 150);
		});


	});


});


