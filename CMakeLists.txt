cmake_minimum_required(VERSION 3.12)
project(CheddarAndFeta VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

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

set(MAP_EDITOR_SRC
    src/map_editor/map_editor.cpp
)

set(FETA_SRC
    src/feta/net_receiver.cpp
    src/feta/net_sender.cpp
    src/feta/init.cpp
)

set(CHEDDAR_SRC
    src/cheddar/billboard.cpp
    src/cheddar/foes/bat.cpp
    src/cheddar/foes/bell.cpp
    src/cheddar/foes/bug.cpp
    src/cheddar/foes/foe.cpp
    src/cheddar/foes/foe_dijkstra.cpp
    src/cheddar/foes/frog.cpp
    src/cheddar/foes/gate.cpp
    src/cheddar/foes/porcupine.cpp
    src/cheddar/foes/spawner.cpp
    src/cheddar/foes/spitter/abdomen.cpp
    src/cheddar/foes/spitter/head.cpp
    src/cheddar/foes/spitter/spitter.cpp
    src/cheddar/foes/spitter/thorax.cpp
    src/cheddar/hitbox.cpp
    src/cheddar/hurtbox.cpp
    src/cheddar/init.cpp
    src/cheddar/items/cannon_ball.cpp
    src/cheddar/items/cheese.cpp
    src/cheddar/items/fire.cpp
    src/cheddar/items/frog_tongue.cpp
    src/cheddar/items/molotov.cpp
    src/cheddar/items/persister.cpp
    src/cheddar/items/toothpick.cpp
    src/cheddar/items/tossed.cpp
    src/cheddar/items/item.cpp
    src/cheddar/ladder.cpp
    src/cheddar/mouse.cpp
    src/cheddar/net_receiver.cpp
    src/cheddar/net_sender.cpp
    src/cheddar/textbox.cpp
)

# configure targets with source files
add_executable(MapEditor ${ENGINE_SRC} ${MAP_EDITOR_SRC})
add_executable(FetaLauncher ${ENGINE_SRC} ${FETA_SRC})
add_executable(CheddarLauncher ${ENGINE_SRC} ${CHEDDAR_SRC})

# configure target include directories
target_include_directories(MapEditor PRIVATE src/engine src/map_editor)
target_include_directories(FetaLauncher PRIVATE src/engine src/feta)
target_include_directories(CheddarLauncher PRIVATE src/engine src/cheddar)

# build options for macOS (NOT WIN32) or Windows (WIN32)
if(NOT WIN32)
    set(BUILD_SHARED_LIBS OFF)

    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug")
    endif()

    # debugging stuff
    #set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g -O1")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g -O1")
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    #set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")

    # output build executables into build/(Debug|Release)/
    set_target_properties(CheddarLauncher FetaLauncher MapEditor
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/"
    )

    # prep dependencies
    add_subdirectory(libdatachannel EXCLUDE_FROM_ALL)
    add_subdirectory(SDL EXCLUDE_FROM_ALL)
    set(DEPS LibDataChannel::LibDataChannelStatic SDL3::SDL3)
else()
    set(BUILD_SHARED_LIBS ON)

    # VS generator generates both Debug and Release
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)

    # hides some dumb warnings about sscanf, ...
    target_compile_definitions(MapEditor PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(FetaLauncher PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(CheddarLauncher PRIVATE _CRT_SECURE_NO_WARNINGS)

    # prep dependencies
    set(SDL3_DISABLE_WINDOWS_GAMEINPUT TRUE CACHE BOOL "" FORCE)
    add_subdirectory(libdatachannel EXCLUDE_FROM_ALL)
    add_subdirectory(SDL EXCLUDE_FROM_ALL)
    set(DEPS LibDataChannel::LibDataChannel SDL3::SDL3)

    # copy dlls over after compiling CheddarLauncher
    add_custom_command(TARGET CheddarLauncher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:CheddarLauncher>
            $<TARGET_FILE_DIR:CheddarLauncher>
        COMMAND_EXPAND_LISTS
    )
endif()

# link dependencies
target_link_libraries(MapEditor PRIVATE ${DEPS})
target_link_libraries(FetaLauncher PRIVATE ${DEPS})
target_link_libraries(CheddarLauncher PRIVATE ${DEPS})

# Extra debug thingys (hitboxes, hurtboxes)
#target_compile_definitions(CheddarLauncher PRIVATE ONSCREEN_DEBUG)
