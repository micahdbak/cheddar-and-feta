#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "colour.h"
#include "map/map.h"

constexpr thoom::Colour kBackground{0x181818ff};

enum EditorSelectedLayer { BACKGROUND, FOREGROUND, COLLISION };
enum EditorInputMode { INPUT_TILE, INPUT_SHEET };

class EditorUI {
 public:
  EditorUI() = default;
  ~EditorUI();

  void init(thoom::Map* map);

  void move(const thoom::Map* map);
  void step(const thoom::Map* map, EditorSelectedLayer layer);
  void render(SDL_Texture* dst, const thoom::Map* map,
              EditorSelectedLayer layer) const;
  void render_input(SDL_Texture* dst, const thoom::Map* map,
                    EditorSelectedLayer layer) const;

  void begin_tile();
  void begin_sheet();

  bool inputting = false;
  EditorInputMode input_mode = INPUT_TILE;
  std::string text;

  int tilesheet = -1;
  int tile_x = 0, tile_y = 0;
  int sheet_x = 0, sheet_y = 0;
  int region_x = -1, region_y = -1;
  int collider = -1;

 private:
  void step_tile(const thoom::Map* map, EditorSelectedLayer layer);
  void step_sheet(const thoom::Map* map);
  void render_tile_input(SDL_Texture* dst, const thoom::Map* map,
                         EditorSelectedLayer layer) const;
  void render_sheet_input(const thoom::Map* map) const;

  SDL_Texture* collision_sheet = nullptr;
  int sheet_cols = 14;
};
