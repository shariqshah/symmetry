cmake_minimum_required(VERSION 3.2)

project(Symmetry)


if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE) 
else()
  if(CMAKE_BUILD_TYPE MATCHES Debug)
	set(SYMMETRY_BUILD_TYPE debug)
  else()
	set(CMAKE_BUILD_TYPE Release)
	set(SYMMETRY_BUILD_TYPE release)
  endif()
endif()

################################################################
# Setup external libs
################################################################

add_library(SDL2 SHARED IMPORTED)
add_library(SOLOUD SHARED IMPORTED)
add_library(ODE SHARED IMPORTED)

if(WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/include/common
		${CMAKE_SOURCE_DIR}/include/common/soloud
		${CMAKE_SOURCE_DIR}/include/windows/sdl2
		${CMAKE_SOURCE_DIR}/include/windows)

	set(SDL2_LIB_FILE ${CMAKE_SOURCE_DIR}/lib/windows/sdl2/SDL2.dll)
	set(SOLOUD_LIB_FILE ${CMAKE_SOURCE_DIR}/lib/windows/soloud/soloud_x64.dll)
	set(ODE_LIB_FILE ${CMAKE_SOURCE_DIR}/lib/windows/ode/ode_double.dll)

	set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${SDL2_LIB_FILE})
	set_target_properties(SDL2 PROPERTIES IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/lib/windows/sdl2/SDL2.lib)

	set_target_properties(SOLOUD PROPERTIES IMPORTED_LOCATION ${SOLOUD_LIB_FILE})
	set_target_properties(SOLOUD PROPERTIES IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/lib/windows/soloud/soloud_x64.lib)

	set_target_properties(ODE PROPERTIES IMPORTED_LOCATION ${ODE_LIB_FILE})
	set_target_properties(ODE PROPERTIES IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/lib/windows/ode/ode_double.lib)

	set(SDL2_LIB_FILES ${SDL2_LIB_FILE})
	set(ODE_LIB_FILES ${ODE_LIB_FILE})
	set(SOLOUD_LIB_FILES ${SOLOUD_LIB_FILE})
	
elseif(UNIX AND NOT APPLE)
	include_directories(${CMAKE_SOURCE_DIR}/include/common
		${CMAKE_SOURCE_DIR}/include/common/soloud
		${CMAKE_SOURCE_DIR}/include/linux/sdl2
		${CMAKE_SOURCE_DIR}/include/linux)

  set(SDL2_LIB_FILE ${CMAKE_SOURCE_DIR}/lib/linux/sdl2/libSDL2.so)
  set(SOLOUD_LIB_FILE ${CMAKE_SOURCE_DIR}/lib/linux/soloud/libsoloud_x64.so)
  set(ODE_LIB_FILE ${CMAKE_SOURCE_DIR}/lib/linux/ode/libode.so)
	
  set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${SDL2_LIB_FILE})
  set_target_properties(SOLOUD PROPERTIES IMPORTED_LOCATION ${SOLOUD_LIB_FILE})
  set_target_properties(ODE PROPERTIES IMPORTED_LOCATION ${ODE_LIB_FILE})
	
	file(GLOB SDL2_LIB_FILES "${CMAKE_SOURCE_DIR}/lib/linux/sdl2/*")
	file(GLOB ODE_LIB_FILES "${CMAKE_SOURCE_DIR}/lib/linux/ode/*")
	file(GLOB SOLOUD_LIB_FILES "${CMAKE_SOURCE_DIR}/lib/linux/soloud/*")
	
endif(WIN32)



################################################################
# Setup source files
################################################################

set(COMMON_HEADERS
  src/common/array.h
  src/common/hashmap.h
  src/common/linmath.h
  src/common/log.h
  src/common/parser.h
  src/common/string_utils.h
  src/common/utils.h
  src/common/variant.h
  src/common/array_str.h
  src/common/common.h
  src/common/num_types.h)

set(COMMON_SOURCES
  src/common/array.c
  src/common/hashmap.c
  src/common/linmath.c
  src/common/log.c
  src/common/parser.c
  src/common/string_utils.c
  src/common/utils.c
  src/common/variant.c)

set(EXECUTALE_HEADERS
  src/game/main.h
  src/game/config_vars.h
  src/game/file_io.h
  src/game/physics.h
  src/game/platform.h
  src/game/sound.h)

set(EXECUTABLE_SOURCES
  src/game/main.c
  src/game/config_vars.c
  src/game/file_io.c
  src/game/physics.c
  src/game/platform.c
  src/game/sound.c)


