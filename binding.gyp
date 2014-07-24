{
  "targets":
  [
    {
      "target_name": "nj",
      "sources":     [ "nj.cpp" ],
      "cflags": [ "-std=c++11" ],
      "cflags!":     [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "conditions":
      [
        [ "OS=='mac'", { "xcode_settings": { "GCC_ENABLE_CPP_EXCEPTIONS": "YES" } } ] 
      ]
    }
  ]
}
