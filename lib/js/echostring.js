var events = require('events');
var util = require('util');

var EchoString  = require('../../build/Release/studyechostring');


util.inherits(EchoString, events.EventEmitter);


module.exports = EchoString;

