cmake_minimum_required(VERSION 3.2)

project(Symmetry)

include_directories(include)
file(GLOB SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE) 
else()
  if(CMAKE_BUILD_TYPE MATCHES Debug)
	set(SYMMETRY_BUILD_TYPE debug)
	add_definitions(-DGL_DEBUG_CONTEXT)
  else()
	set(CMAKE_BUILD_TYPE Release)
	set(SYMMETRY_BUILD_TYPE release)
  endif()
endif()


# Flags
add_definitions(-DUSE_GLAD)

# Platform specific libs and flags
if(WIN32)
  set(CURRENT_PLATFORM windows)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_library(sdl2 STATIC IMPORTED)
  set_target_properties(sdl2 PROPERTIES IMPORTED_LOCATION_DEBUG ${CMAKE_SOURCE_DIR}/libs/debug/win64_msvc/SDL2.lib)
  set_target_properties(sdl2 PROPERTIES IMPORTED_LOCATION_RELEASE ${CMAKE_SOURCE_DIR}/libs/release/win64_msvc/SDL2.lib)
  target_link_libraries(${PROJECT_NAME} sdl2 winmm.lib imm32.lib version.lib)
  set(CMAKE_C_FLAGS_DEBUG /MTd)
  set(CMAKE_C_FLAGS_RELEASE /MT)
elseif(UNIX AND NOT APPLE)
  set(CURRENT_PLATFORM linux)
  find_library(LIB_M m REQUIRED)
  find_library(LIB_RT rt REQUIRED)
  find_library(LIB_PTHREAD pthread REQUIRED)
  find_library(SDL2_LIBRARY SDL2 HINTS libs/${SYMMETRY_BUILD_TYPE}/linux64_gcc REQUIRED)
  target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} ${LIB_PTHREAD} ${LIB_M} ${LIB_RT} ${CMAKE_DL_LIBS})
  set(CMAKE_C_FLAGS "-Wall -Wno-undefined -std=c99")
  #after build is complete create a link to assets folder
  add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets)
else()
  message(FATAL_ERROR "Unsupported Platform! Currently only Windows and Linux supported")
endif(WIN32)

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin) # Does not work on windows for some reason! TODO: Find a way around this for faster windows installation
install(TARGETS ${PROJECT_NAME} DESTINATION ${CURRENT_PLATFORM})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CURRENT_PLATFORM})
