* This repository is for my personal study of 'node.js'.

* node v0.12.2
* nan@1.8.4


APIs
----
### echoStringCyclic(aFilePath, aInterval, aCbProgress, aCbFinish) ###

+   `aFilePath` :
    the path of the file that string has been described. as format below:

		String 1
		String 2
		   ÅF
		String n

+   `aInterval` :
    interval[ms]

+   `aCbProgress` : arg0 is a handle id. arg1 is a echoed string.

+   `aCbFinish` : notify finished. arg0 is a handle id.

+   `Return` : a handle id.

###echoStringCyclicAbort(aHandleId)###

+   `aHandleId` : a handle id.


