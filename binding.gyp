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
          "juliaBase":"<!(python tools/nj_config.py <(OS) base)"
        },
        "version":"<!(python tools/nj_config.py <(OS) version)",
        "NJ_LIB":"<!(python tools/nj_config.py <(OS) nj_lib_define)",
        "juliaBin":"<(juliaBase)/bin",
        "V8MAJOR":"<!(node tools/nodev.js major)",
        "V8MINOR":"<!(node tools/nodev.js minor)",
        "V8PATCH":"<!(node tools/nodev.js patch)",
        "conditions":
        [
          [ "gcc=='4.6'", { "std":"c++0x" } , { "std":"c++11" } ],
          [ "OS=='linux' and juliaBase=='/usr'",
            {
              "juliaLib":"/usr/lib/x86_64-linux-gnu/julia",
              "JULIA_LIB":"/usr/lib/x86_64-linux-gnu/julia",
              "juliaInclude":"/usr/include/julia"
            },
            {
              "juliaLib":"<(juliaBase)/lib/julia",
              "JULIA_LIB":"<!(python tools/nj_config.py <(OS) julia_lib_define)",
              "juliaInclude":"<(juliaBase)/include/julia"
            }
          ],
          [ "OS=='win'",
            {
              "juliaLib":"<(juliaBase)/bin",
              "JULIA_LIB":"<!(python tools/nj_config.py <(OS) julia_lib_define)",
              "juliaInclude":"<(juliaBase)/include/julia",
              "libjulialib":"libjulia.lib"
            }
          ]
        ]
      },
      "rules":
      [
        {
          "rule_name": "Create Static LIBS",
          "extension": "dll",
          "inputs": "<(RULE_INPUT_NAME)",
          "outputs": "<(RULE_INPUT_NAME)",
          "action": [ "python","tools/create_julia_libs.py","--bin","<(juliaLib)","--files","<(RULE_INPUT_NAME)"],
          "message": "Generating <(RULE_INPUT_ROOT).lib"
        }
      ],
      "target_name": "nj",
      "sources":
      [
        "src/Alloc.cpp",
        "src/Call.cpp",
        "src/Exception.cpp",
        "src/Expr.cpp",
        "src/Convert.cpp",
        "src/Immediate.cpp",
        "src/Import.cpp",
        "src/JMain.cpp",
        "src/JuliaExecEnv.cpp",
        "src/JuAlloc.cpp",
        "src/JuliaHandle.cpp",
        "src/Kernel.cpp",
        "src/NAlloc.cpp",
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
         'JULIA_LIB="<(JULIA_LIB)"',
         'V8MAJOR=<(V8MAJOR)',
         'V8MINOR=<(V8MINOR)',
         'V8PATCH=<(V8PATCH)'
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
         "conditions":
         [
           [ "OS == 'win'",
             {
               "libraries":
               [
                 "-llibjulia",
                 "-llibopenlibm"
               ]
             },
             {
               "libraries":
               [
                 "-ljulia"
               ]
             }
           ]
         ]
      },
      "conditions":
      [
        [ "OS == 'mac'",
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
        [ "OS == 'win'",
          {
            "sources":
            [
              "<(juliaLib)/libjulia.dll",
              "<(juliaLib)/libopenlibm.dll",
            ],
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
                  "<(juliaLib)",
                ]
              }
            }
          }
        ],
        [ "version == '0.10.x'",
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
        [ "version != '0.10.x'",
          {
            "sources":
            [
              "src/JSAlloc.cpp",
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
