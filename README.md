# Project Symmetry

## About

A simple first person shooter that may or may not have anything to do with the concept of symmetry.
The game has a similar struct to older games like Quake where the objective is usually to survive and get to end of the level while killing monsters/demons.
The purpose of this project is to serve as an exercise in creating a game from the ground up using as few libraries as possible. The game uses the following 
libraries:

| Library                                      | Usage                                   |
| ---------------------------------------------| --------------------------------------- |
| [SDL2](https://www.libsdl.org/)              | Windowing, Input, Platform abstraction  |
| [Soloud](http://sol.gfxile.net/soloud/)      | 3d positional audio                     |
| [ODE](http://ode.org/)                       | Physics                                 |
| [Nuklear](https://github.com/vurtun/nuklear) | In-game and editor UI                   |
| [GLAD](https://github.com/dav1dde/glad-web)  | OpenGL Extension Loading                |

## Building

The game uses the [GENie](https://github.com/bkaradzic/GENie) build system. The game can be build by llowing steps:

-**Windows**: Execute the following command in the project's root directory by opening a visual studio veloper command prompt:

  ```shell
  cd build
  ..\tools\genie.exe vs2017
  ```

This will generate a visual studio 2017 solution in the *build/vs2017* folder which can be opened in sual studio and built and run as ususal.

-**Linux(Ubuntu)**: Execute the following in the project's root directory

  ```bash
  cd build
  ../tools/genie gmake
  ```

  This will generate makefiles in the *build/gmake* directory. Then,

  ```bash
  cd gmake
  make all
  ```

  This will build the debug configuration by default and it's output will be in *build/gmake/debug* folder. You can then run the game by,

  ```bash
  cd debug
  ./Symmetry
  ```

## License

All the code in this repository is under GPLv3, see LICENSE for more information

## File format specifications

- ### Entity

  ```bash
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

- ### Configuration Variables a.k.a cfg-vars

  ```bash
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

  ```bash
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
      file but creating a copy of entity list for this purpose only would be slow and consume a lot of memory.
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
      scene file, we save all the mutable	state in the savegame files. When restoring game's state from a save name we will need
      to handle loading of a scene and then applying the mutable state to entites after loading.
    - Entities can have (a fixed number of?) properties. Each property has a name and a corresponding
      variant value like, health or ammo etc. But, how to save/load all of that?

- ### Materials

  *TODO*

- ### Mesh/Geometry

  *TODO*
