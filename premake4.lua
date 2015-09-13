solution "Symmetry"
configurations {"Debug", "Release"}
location "build"
language "C"
includedirs {"include/**"}
buildoptions {"-Wall", "-std=c11"}
linkoptions { "`pkg-config --libs --static glfw3`" }
links {"GLEW"}
local lib_base = "libs"

-- local cmd_stream = assert(io.popen("pkg-config --libs --static glfw3", r))
-- local libs_to_link = assert(cmd_stream:read("*all"))
-- cmd_stream:close()
-- libs_to_link = string.gsub(libs_to_link, "-lglfw", "")
-- libs_to_link = string.gsub(libs_to_link, "\n", "")
-- linkoptions {libs_to_link}

project "Symmetry"
kind "ConsoleApp"
files { "src/*.h", "src/*.c"}

configuration "Debug"
flags {"Symbols"}
libdirs {path.join(lib_base, "debug/**")}
links {"kazmath"}
targetdir "bin/debug"

configuration "Release"
defines {"NDEBUG"}
flags {"Optimize"}
libdirs {path.join(lib_base, "release/**")}
links {"kazmath"}
targetdir "bin/release"