set(GAME_LIB_SOURCES
  src/libsymmetry/bounding_volumes.c
  src/libsymmetry/camera.c
  src/libsymmetry/editor.c
  src/libsymmetry/console.c
  src/libsymmetry/editor.c
  src/libsymmetry/entity.c
  src/libsymmetry/framebuffer.c
  src/libsymmetry/game.c
  src/libsymmetry/geometry.c
  src/libsymmetry/glad.c
  src/libsymmetry/gl_load.c
  src/libsymmetry/gui.c
  src/libsymmetry/im_render.c
  src/libsymmetry/input.c
  src/libsymmetry/light.c
  src/libsymmetry/material.c
  src/libsymmetry/model.c
  src/libsymmetry/player.c
  src/libsymmetry/renderer.c
  src/libsymmetry/scene.c
  src/libsymmetry/shader.c
  src/libsymmetry/sprite.c
  src/libsymmetry/texture.c
  src/libsymmetry/transform.c)


set(GAME_LIB_HEADERS
  src/libsymmetry/bounding_volumes.h
  src/libsymmetry/camera.h
  src/libsymmetry/editor.h
  src/libsymmetry/console.h
  src/libsymmetry/editor.h
  src/libsymmetry/entity.h
  src/libsymmetry/event.h
  src/libsymmetry/framebuffer.h
  src/libsymmetry/game.h
  src/libsymmetry/geometry.h
  src/libsymmetry/gl_load.h
  src/libsymmetry/gui.h
  src/libsymmetry/im_render.h
  src/libsymmetry/input.h
  src/libsymmetry/light.h
  src/libsymmetry/material.h
  src/libsymmetry/model.h
  src/libsymmetry/player.h
  src/libsymmetry/renderer.h
  src/libsymmetry/scene.h
  src/libsymmetry/shader.h
  src/libsymmetry/sprite.h
  src/libsymmetry/texture.h
  src/libsymmetry/transform.h)


################################################################
# Game Executable
################################################################

set(EXECUTABLE_NAME ${PROJECT_NAME})

add_executable(${EXECUTABLE_NAME} ${EXECUTABLE_HEADERS} ${EXECUTABLE_SOURCES} ${COMMON_HEADERS} ${COMMON_SOURCES})
target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(${EXECUTABLE_NAME} PUBLIC -DGAME $<$<CONFIG:DEBUG>: -DGL_DEBUG_CONTEXT>)
target_link_libraries(${EXECUTABLE_NAME} SDL2 ODE SOLOUD)

if(WIN32)
	target_compile_definitions(${EXECUTABLE_NAME} PUBLIC -D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)

	set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "'-Wl,-rpath,$ORIGIN/lib' -Wl,--no-undefined")
	target_compile_options(${EXECUTABLE_NAME} PUBLIC -Wall --std=c99)

	find_library(LIB_M m REQUIRED)
	find_library(LIB_RT rt REQUIRED)
	find_library(LIB_PTHREAD pthread REQUIRED)
	target_link_libraries(${EXECUTABLE_NAME} ${LIB_M} ${LIB_RT} ${LIB_PTHREAD})
endif(WIN32)

################################################################
# Game Library
################################################################

set(GAME_LIB_NAME "${PROJECT_NAME}_Game")
add_library(${GAME_LIB_NAME} SHARED ${GAME_LIB_HEADERS} ${GAME_LIB_SOURCES} ${COMMON_HEADERS} ${COMMON_SOURCES})
target_include_directories(${GAME_LIB_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(${GAME_LIB_NAME} PUBLIC -DGAME_LIB -DUSE_GLAD $<$<CONFIG:DEBUG>: -DGL_DEBUG_CONTEXT>)

if(WIN32)
	target_compile_definitions(${GAME_LIB_NAME} PUBLIC -D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
	target_compile_options(${GAME_LIB_NAME} PUBLIC -Wall --std=c99)
endif(WIN32)


################################################################
# Post build
################################################################

message(STATUS " MESSAGE ${RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS " CMAKE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}")
message(STATUS " RESUME HERE!!!! ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")


add_custom_command(
		TARGET ${EXECUTABLE_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/assets/ ${CMAKE_BINARY_DIR}/assets)
		
if(WIN32)
	add_custom_command(
	TARGET ${EXECUTABLE_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SDL2_LIB_FILES} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ODE_LIB_FILES} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SOLOUD_LIB_FILES} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}
	COMMENT "Copying libraries...")
elseif(UNIX AND NOT APPLE)
	add_custom_command(
		TARGET ${EXECUTABLE_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/lib/
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SDL2_LIB_FILES} ${CMAKE_BINARY_DIR}/lib/
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ODE_LIB_FILES} ${CMAKE_BINARY_DIR}/lib/
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SOLOUD_LIB_FILES} ${CMAKE_BINARY_DIR}/lib/
		COMMENT "Copying libraries...")
endif()


################################################################
# Install  
################################################################

Install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
Install(TARGETS ${GAME_LIB_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_INSTALL_PREFIX})
install(
  FILES ${SDL2_LIB_FILES} ${ODE_LIB_FILES} ${SOLOUD_LIB_FILES}
  DESTINATION lib)
