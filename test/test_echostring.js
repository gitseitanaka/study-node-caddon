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
    describe('echoStringCyclic', function () {
      describe('arg 0', function () {
        it ('setting file is null', function () {
            assert.throws(
                function() {
                    stringecho.echoStringCyclic(null, 100,
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
                    stringecho.echoStringCyclic(undef, 100,
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
                    stringecho.echoStringCyclic('', 100,
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
                    stringecho.echoStringCyclic(1, 100,
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
                    stringecho.echoStringCyclic(testfilename, 0,
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
                    stringecho.echoStringCyclic(testfilename, -1,
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
                    stringecho.echoStringCyclic(testfilename, null,
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
                    stringecho.echoStringCyclic(testfilename, undef,
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
                    stringecho.echoStringCyclic(testfilename, '100',
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
                    stringecho.echoStringCyclic(testfilename, 100,
                              null,
                              function(id) {
                              }
                            );
                });
        });
        it ('progress cb is undef', function () {
            assert.throws(
                function() {
                    stringecho.echoStringCyclic(testfilename, 100,
                              undef,
                              function(id) {
                              }
                            );
                });
        });
        it ('progress cb is string', function () {
            assert.throws(
                function() {
                    stringecho.echoStringCyclic(testfilename, 100,
                              'test',
                              function(id) {
                              }
                            );
                });
        });
        it ('progress cb is number', function () {
            assert.throws(
                function() {
                    stringecho.echoStringCyclic(testfilename, 100,
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
                    stringecho.echoStringCyclic(testfilename, 100,
                              function(id) {
                              },
                              null
                            );
                });
        });
        it ('finished cb is undef', function () {
            assert.throws(
                function() {
                    stringecho.echoStringCyclic(testfilename, 100,
                              function(id) {
                              },
                              undef
                            );
                });
        });
        it ('finished cb is string', function () {
            assert.throws(
                function() {
                    stringecho.echoStringCyclic(testfilename, 100,
                              function(id) {
                              },
                              'test'
                            );
                });
        });
        it ('finished cb is number', function () {
            assert.throws(
                function() {
                    stringecho.echoStringCyclic(testfilename, 100,
                              function(id) {
                              },
                              1
                            );
                });
        });
      });
    });
    describe('echoStringCyclicAbort', function () {
      describe('arg 0', function () {
        it ('discriptor id is null', function () {
            assert.throws(
                function() {
                    stringecho.echoStringCyclicAbort(null);
                });
        });
        it ('discriptor id is string', function () {
            assert.throws(
                function() {
                    stringecho.echoStringCyclicAbort('');
                });
        });
        it ('discriptor id is undef', function () {
            assert.throws(
                function() {
                    stringecho.echoStringCyclicAbort(undef);
                });
        });
      });
    });
  });

  describe('start-stop', function () {
    var gid = 0;
    
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
      }, interval * times + (interval / 2));
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
    it ('start->stop(called cb 20 time) x 4', function (done) {
      var interval = 100 /* ms */;
      var times = 20;
      var tester = 4;
      var count = 0;
      var range = 1;
      if ( /v0\.10\./g.exec(process.version) ){
          range = 2;
      }
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
                              //console.log('***', id, _called);
                              assert.equal(aGid, id);
                              assert.ok(_called >= times - range, 'count error');
                              assert.ok(_called <= times + range, 'count error');
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
        //console.log('');
        var testfunc = func();
        assert.equal(nextgid + i ,testfunc(nextgid + i));
      }
    });
  });
});
