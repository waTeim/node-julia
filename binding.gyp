{
  "targets":
  [
    {
      "variables":
      {
         "variables":
         {
            "conditions":
            [
               [ "OS=='linux'", { "gcc":"<!(gcc --version 2>&1 | head -1 | sed -e 's/^.*(.*) \(.*\)\..*$/\\1/')" } , { "gcc":"" } ]
            ]
         },
         "version":"<!(node --version | sed -e 's/^v\([0-9]*\.[0-9]*\).*$/\\1.x/')",
         "julia":"<!(python tools/find_julia.py <(OS))",
         "libDir":"<!(python -c 'import os; print(os.path.abspath(\"\"))')/lib",
         "conditions":
         [
            [ "gcc=='4.6'", { "std":"c++10" } , { "std":"c++11" } ]
         ]
      },
      "target_name": "nj",
      "sources":     
      [ 
        "src/Call.cpp",
        "src/Exception.cpp",
        "src/Immediate.cpp",
        "src/JMain.cpp",
        "src/JuliaExecEnv.cpp",
        "src/JuliaHandle.cpp",
        "src/Script.cpp",
        "src/Type.cpp",
        "src/Value.cpp",
        "src/Values.cpp",
        "src/debug.cpp",
        "src/error.cpp",
        "src/lvalue.cpp",
        "src/request.cpp",
        "src/rvalue.cpp"
      ],
      "cflags!":     [ "-fno-exceptions" ],
      "cflags":
      [ 
         "-DOS=<(OS)",
         "-std=<(std)",
         '-DJULIA_DIR="<(julia)"',
         '-DLIB_DIR="<(libDir)"',
         "-I<(julia)/include/julia",
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
              "src/ScriptEncapsulated-v10.cpp",
              "src/nj-v10.cpp"
            ]
          }
        ],
        [ "version=='0.11.x'",
          {
             "sources":
             [ 
               "src/ScriptEncapsulated-v11.cpp",
               "src/nj-v11.cpp"
             ]
          }
        ]
      ]
    }
  ]
}
