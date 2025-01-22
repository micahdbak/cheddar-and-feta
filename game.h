#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <cstdlib>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "font.h"
#include "map.h"
#include "object.h"

#define clamp(_x, _min, _max) ((_x) < (_min) ? (_min) : ((_x) > (_max) ? (_max) : (_x)))
#define min(_a, _b) ((_b) < (_a) ? (_b) : (_a))
#define max(_a, _b) ((_b) > (_a) ? (_b) : (_a))

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define MONO_FONT  0
#define SMALL_FONT 1
#define NUM_FONTS  2

struct SpriteRender {
    SDL_Texture *texture;
    SDL_FRect *src_rect, *dst_rect;
    int y; // for depth calculations

    bool operator<(const SpriteRender &other) const {
        return this->y < other.y;
    }
};

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

    // sprite related
    void set_view(int x, int y);
    void push_sprite(SDL_Texture *texture, SDL_FRect *src_rect, SDL_FRect *dst_rect, int depth_offset);

    // ui related
    void draw_rect(SDL_FRect *rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_BlendMode blend_mode);
    void draw_text(const std::string &str, int font, int x, int y);

    void step();
    void unload();

    int view_x = SCREEN_WIDTH/2, view_y = SCREEN_HEIGHT/2;
    int corner_x = 0, corner_y = 0;

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

    std::vector<SpriteRender> sprites;

    std::vector<Font *> fonts;
    SDL_Texture *ui = nullptr;
    std::queue<Text> texts;
};

extern SDL_Renderer *renderer;
extern Game *game;
extern bool _running;

#endif
