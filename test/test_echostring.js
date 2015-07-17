var assert = require("power-assert");
var stringecho = require('../index.js');

var testfilename;
var namearray;

describe('study-caddon-string-echo', function () {
  testfilename = 'test/teststrings.txt';
  namearray = [];
  before(function(done) {
    var fs = require('fs'),
        readline = require('readline'),
        rs = fs.ReadStream(testfilename),
        rl = readline.createInterface({'input': rs, 'output': {}});
    rl.on('line', function (line) {
        var name = line.trim();
        if (name.length !== 0) {
          namearray.push(name);
        }
    });
    rl.resume();
    done();
  });
  
  describe('args error', function () {

    describe('new', function () {
      describe('arg 0', function () {
        it ('setting file is null', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(null, 100,
                              function(id, name) {
                              },
                              function(id) {
                              }
                            );
                });
        });
        it ('setting file is undef', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(undef, 100,
                              function(id, name) {
                              },
                              function(id) {
                              }
                            );
                });
        });
        it ('setting file is length 0', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker('', 100,
                              function(id, name) {
                              },
                              function(id) {
                              }
                            );
                });
        });
        it ('setting file is number', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(1, 100,
                              function(id, name) {
                              },
                              function(id) {
                              }
                            );
                });
        });
      });
      describe('arg 1', function () {
        it ('interval is 0', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, 0,
                              function(id, name) {
                              },
                              function(id) {
                              }
                            );
                });
        });
        it ('interval is -1', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, -1,
                              function(id, name) {
                              },
                              function(id) {
                              }
                            );
                });
        });
        it ('interval is null', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, null,
                              function(id, name) {
                              },
                              function(id) {
                              }
                            );
                });
        });
        it ('interval is undef', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, undef,
                              function(id, name) {
                              },
                              function(id) {
                              }
                            );
                });
        });
        it ('interval string', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, '100',
                              function(id, name) {
                              },
                              function(id) {
                              }
                            );
                });
        });
      });
      describe('arg 2', function () {
        it ('progress cb is null', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, 100,
                              null,
                              function(id) {
                              }
                            );
                });
        });
        it ('progress cb is undef', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, 100,
                              undef,
                              function(id) {
                              }
                            );
                });
        });
        it ('progress cb is string', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, 100,
                              'test',
                              function(id) {
                              }
                            );
                });
        });
        it ('progress cb is number', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, 100,
                              1,
                              function(id) {
                              }
                            );
                });
        });
      });
      describe('arg 3', function () {
        it ('finished cb is null', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, 100,
                              function(id) {
                              },
                              null
                            );
                });
        });
        it ('finished cb is undef', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, 100,
                              function(id) {
                              },
                              undef
                            );
                });
        });
        it ('finished cb is string', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, 100,
                              function(id) {
                              },
                              'test'
                            );
                });
        });
        it ('finished cb is number', function () {
            assert.throws(
                function() {
                    new stringecho.AsyncWorker(testfilename, 100,
                              function(id) {
                              },
                              1
                            );
                });
        });
      });
    });

    describe('start', function () {
    });
    describe('stop', function () {
    });
  });

	var workerid;
  describe('new', function () {
	  before(function(done) {
			workerid = 0;
			done();
		});
    it ('new', function () {
      var worker = new stringecho.AsyncWorker(testfilename, 100,
                function(id, name) {  // process
                  called++;
                },
                function(id) {        // finish
                  assert.equal(called, 0);
                }
              );
			workerid = worker.id();
			assert.equal(worker.id(), workerid);
			workerid++;
    });
    it ('(new)', function () {
      var worker = stringecho.AsyncWorker(testfilename, 100,
                function(id, name) {  // process
                  called++;
                },
                function(id) {        // finish
                  assert.equal(called, 0);
                }
              );
			assert.equal(worker.id(), workerid);
			workerid++;
    });
	});


  describe('start-stop', function () {
    var gid = 0;
    
    it ('start->stop(none call cb)', function (done) {
      var called = 0;
      gid = stringecho.start(testfilename, 100,
                function(id, name) {  // process
                  called++;
                },
                function(id) {        // finish
                  assert.equal(called, 0);
                  done();
                }
              );
      stringecho.stop(gid);
    });

    it ('start->stop(called cb 1 time)', function (done) {
      var interval = 100 /* ms */;
      var times = 1;
      var called = 0;
      gid = stringecho.start(testfilename, interval,
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
        stringecho.stop(gid);
      }, interval * times + (interval / 2));
    });
    
    it ('start->stop(called cb 20 time)', function (done) {
      var interval = 100 /* ms */;
      var times = 20;
      var called = 0;
      gid = stringecho.start(testfilename, interval,
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
        stringecho.stop(gid);
      }, interval * times);
    });

    var tester_number =1000;
    it ('start->stop(called cb 20 time) x ' + tester_number, function (done) {
      var interval = 100 /* ms */;
      var times = 20;
      var tester =tester_number;
      var count = 0;
      var range = 2;
      if ( /v0\.10\./g.exec(process.version) ){
          range = 4;
      }
      var func = function() {
          return function(aGid) {
                  var _called = 0;
                  var _gid = stringecho.start(testfilename, interval,
                            function(id, name) {  // process
                              assert.equal(namearray[_called % namearray.length], name);
                              assert.equal(aGid, id);
                              
                              _called++;
                            },
                            function(id) {        // finish
                              count++;
                              //console.log('***', id, _called, times);
                              assert.equal(aGid, id);
                              assert.ok(_called >= (times - range), 'count error');
                              assert.ok(_called <= times + range, 'count error');
                              if (count >= tester) {
                                done();
                              }
                            }
                          );
                  
                    setTimeout(function () {
                      stringecho.stop(aGid);
                    }, interval * times);
                  return _gid;
            };
          };
      
      var nextgid = gid + 1;
      for (var i = 0; i < tester; i++) {
        //console.log('');
        var testfunc = func();
        assert.equal(nextgid + i ,testfunc(nextgid + i));
      }
    });
  });
});
