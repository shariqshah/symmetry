# Project Symmetry

- ## What?
	A topdown 2D shooter exploring symmetry.

- ## License
	All the code in this repository is under GPLv3, see LICENSE for more information

- ## File format specifications

   - ### Entity

		```.conf
		# Comment, Sample entity definition in file, paremeters left out are set to defaults
		# Empty line at the end specifies end of entity definition
		entity:   "Something"
		position: 0 0 0
		scale:    1 1 1
		rotation: 0 0 0 1
		model:    "suzanne.pamesh"
		material: "blinn_phong"
		diffuse_color: 1 0 0 1
		diffuse_texture: "checkered.tga"
		specular: 0.55
		```

   - Add to_string functions for major structs like transform, model etc to ease in conversion?
   - ### Configuration Variables a.k.a cfg-vars

		```.conf
		# Comment
		render_width: 1024
		render_height: 1024
		debug_draw_enabled: true
		fog_color: 0.5 0.2 0.2 1
		# There can be comments or empty newlines in between unlike entity definitions

		ambient_light: 0.1 0.1 0.1 1
		msaa: true
		msaa_levels: 8

		```

   - ### Keybindings

       ```.conf
       # All keys are parsed by comparing the output of SDL_GetKeyname
	   # Each line represents a keybinding
	   Move_Forward: W

       # Multiple keys to a single binding are specified with commas
	   Move_Backward: S,Down

       # Combinations are specified with a hyphen/dash
	   # When specifing combinations, modifiers(shift, alt, ctrl) always come before
	   # the hyphen and the actual key comes afterwards. At the moment modifier keys are
	   # forced to be on the left side i.e. Left Control, Left Shift and Left Alt.
	   Quit: Left Ctrl-Q

       # Single modifier keys are allowed but multiple modifier keys without corresponding
	   # non-modifier key are not allowed
	   Sprint: Left Shift
	   ```

   - ### Level/Scene

	- Binary format with header attached at the top
	- Save child entities first
	- Copy paste all entites in the file one by one. Since the entites all look
	  the same in memory and are made up of tagged unions, a simple memcpy approach
	  should suffice. The problem is entity heirarchies. There are multiple approaches to
	  solve this problem.
	  - Save a sorted list of entites to file i.e. before saving create a new list that does
		not have the empty array slots in the entity list and then just copy and paste. This
		is the simplest way to solve the problem as we don't have to worry about indexes of
		parent/child entites in heirarchy. We can take the whole array and paste it to the
		file but creating a copy of entity list for this purpose only would be slow and consume a   lot of memory.
	  - Instead of creating a copy of the entity list for sorting and saving, sort the actual   entity list
		and update all references as necessary then save the array to file.
	  - Just write the name of the parent entity as parent. Make sure that all entity names are unique.
	- Use separate EntityDefinition file that serves as a blueprint/prefab for the entity
	  to load/save. When the entity is saved in  a scene file, the scene file only needs to
	  refer to the entity's EntityDefinition file/asset along with it's parent and children
	  - This approach requires seperating a scene into mutable/immutable parts.
	    Meaning, entities that can change their state during the duaration of the level are
		mutable and those that remain the same as they were defined in their EntityDefinition
		file are immutable.
	  - In each level there going to be mutable entites i.e player and player's position/orientation, objectives
	    cleared/remaining, doors opened and puzzles	solved etc. Instead of handling all of these in the
		scene file, we save all the mutable	state in the savegame files. When restoring game's state from a save game we will need
		to handle loading of a scene and then applying the mutable state to entites after loading.
	  - Entities can have (a fixed number of?) properties. Each property has a name and a corresponding
	    variant value like, health or ammo etc. But, how to save/load all of that?

  - ### Materials
  - ### Mesh/Geometry

- ### Notes on entity Systems

	- Fat entites with all related properties, i.e. position, mesh etc in them. Easy to serialize, memory friendly, simple to implement but would require significant changes to the current codebase, for example:

	```.C
   	struct Entity
	{
		int    type;
		char*  name;
		struct Transform {....};
		struct Camera {....};

		// Separate properties unique to entity types by using unions
		struct Renderable
		{
			struct Model {....};
			union
			{
				struct Player
				{
					int score;
					int bullets;
				};

				struct Enemy
				{
					int target;
				};
			}
		}
	};
	```

   - Change component implementation by using anonymous unions to simulate interfaces. e.g

	```.C
     struct Component
	 {
		int type;
		union
		{
			struct Transform {....};
			struct Model {....};
			struct Camera {....};
		}
	 }
	 ```

   - Use handles for assets
   - Use something similar to Variant to use as entity, not sure what or how
   - Don't forget to think of the actual use-case and usage when coming up with a solution, don't build castles in the air!

