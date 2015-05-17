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
            "juliaBase":"<!(python tools/nj_config.py <(OS) find)",
         },
         "version":"<!(python tools/nj_config.py <(OS) version)",
         "NJ_LIB":"<!(python tools/nj_config.py <(OS) nj_lib)",
         "juliaBin":"<(juliaBase)/bin",
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
        "src/Expr.cpp",
        "src/Convert.cpp",
        "src/Immediate.cpp",
        "src/Import.cpp",
        "src/JMain.cpp",
        "src/JuliaExecEnv.cpp",
        "src/JuliaHandle.cpp",
        "src/Kernel.cpp",
        "src/NativeArray.cpp",
        "src/Script.cpp",
        "src/Trampoline.cpp",
        "src/Type.cpp",
        "src/Types.cpp",
        "src/Value.cpp",
        "src/Values.cpp",
        "src/debug.cpp",
        "src/error.cpp",
        "src/lvalue.cpp",
        "src/request.cpp",
        "src/rvalue.cpp",
        "src/util.cpp"
      ],
      "cflags!":     [ "-fno-exceptions" ],
      "cflags":
      [
         "-std=<(std)",
      ],
      "defines":
      [
         '<(OS)',
         'NJ_LIB="<(NJ_LIB)"',
         'JULIA_LIB="<(juliaLib)"'
      ],
      "cflags_cc!":  [ "-fno-exceptions" ],
      "include_dirs":
      [
         "<(juliaInclude)"
      ],
      "link_settings":
      {
         "ldflags":
         [
            "-L<(juliaLib)",
            "-Wl,-rpath,<(juliaLib)"
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
                 "-std=c++11",
                 "-stdlib=libc++",
              ],
              "OTHER_LDFLAGS":
              [
                "-stdlib=libc++",
                "-L<(juliaLib)",
                "-Wl,-rpath,<(juliaLib)",
                "-Wl,-flat_namespace"
              ]
            }
          }
        ],
        [ "OS=='win'",
          {
            "msvs_settings":
            {
              "VCCLCompilerTool":
              {
                "AdditionalOptions": [ "/EHa" ],
                "DisableSpecificWarnings": 
                [
                  4290,
                  4200
                ]
              },
              "VCLinkerTool":
              {
                "AdditionalLibraryDirectories":
                [
                  "<(juliaBin)"
                ]
              }
            }
          }
        ],
        [ "version=='0.10.x'",
          {
            "sources":
            [
              "src/Callback-v10.cpp",
              "src/JRef-v10.cpp",
              "src/ScriptEncapsulated-v10.cpp",
              "src/dispatch-v10.cpp",
              "src/nj-v10.cpp"
            ]
          }
        ],
        [ "version!='0.10.x'",
          {
            "sources":
            [
              "src/Callback-v11.cpp",
              "src/JRef-v11.cpp",
              "src/ScriptEncapsulated-v11.cpp",
              "src/dispatch-v11.cpp",
              "src/nj-v11.cpp"
            ]
          }
        ]
      ]
    }
  ]
}
