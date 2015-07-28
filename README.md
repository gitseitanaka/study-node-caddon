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
    - node v0.12.5 (0.10.26)
    - nan@1.8.4
    - VS2013/2012(2008(maybe)), gcc version 4.8.4 (Ubuntu 4.8.4-2ubuntu1~14.04)

## Example

	# see "example/example.js".
	node example.js
    

## Setting
	
	# clone
	git clone https://github.com/gitseitanaka/study-node-caddon.git
	cd study-node-caddon
	
	# install
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

* MIT

