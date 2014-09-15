{
  "targets":
  [
    {
      "variables":
      {
         "version":"<!(node --version | sed -e 's/^v\([0-9]*\.[0-9]*\).*$/\\1.x/')",
         "julia":"<!(python tools/find_julia.py <(OS))"
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
        "debug.cpp",
        "error.cpp",
        "lvalue.cpp",
        "request.cpp",
        "rvalue.cpp"
      ],
      "cflags!":     [ "-fno-exceptions" ],
      "cflags":      [ "-std=c++11" , '-DJULIA_DIR="<(julia)"' , "-I<(julia)/include/julia" ],
      "cflags_cc!":  [ "-fno-exceptions" ],
      "ldflags":
      [
         "-L<(julia)/lib/julia",
         "-Wl,-rpath,<(julia)/lib/julia",
         "-ljulia"
      ],
      "conditions":
      [
        [ "OS=='mac'", 
          { 
            "xcode_settings":
            { 
              "MACOSX_DEPLOYMENT_TARGET":"10.7",
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "OTHER_CPLUSPLUSFLAGS": [ "-std=c++11" , "-stdlib=libc++" , '-DJULIA_DIR="<(julia)"' , "-I<(julia)/include/julia" ],
              "OTHER_LDFLAGS":
              [
                "-stdlib=libc++",
                "-L<(julia)/lib/julia",
                "-Wl,-rpath,<(julia)/lib/julia",
                "-ljulia"
              ]
            }
          }
        ],
        [ "version=='0.10.x'",
          {
             "sources": [ "nj-v10.cpp" ]
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
