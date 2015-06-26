{
  "targets": [
    {
      "target_name": "timercb",
      "sources": [ "lib/timer.cc" ],
	  "include_dirs" : [
	    "<!(node -e \"require('nan')\")"
	  ]
    }
  ]
}
