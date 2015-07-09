Study(private) of C Addon using Nan module.

====

## Overview

* This repository is for my private study of 'node.js(native module)'.
* Using the following:
    - NanAsyncProgressWorker (uv_queue_work)
    - uv_async
    - uv_timer
    - uv_sem
    - etc..

* Other
    - node v0.12.5 (0.10.26)
    - nan@1.8.4
    - VS2013/2012(2008(mybe)), gcc version 4.8.4 (Ubuntu 4.8.4-2ubuntu1~14.04)

## APIs

### echoStringCyclic(aFilePath, aInterval, aCbProgress, aCbFinish) ###

+   `aFilePath` :
    the path of the file that string has been described. as format below:

		String 1
		String 2
		   :
		String n

+   `aInterval` :
    interval[ms]

+   `aCbProgress` : arg0 is a handle id. arg1 is a echoed string.

+   `aCbFinish` : notify finished. arg0 is a handle id.

+   `Return` : a handle id.

###echoStringCyclicAbort(aHandleId)###

+   `aHandleId` : a handle id.

## Sample

* see test/*.js

## Setting
	
	# clone
	git clone https://github.com/gitseitanaka/study-node-caddon.git
	cd study-node-caddon
	
	# install
	npm install
	
	# test
	npm test


## Lisence

* MIT