- ## TODO

    - Move game, common and game library related code into separate folders.
	- Replace OpenAL with soloud
	- Remove hardcoded numerical values from sscanf and other format strings.
	- Finish entity loading from file then move on to 2D rendering
	- First class 2d rendering
	  - Sprite batching (XNA like)
	  - Font rendering(2d/3d) with stb_ttf or freetype
	  ? Minimal custom UI for in-game usage that can be rendered to a texture or modify nuklear for that?
	- Bounding Boxes
	  ? Recalculated bounding boxes for rotated meshes
	- Wrap malloc and free calls in custom functions to track usage
	- File extension checking for asset loading
	- Only allocate hashmap bucket when required
	- Mapping actions to keybindings, for example map action "Jump" to Space key etc
	- Ability to mark meshes for debug rendering with possibility of different color for each?
	- Switch to completely static allocation of entites i.e. have a static array of MAX_ENTITIES 	size. This way we can store pointers to entites and they'll still be in an array and fast to 	process.
	- Add marking or queuing up custom meshes for debug render with particular transform and color 	for rendering bounding spheres for example
	- Interleaved vbos for meshes and changes to blender exporter accordingly
	- Enumerate and save all the uniform and attribute positions in shader when it is added and 	cache them in shader object?
	- Physics/Collision detection in 2d
	- Complete gui integration
	- Decoupled event handling of gui and input if possible
	- Custom rendering for gui
	- Allow passsing base path as commandline argument?
	- Remove components and switch to "Fat Entities" i.e. one entity struct contains all 	combinations
	- Use variants for material params
	- Improve Material Parameters/Pipeline Uniforms/Instance Uniforms are handled
	- Fix light rotation/direction bugs
	- Better handling incase assets folder is not found?
	- Write entity to/from file
	- Ogg format loading and playback
	- Sound streaming
	- Implment missing sound source properties (inner/outer cone, getting sound source data)
	- Ingame console and console commands etc
	- Allow binding/unbinding input maps to functions at runtime, for example if input map 	"Recompute" is triggered, it would call some function that can recompute bounding spheres.
	- Better handling of wav format checking at load time
	- Sprite sheet animations
	- Ray picking
	- Shadow maps
	- Print processor stats and machine capabilites RAM etc on every run to log.
	- Milestone: Pong!
	  - In order to put things into perspective and get a feel for what really needs to be 	prioritized, a very small but actual game release is necessary.
	  - Release platforms: Windows and Linux
	  - Makefile additions. Try to compile game as a dynamically loaded library with ability to 	reload on recompile
	  - Separation between game and engine base
	  ? Game .so with init, update and cleanup functions
	  x Configuration files and "cvars" load/reload
	  x Keybindings in config
	  x Log output on every run.
	  - Implement entity load/save to file
	  ? Prefab load/save to file
	- Do input maps really need to be queried by their string names?
	- Reloading all the things! (textures/shaders/models/settings/entities etc)
	- Separate Debug/Editor camera from the active camera in the scene that can be switched to at 	any time
	- Make logging to file and console toggleable at complie-time or run-time
	- Add default keybindings
	- Write default config/keybindings etc to file if none are found in preferences dir
	- Flatpak packaging for linux releases
	- Use hashmap for debugvar slots in editor
	- Use hashmap to store input maps
	- Multisampled textures and framebuffers
	- Validate necessary assets at game launch
	- Gamma correctness
	- Log and debug/stats output in gui
	- Editor automatic window layout adjusting to the current window resolution
	- Event Subsystem
	- Keybindings for gui?
	- Textual/Binary format for data serialization and persistance
	- Better logging
	- Hatching/Ink rendering style
	- Array based string type comptible with cstring(char*)
	- Separate game, engine and assets into different repositories. Combine as sub-repositories
	? Positive and negative values for input_maps and returning corresponding values when they are 	true
	? CANCELED Image based lighting?
	? CANCELED Deferred rendering?
	- ???
	- Profit!

- ## Completed

	* Input
	* Shaders
	* Geometry
	* change struct usage
	* change Array implementation
	* resolve vec-types sizes
	* Transform
	* Deltatime
	* Investigate about Exit() and at_exit() functions and whether to use them or not.
	* Fix readme markdown
	* Framebuffer and resolution independent rendering
	* A simpler build system without dependencies
	* Remove dependencies
	* Remove Kazmath dependency
	* Entity
	* Find a permanent solution for build system
	* Textures
	* Camera
	* Test render
	* Fix input lag and other framerate related issues
	* Materials
	* Mesh/Model
	* Add modifiers to input maps to enable combinations for example, c-x, m-k etc
	* Heirarchical Transforms
	* Materials with textures
	* Lights!
	* Fix problems with texture units
	* Fix problems with frustrum culling
	* Gui
	* Fix mouse bugs on windows
	* Configuration/Settings load/save handling
	* Fix mousewheel bugs and gui not responding to mousewheel input
	* Setup cross compilation with mingw or stick to msvc?
	* Toggleable debug drawing for meshes
	* Font selection
	* Font atlas proper cleanup
	* In second refactor pass, use entities everywhere, no need to pass in transform and model 	separately for example since they're both part of the same entity anyway
	* Show SDL dialogbox if we cannot launch at all?
	* Writing back to config file
	* Reading from config file
	* Variant -> String conversion procedure. Use in editor for debug var slots
	* Add strings and booleans to variant types
	* Fix Key release not being reported
	* OpenAL not working in releasebuilds
	* 3d sound using OpenAL
	* Fix frustum culling bugs
	* Array-based Hashmaps
	* Fix bugs with heirarchical transformations
	* Remove reduntant "settings" structures and move all configuration stuff to config variables
	* Log output to file on every run
	* Add option to specify where to read/write files from instead of being hard-coded assets dir
	* Fix input map bugs
	* Live data views in editor
	* Camera resize on window reisze
	* Resizable framebuffers and textures
	* Support for multiple color attachments in framebuffers?
	* Better way to store and manage textures attached to framebuffers
	* Variant type
	* Editor
	* Fix frustum culling sometimes not working
	* Compile and test on windows
	* Fix mouse bugs
	* Fix
	* issues with opengl context showing 2.1 only
	* Improve this readme
	* Replace orgfile with simple text readme and reduce duplication
	* Fix camera acting all weird when right click is held
	* Fix README to conform with markdown syntax
	* Added video driver selection to make game launch under wayland or x11 on linux.
	* Separate game code into a dynamical library that can be reloaded at runtime.
