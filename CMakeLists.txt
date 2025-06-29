cmake_minimum_required(VERSION 3.12)
project(CheddarAndFeta VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_BUILD_TYPE Debug)

if(NOT WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.1 AND NOT WIN32)
  set(CMAKE_C_FLAGS
      "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g -O1")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g -O1")
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
  set(CMAKE_SHARED_LINKER_FLAGS
      "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
endif()

add_subdirectory(libdatachannel)

if(WIN32)
    set(SDL3_DISABLE_WINDOWS_GAMEINPUT TRUE CACHE BOOL "" FORCE)
endif()

add_subdirectory(SDL)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)

set(ENGINE_SRC
    src/engine/bmp_texture.cpp
    src/engine/font.cpp
    src/engine/game_const.cpp
    src/engine/game_draw.cpp
    src/engine/game_sprite.cpp
    src/engine/game_step.cpp
    src/engine/controller.cpp
    src/engine/main.cpp
    src/engine/map.cpp
    src/engine/net_agent.cpp
    src/engine/net_connection.cpp
    src/engine/net_signaller.cpp
    src/engine/save_data.cpp
    src/engine/sprite.cpp
)

set(DEPS LibDataChannel::LibDataChannelStatic SDL3::SDL3)

add_executable(MapEditor
    ${ENGINE_SRC}
    src/map_editor/map_editor.cpp
)
target_include_directories(MapEditor PRIVATE src/engine src/map_editor)
target_link_libraries(MapEditor PRIVATE ${DEPS})

add_executable(FetaLauncher
    ${ENGINE_SRC}
    src/feta/net_receiver.cpp
    src/feta/net_sender.cpp
    src/feta/init.cpp
)
target_include_directories(FetaLauncher PRIVATE src/engine src/feta)
target_link_libraries(FetaLauncher PRIVATE ${DEPS})

add_executable(CheddarLauncher
    ${ENGINE_SRC}
    src/cheddar/foes/bat.cpp
    src/cheddar/foes/bell.cpp
    src/cheddar/foes/bug.cpp
    src/cheddar/foes/foe.cpp
    src/cheddar/foes/foe_dijkstra.cpp
    src/cheddar/foes/frog.cpp
    src/cheddar/foes/gate.cpp
    src/cheddar/foes/spawner.cpp
    src/cheddar/items/fire.cpp
    src/cheddar/items/frog_tongue.cpp
    src/cheddar/items/grub.cpp
    src/cheddar/items/molotov.cpp
    src/cheddar/items/toothpick.cpp
    src/cheddar/items/item.cpp
    src/cheddar/billboard.cpp
    src/cheddar/cheese.cpp
    src/cheddar/init.cpp
    src/cheddar/mouse.cpp
    src/cheddar/net_receiver.cpp
    src/cheddar/net_sender.cpp
    src/cheddar/save_station.cpp
    src/cheddar/textbox.cpp
)
target_include_directories(CheddarLauncher PRIVATE src/engine src/cheddar)
target_link_libraries(CheddarLauncher PRIVATE ${DEPS})
