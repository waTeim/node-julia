{
  "targets":
  [
    {
      "variables":
      {
         "version":"<!(node --version | sed -e 's/^v\([0-9]*\.[0-9]*\).*$/\\1.x/')"
      },
      "target_name": "nj",
      "sources":     
      [ 
        "Call.cpp",
        "Exception.cpp",
        "Immediate.cpp",
        "JMain.cpp",
        "JuliaExecEnv.cpp",
        "Type.cpp",
        "Value.cpp",
        "Values.cpp",
        "lvalue.cpp"
      ],
      "cflags!":     [ "-fno-exceptions" ],
      "cflags":      [ "-std=c++11" , "-I/usr/local/julia/include/julia" ],
      "cflags_cc!":  [ "-fno-exceptions" ],
      "conditions":
      [
        [ "OS=='mac'", 
          { 
            "xcode_settings":
            { 
              "MACOSX_DEPLOYMENT_TARGET":"10.7",
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "OTHER_CPLUSPLUSFLAGS": [ "-std=c++11" , "-stdlib=libc++" , "-I/usr/local/julia/include/julia" ],
              "OTHER_LDFLAGS":
              [
                "-stdlib=libc++",
                "-L/usr/local/julia/lib/julia",
                "-rpath /usr/local/julia/lib/julia",
                "-ljulia",
                "-lopenblas",
                "-ldSFMT"
              ]
            }
          }
        ],
        [ "version=='0.11.x'",
          {
             "sources": [ "nj-v11.cpp" ]
          }
        ]
      ]
    }
  ]
}
