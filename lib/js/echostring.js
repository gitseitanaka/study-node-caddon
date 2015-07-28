var events = require('events');

var bindings = require('bindings')('studyechostring');
var _AsyncWorker = bindings.AsyncWorker;

var AsyncWorker = function (aFilename,     // filename
                            aInterval,     // interval
                            aCbProgress,   // callback of progress
                            aCbFinished) { // callback of finished
  events.EventEmitter.call(this);
  this._asyncworker = new _AsyncWorker(
      aFilename, aInterval,
      aCbProgress, aCbFinished);
};
require('util').inherits(AsyncWorker, events.EventEmitter);


module.exports = function (aFilename,    // filename
                           aInterval) {    // interval
  if (typeof aFilename !== 'string') {
    throw("param error : 'setting file' is not string.");
  }
  if (aFilename.length === 0) {
    throw("param error : 'setting file' is length 0.");
  }
  if (!require("fs").existsSync(aFilename)) {
    throw("param error : " + aFilename + " is not exist.");
  }
  if (typeof aInterval !== 'number') {
    throw("param error : 'interval' is not number.");
  }
  if (aInterval <= 0) {
    throw("param error : 'interval' is '0' or less.");
  }

  var obj = new AsyncWorker(aFilename,
    aInterval,
    function (aaId, aaName) {
      obj.emit('progress', aaId, aaName);
    },
    function (aaId) {
      obj.emit('end', aaId);
    });
  return obj;
};

AsyncWorker.prototype.start = function () {
  return this._asyncworker._start();
};
AsyncWorker.prototype.stop = function () {
  return this._asyncworker._stop();
};
AsyncWorker.prototype.id = function () {
  return this._asyncworker._id();
};
