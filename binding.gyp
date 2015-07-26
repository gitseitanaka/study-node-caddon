{
    "targets": [{
        "target_name": "studyechostring",
        "sources": [ "lib/cpp/AsyncWorker.cc",
					 "lib/cpp/addon.cc"
					  ],
        "include_dirs" : [
            "<!(node -e \"require('nan')\")"
        ],
        'conditions': [
            ['OS=="linux"', {
                'cflags' : [ '-std=c++0x' ]
            }],
            ['OS=="win"', {
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'AdditionalOptions': [ '/EHsc' ]
                }
              }
            }],
            
        ]
    }]
}
