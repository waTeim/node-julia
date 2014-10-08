{
  "targets":
  [
    {
      "variables":
      {
         "version":"<!(node --version | sed -e 's/^v\([0-9]*\.[0-9]*\).*$/\\1.x/')",
         "julia":"<!(python tools/find_julia.py <(OS))",
         "libDir":"<!(python -c 'import os; print(os.path.abspath(\"\"))')/lib"
      },
      "target_name": "nj",
      "sources":     
      [ 
        "Call.cpp",
        "Exception.cpp",
        "Immediate.cpp",
        "JMain.cpp",
        "JuliaExecEnv.cpp",
        "JuliaHandle.cpp",
        "Script.cpp",
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
      "cflags":
      [ 
         "-DOS=<(OS)",
         "-std=c++11",
         '-DJULIA_DIR="<(julia)"',
         '-DLIB_DIR="<(libDir)"',
         "-I<(julia)/include/julia"
      ],
      "cflags_cc!":  [ "-fno-exceptions" ],
      "link_settings":
      {
         "ldflags":
         [
            "-L<(julia)/lib/julia",
            "-Wl,-rpath,<(julia)/lib/julia",
         ],
         "libraries":
         [
            "-ljulia"
         ]
      },
      "conditions":
      [
        [ "OS=='mac'", 
          { 
            "xcode_settings":
            { 
              "MACOSX_DEPLOYMENT_TARGET":"10.7",
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "OTHER_CPLUSPLUSFLAGS":
              [
                 "-DOS=<(OS)",
                 "-std=c++11",
                 "-stdlib=libc++",
                 '-DJULIA_DIR="<(julia)"',
                 '-DLIB_DIR="<(libDir)"',
                 "-I<(julia)/include/julia"
              ],
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
             "sources":
            [ 
               "ScriptEncapsulated-v10.cpp",
               "nj-v10.cpp"
            ]
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
