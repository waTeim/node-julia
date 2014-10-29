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
            ],
            "juliaBase":"<!(python tools/find_julia.py <(OS))",
         },
         "version":"<!(python tools/node_version.py)",
         "njLib":"<!(python -c 'import os; print(os.path.abspath(\"\"))')/lib",
         "conditions":
         [
            [ "gcc=='4.6'", { "std":"c++0x" } , { "std":"c++11" } ],
            [ "OS=='linux' and juliaBase=='/usr'", 
               { "juliaLib":"<(juliaBase)/lib/x86_64-linux-gnu/julia" , "juliaInclude":"<(juliaBase)/include/julia" },
               { "juliaLib":"<(juliaBase)/lib/julia" , "juliaInclude":"<(juliaBase)/include/julia" }
            ]
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
        "src/Types.cpp",
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
         '-DNJ_LIB="<(njLib)"',
         '-DJULIA_LIB="<(juliaLib)"',
         "-I<(juliaInclude)",
      ],
      "cflags_cc!":  [ "-fno-exceptions" ],
      "link_settings":
      {
         "ldflags":
         [
            "-L<(juliaLib)",
            "-Wl,-rpath,<(juliaLib)",
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
                 '-DNJ_LIB="<(njLib)"',
                 '-DJULIA_LIB="<(juliaLib)"',
                 "-I<(juliaInclude)"
              ],
              "OTHER_LDFLAGS":
              [
                "-stdlib=libc++",
                "-L<(juliaLib)",
                "-Wl,-rpath,<(juliaLib)",
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
