var assert = require("power-assert");
var AsyncWorker = require('../index.js');

var testfilename;
var namearray;

describe('study-caddon-string-echo events', function () {
  testfilename = 'test/teststrings.txt';
  namearray = [];
  before(function (done) {
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
    describe('arg 0', function () {
      it('setting file is null', function () {
        assert.throws(
          function () {
            AsyncWorker(null, 100);
          });
      });
      it('setting file is undef', function () {
        assert.throws(
          function () {
            AsyncWorker(undef, 100);
          });
      });
      it('setting file is length 0', function () {
        assert.throws(
          function () {
            AsyncWorker('', 100);
          });
      });
    });
    describe('arg 1', function () {
      it('interval is 0', function () {
        assert.throws(
          function () {
            AsyncWorker(testfilename, 0);
          });
      });
      it('interval is -1', function () {
        assert.throws(
          function () {
            AsyncWorker(testfilename, -1);
          });
      });
      it('interval is null', function () {
        assert.throws(
          function () {
            AsyncWorker(testfilename, null);
          });
      });
      it('interval is undef', function () {
        assert.throws(
          function () {
            AsyncWorker(testfilename, undef);
          });
      });
      it('interval string', function () {
        assert.throws(
          function () {
            AsyncWorker(testfilename, '100');
          });
      });
      it('not exist', function () {
        assert.throws(
          function () {
            AsyncWorker("xxxx", '100');
          });
      });
    });
  });
  describe('start-stop', function () {
    var workerid = 0;
    it('(   )->stop(none call cb)', function () {
      var called = 0;
      var worker = AsyncWorker(testfilename, 100);
      worker.on('progress', function (id, name) {  // process
        called++;
      });
      worker.on('end', function (id) {            // finish
        assert.equal(called, 0);
      });

      assert.equal(worker.stop(), -1);
      assert.equal(worker.stop(), -1);
      workerid = worker.id() + 1;
    });
    it('start->stop(none call cb)', function () {
      var called = 0;
      var worker = AsyncWorker(testfilename, 100);
      worker.on('progress', function (id, name) {  // process
        called++;
      });
      worker.on('end', function (id) {        // finish
        assert.equal(called, 0);
      });
      assert.equal(worker.start(), workerid);
      workerid++;
      assert.equal(worker.stop(), 0);
    });
    it('start->start->stop(none call cb)', function () {
      var called = 0;
      var worker = AsyncWorker(testfilename, 100);
      worker.on('progress', function (id, name) {  // process
        called++;
      });
      worker.on('end', function (id) {        // finish
        assert.equal(called, 0);
      });
      assert.equal(worker.start(), workerid);
      workerid++;
      assert.equal(worker.start(), -1);
      assert.equal(worker.stop(), 0);
    });

    it('start->stop(called cb 1 time)', function (done) {
      var interval = 100 /* ms */;
      var times = 1;
      var called = 0;
      var worker = AsyncWorker(testfilename, interval);
      worker.on('progress', function (id, name) {  // process
        assert.equal(namearray[called], name);
        called++;
      });
      worker.on('end', function (id) {        // finish
        assert.equal(called, 1);
        done();
      });
      assert.equal(worker.start(), workerid);
      workerid++;
      setTimeout(function () {
        worker.stop();
      }, interval * times + (interval / 2));
    });

    it('start->stop(called cb 20 time)', function (done) {
      var interval = 100 /* ms */;
      var times = 20;
      var called = 0;
      var worker = AsyncWorker(testfilename, interval);
      worker.on('progress', function (id, name) {  // process
        assert.equal(namearray[called % namearray.length], name);
        assert.equal(worker.id(), id);
        called++;
      });
      worker.on('end', function (id) {        // finish
        //assert.equal(called, times);
        assert.equal(worker.id(), id);
        done();
      });
      assert.equal(worker.start(), workerid);
      setTimeout(function () {
        worker.stop();
      }, interval * times);
    });

    var tester_number = 1000;
    it('start->stop(called cb 20 time) x ' + tester_number, function (done) {
      var interval = 100 /* ms */;
      var times = 20;
      var tester = tester_number;
      var count = 0;
      var range = 2;
      if (/v0\.10\./g.exec(process.version)) {
        range = 4;
      }
      var func = function () {
        return function () {
          var _called = 0;
          var worker = AsyncWorker(testfilename, interval);
          worker.on('progress', function (id, name) {  // process
            //console.log('---', id, _called, name);
            assert.equal(namearray[_called % namearray.length], name);
            //assert.equal(aGid, id);

            _called++;
          });
          worker.on('end', function (id) {        // finish
            count++;
            //console.log('***', id, _called, times, count, tester);
            //assert.equal(aGid, id);
            assert.ok(_called >= (times - range), 'count error');
            assert.ok(_called <= times + range, 'count error');
            if (count >= tester) {
              done();
            }
          });

          setTimeout(function () {
            worker.stop();
          }, interval * times);
          worker.start();
          return worker;
        };
      };
      workerid++;
      for (var i = 0; i < tester; i++) {
        var testfunc = func();
        var worker = testfunc();
        assert.equal(workerid + i, worker.id());
      }
    });
  });
});