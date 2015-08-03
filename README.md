# Study(private) of C Addon using Nan module.


## Overview

* This repository is for my private study of 'node.js(native module)'.
* Using the following:
    - nan.h
    - uv_thread (unused uv_queue_work)
    - uv_async
    - uv_timer
    - uv_sem
    - node::ObjectWrap
    - EventEmitter
    - etc..
    
    It is not recommended to think web-scale. See <http://nikhilm.github.io/uvbook/threads.html#threads>.

* Other
    - node v0.12.5 / 0.10.26
    - nan@1.8.4
    - VS2015/2013/2012/2008, gcc version 4.8.4 (Ubuntu 4.8.4-2ubuntu1~14.04)

## Example
	
	$ # install
    $ #   if VS2012 or VS2008, add "--msvs_version=2012" or "--msvs_version=2008" 
	$ npm install study-caddon-echo-string
	$ cat strings.txt
	1 TEST 1
    2 TEST 2
    3 TEST 3
	$ cat example.js  
    var strEcho = require('study-caddon-echo-string');
    var testfilename = process.argv[2];
    
    var echo = strEcho(
      testfilename,         // strings file path
      150);                 // interval[ms]
    echo.on('progress',
      function (id, name) { // progress cb
        console.log(nowtimestring(), id, '-> \'' + name + '\'');
      });
    echo.on('end',
      function (id) {       // finish cb
      console.log(nowtimestring(), id, 'finished');
      });
    echo.start();
    setTimeout(function () {
      console.log(nowtimestring(), echo.id(), 'call stop');
      echo.stop();
    }, 2000);
    
    function nowtimestring() {
      var now = new Date();
      return ('0' + now.getHours()).slice(-2) + ':' +
        ('0' + now.getMinutes()).slice(-2)    + ':' +
        ('0' + now.getSeconds()).slice(-2)    + '.' +
        ('000' + now.getMilliseconds()).slice(-3);
    };
    $ node example.js strings.txt
    09:16:13.118 0 -> '1 TEST 1'
    09:16:13.261 0 -> '2 TEST 2'
    09:16:13.411 0 -> '3 TEST 3'
    09:16:13.562 0 -> '1 TEST 1'
    09:16:13.712 0 -> '2 TEST 2'
    09:16:13.862 0 -> '3 TEST 3'
    09:16:14.012 0 -> '1 TEST 1'
    09:16:14.162 0 -> '2 TEST 2'
    09:16:14.312 0 -> '3 TEST 3'
    09:16:14.463 0 -> '1 TEST 1'
    09:16:14.613 0 -> '2 TEST 2'
    09:16:14.763 0 -> '3 TEST 3'
    09:16:14.914 0 -> '1 TEST 1'
    09:16:14.958 0 call stop
    09:16:14.959 0 finished
    $

## Setting(for dev)
	
	# clone
	git clone https://github.com/gitseitanaka/study-node-caddon.git
	cd study-node-caddon
	
	# install
	#   if VS2012 or VS2008, add "--msvs_version=2012" or "--msvs_version=2008" 
	npm install
	
	# test
	npm test

## Debug(using visual studio (x64))

	# 1. clone node
	git clone https://github.com/joyent/node.git node.src
	cd node.src
	git fetch
	git checkout -b v0.10.26 origin/v0.10.26-release

	# 2. build node
	vcbuild.bat debug nosign x64

	# 3. rebuild c-addon
	cd study-node-caddon
	node-gyp clean
	node-gyp configure rebuild --debug --nodedir="C:\XXXX\node.src" 
	
	# 4. open vs solution file at build/binding.sln on visual studio
	
	# 5. setting project properties
	#    [Debug] -> [Command]     node.exe path
	#            -> [CommandArg] want to execute js
	#            -> [Work Dir]    ".."


## Version

* 0.0.0
   - publish.
* draft 0.74
    - Using 'EventEmitter'.
* draft 0.71
    - Using 'node::ObjectWrap'.
    - add example.
* draft 0.70
    - Using 'uv_thread' (unused 'NanAsyncProgressWorker')
* draft 0.60
    - Using 'NanAsyncProgressWorker'

## Licence

	The MIT License (MIT)
	
	Copyright (c) 2015 gitseitanaka
	
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	
	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
