#ifndef MAP_H
#define MAP_H

#include <SDL3/SDL.h>

#include <string>
#include <vector>

class Tilesheet {
 public:
  Tilesheet(const char* tilesheet_path, int tile_width, int tile_height);
  ~Tilesheet();

  std::string path;
  SDL_Texture* texture;
  int cols, rows;
};

struct Tile {
  unsigned int tilesheet, x, y;
};

const struct ColliderPoint {
  float x, y;
} MapColliders[][4] = {
    {{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}},  // full square

    {{0.0f, 0.0f}, {0.5f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.5f}},  // short tri NE
    {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.5f}, {0.0f, 0.5f}},  // half square N
    {{0.0f, 0.0f},
     {1.0f, 0.0f},
     {1.0f, 1.0f},
     {0.0f, 0.5f}},  // horiz fat tri NE
    {{0.0f, 0.0f}, {0.5f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}},  // tri NW
    {{0.0f, 0.0f}, {0.5f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}},  // tri NE
    {{0.0f, 0.0f},
     {1.0f, 0.0f},
     {1.0f, 0.5f},
     {0.0f, 1.0f}},  // horiz fat tri NW
    {{0.0f, 0.0f}, {0.5f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.5f}},  // short tri NW

    {{1.0f, 0.5f}, {1.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 1.0f}},  // short tri SE
    {{0.0f, 0.5f}, {1.0f, 0.5f}, {1.0f, 1.0f}, {0.0f, 1.0f}},  // half square S
    {{0.0f, 0.5f},
     {1.0f, 0.0f},
     {1.0f, 1.0f},
     {0.0f, 1.0f}},  // horiz fat tri SE
    {{0.0f, 0.0f}, {1.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 1.0f}},  // tri SW
    {{1.0f, 0.0f}, {1.0f, 0.5f}, {1.0f, 1.0f}, {0.0f, 1.0f}},  // tri SE
    {{0.0f, 0.0f},
     {1.0f, 0.5f},
     {1.0f, 1.0f},
     {0.0f, 1.0f}},  // horiz fat tri SW
    {{0.0f, 0.5f}, {1.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 1.0f}},  // short tri SW

    {{1.0f, 0.0f}, {1.0f, 0.5f}, {1.0f, 1.0f}, {0.5f, 1.0f}},  // skinny tri SE
    {{0.5f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.5f, 1.0f}},  // half square E
    {{0.5f, 0.0f},
     {1.0f, 0.0f},
     {1.0f, 1.0f},
     {0.0f, 1.0f}},  // vert fat tri SE
    {{0.0f, 0.0f},
     {1.0f, 0.0f},
     {1.0f, 1.0f},
     {0.5f, 1.0f}},  // vert fat tri NE
    {{0.5f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.5f}, {1.0f, 1.0f}},  // skinny tri NE

    {{0.0f, 0.0f}, {0.5f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.5f}},  // skinny tri SW
    {{0.0f, 0.0f}, {0.5f, 0.0f}, {0.5f, 1.0f}, {0.0f, 1.0f}},  // half square W
    {{0.0f, 0.0f},
     {0.5f, 0.0f},
     {1.0f, 1.0f},
     {0.0f, 1.0f}},  // vert fat tri SW
    {{0.0f, 0.0f},
     {1.0f, 0.0f},
     {0.5f, 1.0f},
     {0.0f, 1.0f}},  // vert fat tri NW
    {{0.0f, 0.0f}, {0.5f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.5f}},  // skinny tri NW

    {{0.0f, 0.0f}, {1.0f, 0.5f}, {1.0f, 1.0f}, {0.0f, 0.5f}},  // NE/SW
    {{1.0f, 0.0f}, {1.0f, 0.5f}, {0.0f, 1.0f}, {0.0f, 0.5f}},  // SE/NW

    {{0.0f, 0.0f}, {0.5f, 0.0f}, {1.0f, 1.0f}, {0.5f, 1.0f}},  // NE/SW
    {{0.5f, 0.0f}, {1.0f, 0.0f}, {0.5f, 1.0f}, {0.0f, 1.0f}},  // SE/NW

    {{0.0f, 0.0f}, {0.5f, 0.0f}, {1.0f, 0.5f}, {1.0f, 1.0f}},  // NE
    {{1.0f, 0.0f}, {1.0f, 0.5f}, {0.5f, 1.0f}, {0.0f, 1.0f}},  // SE
    {{0.0f, 0.0f}, {1.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 0.5f}},  // SW
    {{0.0f, 0.5f}, {0.5f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}},  // NW

    {{0.5f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.1f}, {1.0f, 0.5f}},  // NE
    {{1.0f, 0.5f}, {1.0f, 0.9f}, {1.0f, 1.0f}, {0.5f, 1.0f}},  // SE
    {{0.0f, 0.5f}, {0.5f, 1.0f}, {0.1f, 1.0f}, {0.0f, 1.0f}},  // SW
    {{0.0f, 0.0f}, {0.1f, 0.0f}, {0.5f, 0.0f}, {0.0f, 0.5f}},  // NW

    {{0.0f, 0.0f}, {0.5f, 0.0f}, {0.5f, 0.75f}, {0.0f, 1.0f}},  // special
    {{0.5f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.5f, 0.75f}},  // special
};

const size_t n_MapColliders = sizeof(MapColliders) / sizeof(MapColliders[0]);

struct Quad {
  SDL_FPoint vertex[4];
};

class Map {
 public:
  ~Map();

  void make_empty(int tile_width, int tile_height, int cols, int rows);
  void read(const char* map_path);
  void write(const char* map_path);
  void clear();

  void render_tile(int x, int y);

  // handled by Map
  std::string title = "Unnamed map", description = "Floor 1";
  int tile_width, tile_height, cols, rows;
  int bg_r = 0, bg_g = 0, bg_b = 0;
  std::string ambience = "";
  std::vector<Tilesheet*> tilesheets;
  std::vector<std::pair<std::string, std::string>> objects;
  std::vector<Tile>** bg_tiles = nullptr;
  std::vector<Tile>** fg_tiles = nullptr;

  // must be retrieved and released by caller
  SDL_Texture *bg, *fg;  // free with `SDL_DestroyTexture`
  int* collision;        // free with `free`
};

#endif
