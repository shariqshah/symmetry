===================
Project Symmetry
===================

** What?
A topdown 2D shooter exploring symmetry.

** License
All the code in this repository is under GPLv3, see LICENSE for more information

** File format specifications
   *** Entity
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

   - Add to_string functions for major structs like transform, model etc to ease in conversion?

   *** Configuration Variables a.k.a cfg-vars
       # Comment
	   render_width: 1024
	   render_height: 1024
	   debug_draw_enabled: true
	   fog_color: 0.5 0.2 0.2 1
	   # There can be comments or empty newlines in between unlike entity definitions

       ambient_light: 0.1 0.1 0.1 1
	   msaa: true
	   msaa_levels: 8

   *** Keybindings
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

   *** Level/Scene
       - Binary format with header attached at the top
	   - Save child entities first
	   - Copy paste all entites in the file one by one. Since the entites all look
	     the same in memory and are made up of tagged unions, a simple memcpy approach
		 should suffice. The problem is entity heirarchies. There are multiple approaches to
		 solve this problem.
		 -- Save a sorted list of entites to file i.e. before saving create a new list that does
		    not have the empty array slots in the entity list and then just copy and paste. This
			is the simplest way to solve the problem as we don't have to worry about indexes of
			parent/child entites in heirarchy. We can take the whole array and paste it to the
			file but creating a copy of entity list for this purpose only would be slow and consume a lot of memory.
			-- Instead of creating a copy of the entity list for sorting and saving, sort the actual entity list
			and update all references as necessary then save the array to file.
			-- Just write the name of the parent entity as parent. Make sure that all entity names are unique.
   *** Materials
   *** Mesh/Geometry
** Notes on entity Systems
   - Fat entites with all related properties, i.e. position, mesh etc in them. Easy to serialize, memory friendly, simple to implement
     but would require significant changes to the current codebase. e.g.
	 struct Entity
	 {
	     int   type;
		 char* name;
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
   - Change component implementation by using anonymous unions to simulate interfaces. e.g
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
   - Use handles for assets
   - Use something similar to Variant to use as entity, not sure what or how
   - Don't forget to think of the actual use-case and usage when coming up with a solution, don't build castles in the air!


================
Things TODO
================

x Input
x Shaders
x Geometry
x change struct usage
x change Array implementation
x resolve vec-types sizes
x Transform
x Deltatime
x Investigate about Exit() and at_exit() functions and whether to use them or not.
x Fix readme markdown
x Framebuffer and resolution independent rendering
x A simpler build system without dependencies
x Remove dependencies
x Remove Kazmath dependency
x Entity
x Find a permanent solution for build system
? Positive and negative values for input_maps and returning corresponding values when they are true
x Textures
x Camera
x Test render
x Fix input lag and other framerate related issues
x Materials
x Mesh/Model
x Add modifiers to input maps to enable combinations for example, c-x, m-k etc
x Heirarchical Transforms
x Materials with textures
x Lights!
x Fix problems with texture units
x Fix problems with frustrum culling
x Gui
x Fix mouse bugs on windows
x Configuration/Settings load/save handling
x Fix mousewheel bugs and gui not responding to mousewheel input
x Setup cross compilation with mingw or stick to msvc?
x Toggleable debug drawing for meshes
x Font selection
x Font atlas proper cleanup
x In second refactor pass, use entities everywhere, no need to pass in transform and model separately for example since they're both part of the same entity anyway
x Show SDL dialogbox if we cannot launch at all?
x Writing back to config file
x Reading from config file
x Variant -> String conversion procedure. Use in editor for debug var slots
x Add strings and booleans to variant types
x Fix Key release not being reported
x OpenAL not working in releasebuilds
x 3d sound using OpenAL
x Fix frustum culling bugs
x Array-based Hashmaps
x Fix bugs with heirarchical transformations
x Remove reduntant "settings" structures and move all configuration stuff to config variables
x Log output to file on every run
x Add option to specify where to read/write files from instead of being hard-coded assets dir
x Fix input map bugs
x Live data views in editor
x Camera resize on window reisze
x Resizable framebuffers and textures
x Support for multiple color attachments in framebuffers?
x Better way to store and manage textures attached to framebuffers
x Variant type
x Editor
x Fix frustum culling sometimes not working
x Compile and test on windows
x Fix mouse bugs
x Fix
x issues with opengl context showing 2.1 only
x Improve this readme
x Replace orgfile with simple text readme and reduce duplication?
- Bounding Boxes
  ? Recalculated bounding boxes for rotated meshes?
- File extension checking for asset loading
- Only allocate hashmap bucket when required
- Mapping actions to keybindings, for example map action "Jump" to Space key etc
- Ability to mark meshes for debug rendering with possibility of different color for each?
- Switch to completely static allocation of entites i.e. have a static array of MAX_ENTITIES size. This way we can store pointers to entites and they'll still be in an array and fast to process.
? CANCELED Image based lighting?
? CANCELED Deferred rendering?
- Add marking or queuing up custom meshes for debug render with particular transform and color for rendering bounding spheres for example
- Interleaved vbos for meshes and changes to blender exporter accordingly
- Enumerate and save all the uniform and attribute positions in shader when it is added and cache them in shader object?
- Physics/Collision detection in 2d
- Complete gui integration
- Decoupled event handling of gui and input if possible
- Custom rendering for gui
- Allow passsing base path as commandline argument?
- Remove components and switch to "Fat Entities" i.e. one entity struct contains all combinations
- Use variants for material params
- Improve Material Parameters/Pipeline Uniforms/Instance Uniforms are handled
- Fix light rotation/direction bugs
- Better handling incase assets folder is not found?
- Write entity to/from file
- Ogg format loading and playback
- Stick with OpenAL or switch to SoLoud + SDL for sound?
- Sound streaming
- Implment missing sound source properties (inner/outer cone, getting sound source data)
- Ingame console and console commands etc
- Allow binding/unbinding input maps to functions at runtime, for example if input map "Recompute" is triggered, it would call some function that can recompute bounding spheres.
- Better handling of wav format checking at load time
- Sprite sheet animations
- First class 2d rendering
  - Sprite batching (XNA like)
  - Font rendering(2d/3d) with stb_ttf or freetype
  ? Minimal custom UI for in-game usage that can be rendered to a texture or modify nuklear for that?
- Ray picking
- Shadow maps
- Print processor stats and machine capabilites RAM etc on every run to log.
- Milestone: Pong!
  - In order to put things into perspective and get a feel for what really needs to be prioritized, a very small but actual game release is necessary.
  - Release platforms: Windows and Linux
  - Makefile additions. Try to compile game as a dynamically loaded library with ability to reload on recompile
  - Separation between game and engine base
  ? Game .so with init, update and cleanup functions
  x Configuration files and "cvars" load/reload
  x Keybindings in config
  x Log output on every run.
  - Implement entity load/save to file
  ? Prefab load/save to file
- Do input maps really need to be queried by their string names?
- Reloading all the things! (textures/shaders/models/settings/entities etc)
- Separate Debug/Editor camera from the active camera in the scene that can be switched to at any time
- Make logging to file and console toggleable at complie-time or run-time
- Add default keybindings
- Write default config/keybindings etc to file if none are found in preferences dir
- Wrap malloc and free calls in custom functions to track usage
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
- ???
- Profit!
