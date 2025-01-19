#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <cstdlib>
#include <map>
#include <queue>
#include <vector>

#include <SDL3/SDL.h>

#include "map.h"
#include "object.h"

#define clamp(_x, _min, _max) ((_x) < (_min) ? (_min) : ((_x) > (_max) ? (_max) : (_x)))
#define min(_a, _b) ((_b) < (_a) ? (_b) : (_a))
#define max(_a, _b) ((_b) > (_a) ? (_b) : (_a))

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

struct Text {
    std::string str;
    int x, y;
};

class Game {
public:
    Game();
    ~Game();

    void make_map_rect(int x, int y, int w, int h, SDL_FRect *src_rect, SDL_FRect *dst_rect) const;
    bool point_in_collider(float x, float y) const;

    void init();
    void load_map(const char *map_path);
    void create_object(const std::string &id, const std::string &options);
    void draw_text(const std::string &str, int x, int y);

    void step();
    void unload();

    int view_x = 128, view_y = 160;

    int argc;
    const char **argv;

    std::string title = "SDL3 Game";
    SDL_Texture *screen;
    float delta;

private:
    Uint64 last_ticks;

    SDL_Texture *bg = nullptr, *fg = nullptr;
    int *collision = nullptr;
    int tile_width = 32, tile_height = 32, cols = 1, rows = 1;
    Quad colliders[n_MapColliders];

    std::vector<Object *> objects;
    std::unordered_map<std::string, ObjectFactory *> factories;

    SDL_Texture *mono_font;
    std::queue<Text> texts;
};

extern SDL_Renderer *renderer;
extern Game *game;
extern bool _running;

#endif
