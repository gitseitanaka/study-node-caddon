{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "native/hello.cc" ],
	  "include_dirs" : [
	    "<!(node -e \"require('nan')\")"
	  ]
    },
    {
      "target_name": "timercb",
      "sources": [ "native/timer.cc" ],
	  "include_dirs" : [
	    "<!(node -e \"require('nan')\")"
	  ]
    }
  ]
}
