solution "Symmetry"
	configurations { "Debug", "Release" }
	platforms {"x64"}
	location(_ACTION)

	configuration {"linux", "macosx"}
	    postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets " .. os.getcwd() .. "debug"}
	    postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets " .. os.getcwd() .. "release"}
		buildoptions {"-Wall", "-std=c99"}

	configuration {"windows", "vs2019"}	
		defines {"_CRT_SECURE_NO_WARNINGS"}
		flags {"NoIncrementalLink", "NoEditAndContinue"}
		local windowsPlatform = string.gsub(os.getenv("WindowsSDKVersion") or "10.0.16299.0", "\\", "")
		local action = premake.action.current()
		if(action ~= nil and _ACTION == "vs2019") then
			action.vstudio.windowsTargetPlatformVersion    = windowsPlatform
			action.vstudio.windowsTargetPlatformMinVersion = windowsPlatform
		end
	
   configuration "Debug"
	    if (_ACTION ~= nil and _ACTION ~= "postbuild_copy" and _ACTION ~= "build_addon") then
		   os.mkdir(_ACTION .. "/debug")
		   targetdir (_ACTION .. "/debug")
		end
		defines { "DEBUG" }
		flags { "Symbols" }

   configuration "Release"
	    if (_ACTION ~= nil and _ACTION ~= "postbuild_copy" and _ACTION ~= "build_addon") then
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
		  
		  -- if(_ARGS[1] == "vs2019") then
			 -- copy_dest_dir = "vs2019"
			 -- symlink_dest_dir = "..\\..\\..\\assets"
			 -- printf("Copying DLLs to visual studio build directory...\n")
		  -- end

		  -- -- Create sym links
		  -- local output = os.outputof("mklink /D vs2019\\debug\\assets ..\\..\\..\\assets" .. path.translate(copy_dest_dir, "\\")  .. "\\debug\\assets " .. symlink_dest_dir)
		  -- printf("MKlink debug output : %s", output)
		  -- output = os.outputof("mklink /D " .. path.translate(copy_dest_dir, "\\")  .. "\\release\\assets " ..symlink_dest_dir)
		  -- printf("MKlink release output : %s", output)
	   end
	}

	newaction {
		trigger = "generate_version_file",
		description = "Generate version.h from git revision number",
		execute  = function()
			local major_version = 0
			local minor_version = 1
			local revision_number = os.outputof("git rev-list --count HEAD")
			local branch = os.outputof("git rev-parse --abbrev-ref HEAD")

			revision_number = revision_number:gsub("%s+", "")
			branch = branch:gsub("%s+", "")

			print("Writing Version Number.....")
			io.output("../../src/common/version.h")
			io.write("#ifndef SYMMETRY_VERSION_FILE\n")
			io.write("#define SYMMETRY_VERSION_FILE\n\n")
			io.write("/* Auto generated version file. DO NOT MODIFY */\n")
			io.write("#define SYMMETRY_VERSION_MAJOR " .. major_version .. "\n")
			io.write("#define SYMMETRY_VERSION_MINOR " .. minor_version .. "\n")
			io.write("#define SYMMETRY_VERSION_REVISION " .. revision_number .. "\n")
			io.write("#define SYMMETRY_VERSION_BRANCH \"" .. branch .. "\"\n")
			io.write("\n#endif")
			io.close()

			io.output("version.txt")
			io.write(major_version .. "." .. minor_version .. "." .. revision_number .. "-" .. branch)
			io.close()
		end
	}

	-------------------------
	-- Game
	-------------------------
	project "Symmetry"
		kind "ConsoleApp"
		targetname "Symmetry"
		language "C"
		files { "../src/common/**.c", "../src/common/**.h", "../src/system/**.c", "../src/system/**.h", "../src/game/**.h", "../src/game/**.c"}
		includedirs {"../include/common"}
		defines {"USE_GLAD"}

		prebuildcommands
		{
			_PREMAKE_COMMAND .. ' generate_version_file'
		}

		configuration "linux"
		    includedirs {"../include/linux/sdl2/", "../include/common/soloud/", "../include/linux/"}
		    libdirs {"../lib/linux/sdl2/", "../lib/linux/soloud/", "../lib/linux/ode/"}
		    linkoptions {"'-Wl,-rpath,$$ORIGIN'"}
		    links {"SDL2", "m", "ode", "pthread"}

		configuration "macosx"
		    includedirs {"../include/mac/sdl2/", "../include/common/soloud/", "../include/mac/"}
		    libdirs {"../lib/mac/sdl2/", "../lib/mac/soloud/", "../lib/mac/ode/"}
		    links {"SDL2", "m", "ode", "pthread", "soloud"}

		configuration {"macosx", "Debug"}
			postbuildcommands
			{
				'cp ../../lib/mac/sdl2/libSDL2-2.0.0.dylib debug/',
				'cp ../../lib/mac/soloud/libsoloud.dylib debug/',
				'cp ../../lib/mac/ode/libode.0.16.0.dylib debug/',
				'install_name_tool -add_rpath @executable_path/. debug/Symmetry',
				'install_name_tool -change "/usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib" "@rpath/libSDL2-2.0.0.dylib" debug/Symmetry',
				'install_name_tool -change "/Users/shariqshah/Dev/ode/build_cmake/libode.0.16.0.dylib" "@rpath/libode.0.16.0.dylib" debug/Symmetry',
				'install_name_tool -change "/usr/local/lib/libsoloud.dylib" "@rpath/libsoloud.dylib" debug/Symmetry',
			}

		configuration {"macosx", "Release"}
			postbuildcommands
			{
				'cp ../../lib/mac/sdl2/libSDL2-2.0.0.dylib release/',
				'cp ../../lib/mac/soloud/libsoloud.dylib release/',
				'cp ../../lib/mac/ode/libode.0.16.0.dylib release/',
				'install_name_tool -add_rpath @executable_path/. release/Symmetry',
				'install_name_tool -change "/usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib" "@rpath/libSDL2-2.0.0.dylib" release/Symmetry',
				'install_name_tool -change "/Users/shariqshah/Dev/ode/build_cmake/libode.0.16.0.dylib" "@rpath/libode.0.16.0.dylib" release/Symmetry',
				'install_name_tool -change "/usr/local/lib/libsoloud.dylib" "@rpath/libsoloud.dylib" release/Symmetry',
				'cp version.txt release/'
			}

		configuration {"windows", "vs2019"}
		    includedirs	{"../include/windows/sdl2/", "../include/common/soloud/", "../include/windows/"}
		    libdirs {"../lib/windows/sdl2/", "../lib/windows/soloud/", "../lib/windows/ode/"}
			
		configuration {"not macosx", "Debug"}
		    links {"soloud_x64_d"}
		 
		configuration "Debug"
			defines {"GL_DEBUG_CONTEXT", "GL_BREAK_ON_ERROR"}
		
		configuration {"not macosx", "Release"}
			links {"soloud_x64"}
			
		configuration {"windows", "Release", "vs2019"}
			postbuildcommands 
			{
				"copy ..\\..\\lib\\windows\\sdl2\\SDL2.dll release\\ /Y",
				"copy ..\\..\\lib\\windows\\soloud\\soloud_x64.dll release\\ /Y",
				"copy ..\\..\\lib\\windows\\ode\\ode_double.dll release\\ /Y",
				"xcopy ..\\..\\assets ..\\..\\bin\\assets /s /e /h /i /y /d",
				"copy release\\Symmetry.exe ..\\..\\bin\\ /Y",
				"copy release\\SDL2.dll ..\\..\\bin\\ /Y",
				"copy release\\soloud_x64.dll ..\\..\\bin\\ /Y",
				"copy release\\ode_double.dll ..\\..\\bin\\ /Y",
				"copy version.txt ..\\..\\bin\\ /Y",
				"rmdir release\\assets",
				"mklink /D release\\assets ..\\..\\..\\assets"
			}
			links {"ode_double", "SDL2"}
			
		configuration {"windows", "Debug", "vs2019"}
			postbuildcommands 
			{
				"copy ..\\..\\lib\\windows\\sdl2\\SDL2.dll debug\\ /Y",
				"copy ..\\..\\lib\\windows\\soloud\\soloud_x64_d.dll debug\\ /Y",
				"copy ..\\..\\lib\\windows\\ode\\ode_doubled.dll debug\\ /Y",
				"copy ..\\..\\lib\\windows\\ode\\ode_doubled.pdb debug\\ /Y",
				"rmdir debug\\assets",
				"mklink /D debug\\assets ..\\..\\..\\assets"
			}
			links {"ode_doubled", "SDL2"}

		configuration {"not windows"}
			postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets " .. os.getcwd() .. "/" .. _ACTION .. "/debug"}
			postbuildcommands {"ln -fs " .. os.getcwd()  .. "/../assets " .. os.getcwd() .. "/" .. _ACTION .. "/release"}

	newaction {
	   trigger = "build_addon",
	   description = "Build blender addon into zip file that can be loaded into blender, needs zip installed and available on PATH(Only works on bash/nix-style shell for now)",
	   execute = function ()
	   		   local output = os.outputof("cd ../blender_addon && zip -r io_symmetry_exp.zip io_symmetry_exp/__init__.py io_symmetry_exp/exporter.py && mv io_symmetry_exp.zip ../build");
			   printf("Output of blender addon build : \n%s\n", output)
	   end
	}
