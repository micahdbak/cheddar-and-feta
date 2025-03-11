cmake_minimum_required(VERSION 3.30)
project(CheddarAndFeta)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(SDL EXCLUDE_FROM_ALL)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)

set(ENGINE_SRC
    engine/bmp_texture.cpp
    engine/font.cpp
    engine/game_const.cpp
    engine/game_draw.cpp
    engine/game_sprite.cpp
    engine/game_step.cpp
    engine/controller.cpp
    engine/main.cpp
    engine/map.cpp
    engine/save_data.cpp
    engine/sprite.cpp
)

add_executable(CheddarAndFeta
    src/cheese.cpp
    src/enemy_bug.cpp
    src/enemy.cpp
    src/init.cpp
    src/inventory.cpp
    src/item_toothpick.cpp
    src/item.cpp
    src/mouse.cpp
    src/save_station.cpp
    src/textbox.cpp
    ${ENGINE_SRC}
)
target_include_directories(CheddarAndFeta PRIVATE engine src resources)
target_link_libraries(CheddarAndFeta PRIVATE SDL3::SDL3)

add_executable(MapEditor
    map_editor/map_editor.cpp
    ${ENGINE_SRC}
)
target_include_directories(MapEditor PRIVATE engine)
target_link_libraries(MapEditor PRIVATE SDL3::SDL3)
