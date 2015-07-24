var events = require('events');

var bindings = require('bindings')('studyechostring');
var AsyncWorker = bindings.AsyncWorker;

inherits(AsyncWorker, events.EventEmitter);

module.exports = function(aFilename,
                          aInterval) {

  var obj = new AsyncWorker(aFilename,
    aInterval,
    function(aaId, aaName) {
      obj.emit('progress', aaId, aaName);
    },
    function(aaId) {
        obj.emit('end', aaId);
    });
  return obj;
};

AsyncWorker.prototype.start = function() {
  return this._start();
};
AsyncWorker.prototype.stop = function() {
  return this._stop();
};
AsyncWorker.prototype.id = function() {
  return this._id();
};

// extend prototype
function inherits(target, source) {
  for (var k in source.prototype) {
    target.prototype[k] = source.prototype[k];
  }
};
