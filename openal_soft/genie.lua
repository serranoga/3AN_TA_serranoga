solution("MySoundProgram")
  language("c++")
  configurations {"Debug", "Release"}
  platforms "x64"
  includedirs { "include/", "deps/openal-soft/include/" }
  location "build"
  objdir "build/obj"

  configuration "Debug"
    defines {"DEBUG"}
    flags {"FullSymbols"}
    targetdir "bin"
    targetsuffix "_d"

  configuration "Release"
    flags {"Optimize"}
    targetdir "bin"

  project("demo")
    kind "ConsoleApp"
    files { "./src/*.cc" }
    libdirs { "deps/openal-soft/libs/Win64" }
    links { "OpenAL32" }
