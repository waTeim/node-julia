{
  "targets":
  [
    {
      "target_name": "nj",
      "sources":     [ "nj.cpp" , "Exception.cpp" , "JMain.cpp" , "JuliaExecEnv.cpp" ],
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
              "OTHER_CPLUSPLUSFLAGS": [ "-stdlib=libc++" ],
              "OTHER_LDFLAGS": ["-stdlib=libc++"]
            }
          }
        ] 
      ]
    }
  ]
}
