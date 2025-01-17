#ifndef MAP_H
#define MAP_H

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

class Map {
public:
    Map() {}
    ~Map();

    void make_empty(int tile_width, int tile_height, int cols, int rows);
    void read(const char *map_path);
    void write(const char *map_path);
    void clear();

    void render_tile(int x, int y);

    int tile_width, tile_height, cols, rows;
    std::vector<Tilesheet *> tilesheets;
    std::vector<Tile> **bg_tiles;
    std::vector<Tile> **fg_tiles;
    std::vector<std::pair<std::string, std::string>> objects;

    SDL_Texture *bg, *fg;
};

#endif
