cmake_minimum_required(VERSION 3.21)
project(CheddarAndFeta VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
  endif()
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
      Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT WIN32)
  add_library(sanitizers INTERFACE)
  target_compile_options(sanitizers INTERFACE
      $<$<CONFIG:Debug>:-fsanitize=address>
      $<$<CONFIG:Debug>:-fno-omit-frame-pointer>)
  target_link_options(sanitizers INTERFACE
      $<$<CONFIG:Debug>:-fsanitize=address>)
endif()

add_subdirectory(thoom)

add_subdirectory(cheddar)
add_subdirectory(feta)
add_subdirectory(map_editor)
