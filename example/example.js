var AsyncWorker = require('../index.js');

var testfilename = 'example/teststrings.txt';

var nowtimestring = function () {
  var now = new Date();
  return ('0' + now.getHours()).slice(-2) +
    ':' +
    ('0' + now.getMinutes()).slice(-2) +
    ':' +
    ('0' + now.getSeconds()).slice(-2) +
    '.' +
    ('000' + now.getMilliseconds()).slice(-3);
};

var echo1 = AsyncWorker(
  testfilename,             // strings file path
  150);                     // interval[ms]
echo1.on('progress',
      function (id, name) { // progress cb
        console.log('[ECO1]', id, name, nowtimestring());
      });
echo1.on('end',
      function (id) {       // finish cb
        console.log('[ECO1]', id, '****');
      });

var echo2 = AsyncWorker(
  testfilename,             // strings file path
  100);                     // interval[ms]
                            // progress cb
echo2.on('progress', function (id, name) {
    console.log('[ECO2]', id, name, nowtimestring());
  });
                            // finish cb
echo2.on('end', function (id) {
    console.log('[ECO2]', id, '****');
  });

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
