cmake_minimum_required(VERSION 3.12)
project(CheddarAndFeta VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libdatachannel)
add_subdirectory(SDL)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)

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
    src/feta/init.cpp
)
target_include_directories(FetaLauncher PRIVATE src/engine src/feta)
target_link_libraries(FetaLauncher PRIVATE ${DEPS})

add_executable(CheddarLauncher
    ${ENGINE_SRC}
    src/cheddar/cheese.cpp
    src/cheddar/enemy_bug.cpp
    src/cheddar/enemy.cpp
    src/cheddar/init.cpp
    src/cheddar/item_toothpick.cpp
    src/cheddar/item.cpp
    src/cheddar/mouse.cpp
    src/cheddar/save_station.cpp
    src/cheddar/textbox.cpp
)
target_include_directories(CheddarLauncher PRIVATE src/engine src/cheddar)
target_link_libraries(CheddarLauncher PRIVATE ${DEPS})
