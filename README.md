# Cheddar and Feta

![Cheddar and Feta dancing](steam_assets/dance.gif)

## Introduction

Cheddar and Feta is a 2D RPG and corresponding engine built using [SDL3](https://libsdl.org/) for cross-platform windowing and graphics, and [libdatachannel](https://libdatachannel.org) for WebRTC connectivity.

## Building

To build Cheddar and Feta, ensure that you have CMake installed. With CMake, you can build Cheddar and Feta from Linux, macOS, or Windows using the following commands.

Clone the repository, recursing submodules (SDL3, libdatachannel):

```
git clone https://github.com/micahdbak/cheddar-and-feta --recurse-submodules
```

Enter the cloned repository:

```
cd cheddar-and-feta
```

Configure, and build using CMake according to your system:

### Linux or macOS

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=(Release|Debug)
cmake --build build --config (Release|Debug)
./copy_assets.sh
```

The `CheddarLauncher`, `FetaLauncher`, and `MapEditor` executables should now exist in `build/(Release|Debug)`. Open them from the command line to play as Cheddar, play as Feta, or edit the map files.

### Windows

```
cmake -S . -B build -G "Visual Studio 18 2026" -DCMAKE_BUILD_TYPE=(Release|Debug)
cmake --build build --config (Release|Debug)
./copy_assets.sh
```

The `CheddarLauncher.exe`, `FetaLauncher.exe`, and `MapEditor.exe` executables should now exist in `build/(Release|Debug)`. Open them from the command line to play as Cheddar, play as Feta, or edit the map files.

#### Windows Development

If you are developing from Windows, you should open the built solution file in Visual Studio, to gain access to debugging features.

Additionally, be sure to set the linker SubSystem to Windows instead of Console, to not open a terminal on boot.
This can be set in Visual Studio after opening the solution by going to Project Properties > Configuration Properties > Linker > System > SubSystem, and set the SubSystem to /SUBSYSTEM:WINDOWS.

## Engine Design

The engine source is located in `src/engine`. This section provides an overview of the major subsystems.

### Game Loop

The `main.cpp` file initializes [SDL3](https://libsdl.org/), creates a window and renderer, and runs the main loop. Each frame, it polls input events, routes them to the appropriate `Controller` object, calls `Game::step()`, and renders the screen texture to the window. The `Game` class (primarily `game_step.cpp`) orchestrates all game logic, managing ticks, delta time, map loading, object stepping, and rendering. Rendering is performed to an offscreen `screen` texture (320×240) that is scaled to the window while maintaining aspect ratio. The `Game` class is a singleton which is created in `main.cpp`, and is accessible as `game`.

### Maps

Maps are handled by the `Map` class (`map.h`, `map.cpp`) and are persisted as paired `.txt` and `.bin` files. The `.txt` file stores metadata: map title, description, tile dimensions, grid size, background color, ambience audio path, tilesheet paths, and object spawn definitions. The `.bin` file stores tile and collider data in a compact binary format. When `game->map` is set to a new path, `Game::step()` calls `unload()` to destroy existing objects and textures, then `load_map()` to parse the new map, allocate background/foreground textures, and instantiate objects from the map's object list. Collision data is stored as an integer per tile referencing predefined quad colliders defined in `MapColliders` (`map.h`).

### Tile Rendering

Tile rendering is pre-baked at map load time. The `Map::read()` function loads each tile from the binary format and calls `Map::render_tile()` for every grid cell. This renders all background tiles to a single `bg` texture and all foreground tiles to an `fg` texture using the tilesheets loaded earlier. During each frame, `Game::step()` simply renders the `bg` texture, then sprites, then the `fg` texture.

### Sprite Rendering

Dynamic entities render via `Game::push_sprite()` (`game_sprite.cpp`), which inserts a `SpriteRender` into a sorted vector that preserves the y-order of the calling objects to create visual depth. Each `SpriteRender` holds a texture pointer, source/destination rects, and the y-coordinate for sorting. During rendering, `Game::step()` iterates through the sorted sprites and draws only those intersecting the viewport. The `Sprite` class (`sprite.h`, `sprite.cpp`) provides animated texture support, managing frame timing and spritesheet-based frame selection.

### Object Steps

Game objects inherit from the abstract `Object` class (`object.h`), which requires implementing a virtual `step()` function. Objects are instantiated through the `ObjectFactory` pattern. Each object type registers a factory in `game->factories`. During map loading or runtime via `game->push_object()`, objects are created by ID lookup in `game->factories`. `Game::step()` iterates over all created objects, calling their `step()` functions. Objects can request self-destruction by setting `game->delete_object = true` within their `step()` function, which triggers removal after that object's step is completed. Special `_first` and `_last` object slots allow forcing execution order, useful for player controllers or HUD updates that must run at fixed points in the frame. Only one `_first` object may exist, and only one `_last` object may exist at a time.

### P2P Networking

Multiplayer connectivity uses WebRTC via [libdatachannel](https://libdatachannel.org/), encapsulated in `NetworkAgent`, `NetworkConnection`, and `NetworkSignaller` (`net_agent.(h|cpp)`, `net_connection.(h|cpp)`, `net_signaller.(h|cpp)`). The `NetworkAgent` runs a background thread managing a state machine with states: `NO_CONNECTION`, `WAITING_FOR_PEER`, `CONNECTED`, and `TRY_RESET`. A signalling server relays SDP offers/answers and ICE candidates between peers identified by a 6-character alphanumeric code. Once the WebRTC data channel opens, messages are exchanged via `send_message()` and `next_message()`. Message types are single-character prefixed (e.g., `MSG_SPRITE`, `MSG_SYNC`) and handle game state synchronization such as map changes, sprite positions, item usage, and combat events.

### Controller Input

Input handling is managed by the `Controller` class (`controller.h`, `controller.cpp`). Keyboard events are mapped to abstract `Button` enums (UP, DOWN, LEFT, RIGHT, SELECT, etc.) via configurable keybindings stored in `ktobutton_map` and `btokeycode_map`. The controller tracks both held and "hit" (newly pressed) states, with `is_hit()` returning true only on the frame a button was first pressed. The `capture_controls` flag (`main.cpp`) switches input routing between gameplay and menu contexts.

### Audio Playback

Audio is handled via `audio_playback.(h|cpp)`, supporting sound effects and looping ambient music. The `Game` class maintains an audio message queue. Objects push `AudioMsg` structs with a WAV path and spatial coordinates, which are processed for playback with volume adjustment based on `game->volume` and the distance between the sound's source and the player. Ambient audio tracks are specified in the map metadata file, and is volume adjusted based on `game->music_volume`.

### Save Data

`save_data.(h|cpp)` provides a simple dictionary for save data. Objects can store and retrieve integer, float, or string data.

## Copyright and License

Copyright (c) 2026 Micah Baker

This project is licensed under the terms of the GPL-3.0 license. See the [LICENSE](LICENSE) file for full license details.
