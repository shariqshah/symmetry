solution "Symmetry"
	configurations { "Debug", "Release" }
	platforms {"x64"}
	location(_ACTION)

	configuration {"linux"}
	    postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets debug/assets"}
	    postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets release/assets"}
		buildoptions {"-Wall", "-std=c99"}

	configuration {"windows", "vs2017"}	
		defines {"_CRT_SECURE_NO_WARNINGS"}
		flags {"NoIncrementalLink", "NoEditAndContinue"}
		local windowsPlatform = string.gsub(os.getenv("WindowsSDKVersion") or "10.0.16299.0", "\\", "")
		local action = premake.action.current()
		if(action ~= nil and _ACTION == "vs2017") then
			action.vstudio.windowsTargetPlatformVersion    = windowsPlatform
			action.vstudio.windowsTargetPlatformMinVersion = windowsPlatform
		end
	
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
		
	newaction {
	   trigger = "postbuild_copy",
	   description = "Action to copy relevant dlls to executable directory after build",
	   execute = function ()
		  -- local copy_dest_dir = ""
		  -- local symlink_dest_dir = ""
		  
		  -- if(_ARGS[1] == "vs2017") then
			 -- copy_dest_dir = "vs2017"
			 -- symlink_dest_dir = "..\\..\\..\\assets"
			 -- printf("Copying DLLs to visual studio build directory...\n")
		  -- end

		  -- -- Create sym links
		  -- local output = os.outputof("mklink /D vs2017\\debug\\assets ..\\..\\..\\assets" .. path.translate(copy_dest_dir, "\\")  .. "\\debug\\assets " .. symlink_dest_dir)
		  -- printf("MKlink debug output : %s", output)
		  -- output = os.outputof("mklink /D " .. path.translate(copy_dest_dir, "\\")  .. "\\release\\assets " ..symlink_dest_dir)
		  -- printf("MKlink release output : %s", output)
	   end
	}

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
			includedirs	{"../include/linux/sdl2/", "../include/common/soloud/", "../include/linux/"}
			libdirs {"../lib/linux/sdl2/", "../lib/linux/soloud/", "../lib/linux/ode/"}
			links {"SDL2", "m", "ode", "pthread"}

		configuration {"windows", "vs2017"}
		   includedirs	{"../include/windows/sdl2/", "../include/common/soloud/", "../include/windows/"}
		   libdirs {"../lib/windows/sdl2/", "../lib/windows/soloud/", "../lib/windows/ode/"}
		   links {"SDL2"}
			
		configuration "Debug"
			links {"soloud_x64_d"}
		
		configuration "Release"
			links {"soloud_x64"}
			
		configuration {"windows", "Release", "vs2017"}
			postbuildcommands 
			{
				"copy ..\\..\\lib\\windows\\sdl2\\SDL2.dll release\\ /Y",
				"copy ..\\..\\lib\\windows\\soloud\\soloud_x64.dll release\\ /Y",
				"copy ..\\..\\lib\\windows\\ode\\ode_double.dll release\\ /Y",
				"xcopy ..\\..\\assets ..\\..\\bin\\assets /s /e /h /i /y /d",
				"copy release\\Symmetry.exe ..\\..\\bin\\ /Y",
				"copy release\\libSymmetry.dll ..\\..\\bin\\ /Y",
				"copy release\\SDL2.dll ..\\..\\bin\\ /Y",
				"copy release\\soloud_x64.dll ..\\..\\bin\\ /Y",
				"copy release\\ode_double.dll ..\\..\\bin\\ /Y",
				"rmdir release\\assets",
				"mklink /D release\\assets ..\\..\\..\\assets"
			}
			links {"ode_double"}
			
		configuration {"windows", "Debug", "vs2017"}
			postbuildcommands 
			{
				"copy ..\\..\\lib\\windows\\sdl2\\SDL2.dll debug\\ /Y",
				"copy ..\\..\\lib\\windows\\soloud\\soloud_x64_d.dll debug\\ /Y",
				"copy ..\\..\\lib\\windows\\ode\\ode_doubled.dll debug\\ /Y",
				"copy ..\\..\\lib\\windows\\ode\\ode_doubled.pdb debug\\ /Y",
				"rmdir debug\\assets",
				"mklink /D debug\\assets ..\\..\\..\\assets"
			}
			links {"ode_doubled"}
	-------------------------
	-- libSymmetry
	-------------------------
	project "Library"
		kind "SharedLib"
		language "C"
		targetname "Symmetry"
		defines {"GAME_LIB", "USE_GLAD"}
		includedirs {"../include/common"}
		files { "../src/common/**.c", "../src/common/**.h", "../src/libsymmetry/**.h", "../src/libsymmetry/**.c" }

		configuration "windows"
			targetname "libSymmetry"
			
		configuration {"windows", "vs2017"}
			includedirs {"../include/windows/sdl2/"}
			flags "NoImportLib"


		configuration {"linux"}
			includedirs {"../include/linux/sdl2/"}
		
		configuration "Debug"
			defines {"GL_DEBUG_CONTEXT"}
