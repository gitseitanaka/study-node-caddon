var assert = require("power-assert");
var stringecho = require('../index.js');

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


describe('string-echo', function () {
  var gid = 0;
  describe('start-stop', function () {
    
    it ('start->stop(none call cb)', function (done) {
      var called = 0;
      gid = stringecho.echoStringCyclic(testfilename, 100,
                function(id, name) {  // process
                  called++;
                },
                function(id) {        // finish
                  assert.equal(called, 0);
                  done();
                }
              );
      stringecho.echoStringCyclicAbort(gid);
    });

    it ('start->stop(called cb 1 time)', function (done) {
      var interval = 100 /* ms */;
      var times = 1;
      var called = 0;
      gid = stringecho.echoStringCyclic(testfilename, interval,
                function(id, name) {  // process
                  assert.equal(namearray[called], name);
                  called++;
                },
                function(id) {        // finish
                  assert.equal(called, 1);
                  done();
                }
              );
      setTimeout(function () {
        stringecho.echoStringCyclicAbort(gid);
      }, interval * times);
    });
    
    it ('start->stop(called cb 20 time)', function (done) {
      var interval = 100 /* ms */;
      var times = 20;
      var called = 0;
      gid = stringecho.echoStringCyclic(testfilename, interval,
                function(id, name) {  // process
                  assert.equal(namearray[called % namearray.length], name);
                  assert.equal(gid, id);
                  called++;
                },
                function(id) {        // finish
                  //assert.equal(called, times);
                  assert.equal(gid, id);
                  done();
                }
              );
      setTimeout(function () {
        stringecho.echoStringCyclicAbort(gid);
      }, interval * times);
    });
    
    /* used default value(4) with "UV_THREADPOOL_SIZE". */
    it ('start->stop(called cb 20 time) x 3', function (done) {
      var interval = 200 /* ms */;
      var times = 20;
      var tester = 4;
      var count = 0;
      var func = function() {
          return function(aGid) {
                  var _called = 0;
                  var _gid = stringecho.echoStringCyclic(testfilename, interval,
                            function(id, name) {  // process
                              assert.equal(namearray[_called % namearray.length], name);
                              assert.equal(aGid, id);
                              
                              _called++;
                            },
                            function(id) {        // finish
                              count++;
                              assert.equal(aGid, id);
                              assert.equal(_called, times);
//                              console.log('***', id);
                              if (count >= tester) {
                                done();
                              }
                            }
                          );
                  
                    setTimeout(function () {
                      stringecho.echoStringCyclicAbort(aGid);
                    }, interval * times);
                  return _gid;
            };
          };
      
      var nextgid = gid + 1;
      for (var i = 0; i < tester; i++) {
        console.log('');
        var testfunc = func();
        assert.equal(nextgid + i ,testfunc(nextgid + i));
      }
    });
  });
});
