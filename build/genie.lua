solution "Symmetry"
	configurations { "Debug", "Release" }
	platforms {"x64"}
	location(_ACTION)
	defines {"USE_GLAD"}
	includedirs {"../include/"}

	configuration {"linux"}
	    postbuildcommands {"ln -fs /mnt/Dev/Projects/symmetry/assets debug/"}
		postbuildcommands {"ln -fs /mnt/Dev/Projects/symmetry/assets release/"}
		buildoptions {"-Wall", "-std=c99"}


   configuration {"windows"}

	   includedirs {"../third_party/windows/SDL2-2.0.5/include/", "../third_party/windows/OpenAL/include/"}
	   -- postbuildcommands {"mklink /D debug\\assets ..\\..\\..\\assets"}
	   -- postbuildcommands {"mklink /D release\\assets ..\\..\\..\\assets"}

	   local sdl_lib_dir    = "../third_party/windows/SDL2-2.0.5/lib/x64/"
	   local openal_lib_dir = "../third_party/windows/OpenAL/lib/"
	   
	   defines {"_CRT_SECURE_NO_WARNINGS"}
	   flags {"NoIncrementalLink"}

    configuration "Debug"
	    if _ACTION ~= nil and _ACTION ~= "post_build_copy_dll" then
		   os.mkdir(_ACTION .. "/debug")
		   targetdir (_ACTION .. "/debug")
		end
		defines { "DEBUG" }
		flags { "Symbols" }

    configuration "Release"
	    if _ACTION ~= nil and _ACTION ~= "post_build_copy_dll" then
		   os.mkdir(_ACTION .. "/release")
		   targetdir (_ACTION .. "/release")
		end
		defines { "NDEBUG", "ExtraWarnings" }
		flags { "OptimizeSpeed"}

		-------------------------
		-- Game
		-------------------------
		project "Game"
		    kind "ConsoleApp"
			targetname "Symmetry"
			language "C"
			files { "../src/common/**.c", "../src/common/**.h", "../src/game/**.c", "../src/game/**.h" }
			defines {"GAME"}

			configuration "linux"
				buildoptions {"`pkg-config --cflags-only-other sdl2 openal`"}
				linkoptions {"`pkg-config --libs sdl2 openal`"}
				links {"m"}

			configuration "windows"
			    libdirs { sdl_lib_dir, openal_lib_dir }
			    links {"SDL2", "OpenAL32"}
				
				newaction {
				   trigger = "post_build_copy_dll",
				   description = "Action to copy relevant dlls to executable directory after build",
				   execute = function ()
					  printf("Copying DLLs to executable directory...\n")
					  local success = false
					  success = os.copyfile(sdl_lib_dir .. "SDL2.dll", "vs2017/debug/SDL2.dll")
					  success = os.copyfile(sdl_lib_dir .. "SDL2.dll", "vs2017/release/SDL2.dll")
					  success = os.copyfile("../third_party/windows/OpenAL/bin/OpenAL32.dll", "vs2017/debug/OpenAL32.dll")
					  success = os.copyfile("../third_party/windows/OpenAL/bin/OpenAL32.dll", "vs2017/release/OpenAL32.dll")

					  if success ~= true then
						 printf("Copying dlls failed.")
					  else
						 printf("Copying dlls successful.")
					  end
				   end
				}
				
				postbuildcommands
				{
				   _PREMAKE_COMMAND .. " post_build_copy_dll"
				}

		-------------------------
		-- libSymmetry
		-------------------------
		project "Library"
		    kind "SharedLib"
			targetname "libSymmetry"
			language "C"
			defines {"GAME_LIB"}
			files { "../src/common/**.c", "../src/common/**.h", "../src/libsymmetry/**.h", "../src/libsymmetry/**.c" }

		configuration "Debug"
		    defines {"GL_DEBUG_CONTEXT", "AL_DEBUG"}
