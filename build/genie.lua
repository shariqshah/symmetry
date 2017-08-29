solution "Symmetry"
	configurations { "Debug", "Release" }
	platforms {"x64"}
	location(_ACTION)
	defines {"USE_GLAD"}
	includedirs {"../include/"}

	configuration {"linux"}
	    postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets debug/assets"}
	    postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets release/assets"}
		buildoptions {"-Wall", "-std=c99"}

	configuration {"windows", "gmake"}
	    postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets debug/assets"}
	    postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets release/assets"}
		buildoptions {"-Wall", "-std=c99"}


   configuration {"windows", "vs2017 or qbs"}
	   includedirs {"../third_party/windows/SDL2-2.0.5/include/", "../third_party/windows/OpenAL/include/"}

	   local sdl_lib_dir    = "../third_party/windows/SDL2-2.0.5/lib/x64/"
	   local openal_lib_dir = "../third_party/windows/OpenAL/lib/"
	   
	   defines {"_CRT_SECURE_NO_WARNINGS"}
	   flags {"NoIncrementalLink", "NoEditAndContinue"}

   configuration "Debug"
	    if (_ACTION ~= nil and _ACTION ~= "postbuild_copy") then
		   os.mkdir(_ACTION .. "/debug")
		   targetdir (_ACTION .. "/debug")
		end
		defines { "DEBUG" }
		flags { "Symbols" }

   configuration "Release"
	    if (_ACTION ~= nil and _ACTION ~= "postbuild_copy") then
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

			configuration {"windows", "gmake"}
				buildoptions {"`pkg-config --cflags-only-I sdl2 openal`"}
				linkoptions {"`pkg-config --libs sdl2 openal`"}
				links {"m"}

			configuration {"windows", "vs2017 or qbs"}
			    libdirs { sdl_lib_dir, openal_lib_dir }
			    links {"SDL2", "OpenAL32"}
				
				newaction {
				   trigger = "postbuild_copy",
				   description = "Action to copy relevant dlls to executable directory after build",
				   execute = function ()
					  local copy_dest_dir = ""
					  local symlink_dest_dir = ""
					  
					  if(_ARGS[1] == "vs2017") then
						 copy_dest_dir = "vs2017"
						 symlink_dest_dir = "..\\..\\..\\assets"
						 printf("Copying DLLs to visual studio build directory...\n")
					  else
						 copy_dest_dir = "projects/qbs"
						 symlink_dest_dir = "..\\..\\..\\..\\assets"
						 printf("Copying DLLs to qbs build directory...\n")
					  end

					  local success = false
					  success = os.copyfile(sdl_lib_dir .. "SDL2.dll", copy_dest_dir .. "/debug/SDL2.dll")
					  success = os.copyfile(sdl_lib_dir .. "SDL2.dll", copy_dest_dir .. "/release/SDL2.dll")
					  success = os.copyfile("../third_party/windows/OpenAL/bin/OpenAL32.dll", copy_dest_dir .. "/debug/OpenAL32.dll")
					  success = os.copyfile("../third_party/windows/OpenAL/bin/OpenAL32.dll", copy_dest_dir .. "/release/OpenAL32.dll")

					  if success ~= true then
						 printf("Copying one or more dlls failed.")
					  else
						 printf("Copying dlls successful.")
					  end

					  -- Create sym links
					  local output = os.outputof("mklink /D " .. path.translate(copy_dest_dir, "\\")  .. "\\debug\\assets " .. symlink_dest_dir)
					  printf("MKlink debug output : %s", output)
					  output = os.outputof("mklink /D " .. path.translate(copy_dest_dir, "\\")  .. "\\release\\assets " ..symlink_dest_dir)
					  printf("MKlink release output : %s", output)
				   end
				}
				
				if(_ACTION == "vs2017") then 
					postbuildcommands
					{
					   _PREMAKE_COMMAND .. " postbuild_copy vs2017"
					}
				else
					postbuildcommands
					{
					   _PREMAKE_COMMAND .. " postbuild_copy qbs"
					}
				end
		-------------------------
		-- libSymmetry
		-------------------------
		project "Library"
		    kind "SharedLib"
			targetname "libSymmetry"
			language "C"
			defines {"GAME_LIB"}
			files { "../src/common/**.c", "../src/common/**.h", "../src/libsymmetry/**.h", "../src/libsymmetry/**.c" }

			configuration {"windows", "gmake"}
				buildoptions {"`pkg-config --cflags-only-I sdl2`"}
				
			configuration "Debug"
		        defines {"GL_DEBUG_CONTEXT", "AL_DEBUG"}
