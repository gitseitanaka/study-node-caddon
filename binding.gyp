{
    "targets": [{
        "target_name": "study-echostring",
        "sources": [ "lib/cpp/echostring.cc" ],
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
