var AsyncWorker = require('../index.js');

var interval = 100 /* ms */;
var testfilename = 'example/teststrings.txt';

var nowtimestring = function () {
  var now = new Date();
  var nows = ('0' + now.getHours()).slice(-2) +
    ':' +
    ('0' + now.getMinutes()).slice(-2) +
    ':' +
    ('0' + now.getSeconds()).slice(-2) +
    '.' +
    ('000' + now.getMilliseconds()).slice(-3);
  return nows;
};

var echo1 = AsyncWorker(
  testfilename,         // strings file path
  150,                  // interval[ms]
  function (id, name) {  // progress cb
    console.log('[ECO1]', id, name, nowtimestring());
  },
  function (id) {        // finish
    console.log('[ECO1]', id, '****');
  }
  );

var echo2 = AsyncWorker(
  testfilename,         // strings file path
  100,                  // interval[ms]
  function (id, name) {  // progress cb
    console.log('[ECO2]', id, name, nowtimestring());
  },
  function (id) {        // finish
    console.log('[ECO2]', id, '****');
  }
  );

echo2.start();
echo1.start();

setTimeout(function () {
  console.log('[ECO1]', echo1.id(), 'stop called');
  echo1.stop();
}, 3000);
setTimeout(function () {
  console.log('[ECO2]', echo2.id(), 'stop called');
  echo2.stop();
}, 1000);
