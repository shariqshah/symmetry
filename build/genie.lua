solution "Symmetry"
	configurations { "Debug", "Release" }
	platforms {"x64"}
	location(_ACTION)
	defines {"USE_GLAD"}
	includedirs {"../include/"}

	configuration {"linux"}
	    postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets debug/assets"}
	    postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets release/assets"}
		buildoptions {"-Wall", "-std=c99", "`pkg-config --cflags-only-I sdl2`"}

	configuration {"windows", "gmake"}
	    postbuildcommands {"rm -rf debug/assets"}
	    postbuildcommands {"rm -rf release/assets"}
	    postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets debug/assets"}
	    postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets release/assets"}
		buildoptions {"-Wall", "-std=c99"}


   configuration {"windows", "vs2017 or qbs"}
	local sdl_lib_dir    = "../third_party/windows/SDL2-2.0.5/lib/x64/"
	local soloud_lib_dir = "../third_party/windows/Soloud/lib/"
	
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
			files { "../src/common/**.c", "../src/common/**.h", "../src/game/**.c", "../src/game/**.h"}
			defines {"GAME"}
			
			configuration "linux"
			    includedirs	{"../third_party/linux/SDL2/include/", "../third_party/linux/Soloud/include/"}
				buildoptions {"`pkg-config --cflags-only-other sdl2`"}
				linkoptions {"`pkg-config --libs sdl2`"}
				libdirs {"../third_party/linux/Soloud/lib/"}
				links {"m"}

			configuration {"windows", "gmake"}
				buildoptions {"`pkg-config --cflags-only-I sdl2`"}
				linkoptions {"`pkg-config --libs sdl2`"}
				links {"m"}

			configuration {"windows", "vs2017 or qbs"}
			   includedirs {"../third_party/windows/SDL2-2.0.5/include/", "../third_party/windows/Soloud/include/"}
			   libdirs { sdl_lib_dir, soloud_lib_dir }
			   links {"SDL2"}
				
			configuration "Debug"
				links {"soloud_x64_d"}
			
			configuration "Release"
				links {"soloud_x64"}
				
			configuration {"windows", "Release", "vs2017"}
				postbuildcommands {
					"xcopy ..\\..\\assets ..\\..\\bin\\assets /s /e /h /i /y /d",
					"copy release\\Symmetry.exe ..\\..\\bin\\ /Y",
					"copy release\\libSymmetry.dll ..\\..\\bin\\ /Y",
					"copy release\\SDL2.dll ..\\..\\bin\\ /Y",
					"copy release\\soloud_x64.dll ..\\..\\bin\\ /Y",
				}
				
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
					  success = os.copyfile(soloud_lib_dir .. "soloud_x64_d.dll", copy_dest_dir .. "/debug/soloud_x64_d.dll")
					  success = os.copyfile(soloud_lib_dir .. "soloud_x64.dll", copy_dest_dir .. "/release/soloud_x64.dll")
					  

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
			language "C"
			targetname "Symmetry"
			defines {"GAME_LIB"}
			files { "../src/common/**.c", "../src/common/**.h", "../src/libsymmetry/**.h", "../src/libsymmetry/**.c" }

			configuration "windows"
			    targetname "libSymmetry"
				
			configuration {"windows", "vs2017"}
				includedirs {"../third_party/windows/SDL2-2.0.5/include/"}
				flags "NoImportLib"


			configuration {"linux"}
			    includedirs {"../third_party/linux/SDL2/include/"}
			
			configuration "Debug"
		        defines {"GL_DEBUG_CONTEXT"}
