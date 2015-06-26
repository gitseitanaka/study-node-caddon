{
  "targets": [
    {
      "target_name": "echostring",
      "sources": [ "lib/cpp/echostring.cc" ],
	  "include_dirs" : [
	    "<!(node -e \"require('nan')\")"
	  ]
    }
  ]
}
