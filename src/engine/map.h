#ifndef MAP_H
#define MAP_H

#include <string>
#include <vector>

#include <SDL3/SDL.h>

class Tilesheet {
public:
    Tilesheet(const char *tilesheet_path, int tile_width, int tile_height);
    ~Tilesheet();

    std::string path;
    SDL_Texture *texture;
    int cols, rows;
};

struct Tile {
    unsigned int tilesheet, x, y;
};

const struct ColliderPoint { float x, y; } MapColliders[][4] = {
    { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f } }, // full square

    { { 0.0f, 0.0f }, { 0.5f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } }, // NE
    { { 1.0f, 0.0f }, { 1.0f, 0.5f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } }, // SE
    { { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.5f, 1.0f }, { 0.0f, 1.0f } }, // SW
    { { 0.0f, 0.0f }, { 0.5f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f } }, // NW

    { { 0.0f, 0.0f }, { 0.5f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.5f } }, // NE 1/2A
    { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.5f } }, // NE 1/2B
    { { 1.0f, 0.5f }, { 1.0f, 1.0f }, { 0.5f, 1.0f }, { 0.0f, 1.0f } }, // SE 1/2A
    { { 0.0f, 0.5f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } }, // SE 1/2B
    { { 0.0f, 0.0f }, { 1.0f, 0.5f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } }, // SW 1/2B
    { { 0.0f, 0.5f }, { 1.0f, 1.0f }, { 0.5f, 1.0f }, { 0.0f, 1.0f } }, // SW 1/2A
    { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.5f }, { 0.0f, 1.0f } }, // NW 1/2B
    { { 0.0f, 0.0f }, { 0.5f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 0.5f } }, // NW 1/2A
};

const size_t n_MapColliders = sizeof(MapColliders) / sizeof(MapColliders[0]);

struct Quad {
    SDL_FPoint vertex[4];
};

class Map {
public:
    ~Map();

    void make_empty(int tile_width, int tile_height, int cols, int rows);
    void read(const char *map_path);
    void write(const char *map_path);
    void clear();

    void render_tile(int x, int y);

    // handled by Map
    std::string title = "Unnamed map", description = "Floor 1";
    int tile_width, tile_height, cols, rows;
    std::vector<Tilesheet *> tilesheets;
    std::vector<std::pair<std::string, std::string>> objects;
    std::vector<Tile> **bg_tiles = nullptr;
    std::vector<Tile> **fg_tiles = nullptr;

    // must be retrieved and released by caller
    SDL_Texture *bg, *fg; // free with `SDL_DestroyTexture`
    int *collision; // free with `free`
};

#endif
