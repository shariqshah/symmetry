-- A solution contains projects, and defines the available configurations
solution "Symmetry"
configurations { "Debug", "Release" }
location(_ACTION)
defines {"USE_GLAD"}
includedirs {"../include/"}
buildoptions {"-Wall", "-std=c99"}
if os.is("linux") then
   postbuildcommands {"ln -fs /mnt/Dev/Projects/symmetry/assets debug/"}
   postbuildcommands {"ln -fs /mnt/Dev/Projects/symmetry/assets release/"}
end

configuration "Debug"
if _ACTION ~= nil then
   os.mkdir(_ACTION .. "/debug")
   targetdir (_ACTION .. "/debug")
end
defines { "DEBUG" }
flags { "Symbols" }

configuration "Release"
if _ACTION ~= nil then
   os.mkdir(_ACTION .. "/release")
   targetdir (_ACTION .. "/release")
end
defines { "NDEBUG" }
flags { "OptimizeSpeed"}

-------------------------
-- Game
-------------------------
project "Game"
kind "WindowedApp"
targetname "Symmetry"
language "C"
files { "../src/common/**.c", "../src/game/**.c" }
defines {"GAME"}

configuration "linux"
buildoptions {"`pkg-config --cflags-only-other sdl2 openal`"}
linkoptions {"`pkg-config --libs sdl2 openal`"}
links {"m"}

-------------------------
-- libSymmetry
-------------------------
project "Library"
kind "SharedLib"
targetname "Symmetry"
language "C"
defines {"GAME_LIB"}
files { "../src/common/**.c", "../src/libsymmetry/**.c" }

configuration "Debug"
defines {"GL_DEBUG_CONTEXT", "AL_DEBUG"}
