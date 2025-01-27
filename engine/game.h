#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <cmath>
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
#define min(_a, _b)           ((_b) < (_a) ? (_b) : (_a))
#define max(_a, _b)           ((_b) > (_a) ? (_b) : (_a))
#define sign(_x)              ((_x) == 0 ? 0 : ((_x) > 0 ? 1 : -1))

#define distance_between_points(x1, y1, x2, y2) \
    (std::sqrt(std::pow((x2) - (x1), 2) + std::pow((y2) - (y1), 2)))

#define DIAG_MULTIPLIER 0.7071f // for diagonal movement

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define MONO_FONT    0
#define SMALL_FONT   1
#define DEFAULT_FONT 2
#define NUM_FONTS    3

#define TEST_TEXT \
    " !\"#$%&'()*+,-./\n"\
    "0123456789:;<=>?@[\\]^_`|\n"\
    "AaBbCcDdEeFfGgHhIiJjKkLlMm\n"\
    "NnOoPpQqRrSsTtUuVvWwXxYyZz"

#define LOREM_IPSUM \
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"\
    "Donec vehicula venenatis arcu quis blandit.\n"\
    "Suspendisse sagittis risus vitae euismod eleifend.\n"\
    "Quisque mauris felis, scelerisque nec rutrum sit amet, molestie ut quam.\n"\
    "Suspendisse non blandit lorem, quis mollis felis.\n"\
    "Vestibulum vitae ante elementum libero cursus pharetra in non nibh.\n"\
    "Curabitur maximus sem arcu, ac convallis nisl varius vitae.\n"\
    "Fusce quis magna et orci fringilla viverra.\n"\
    "Praesent accumsan leo dictum egestas luctus.\n"\
    "Mauris nibh ligula, porta quis magna id, egestas rutrum eros.\n"\
    "That will be 7$. You have 15% health points. Cheddar & Feta.\n"\
    "Thanks for reading - Cheddar n' Feta!"

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
    bool in_sight(int x0, int y0, int x1, int y1, int *next_x, int *next_y) const; // true if no colliders in way
    void random_target(int x, int y, int *next_x, int *next_y) const; // chooses a random tile to move to, with no collider

    void init();
    void create_object(const std::string &id, const std::string &options);
    void save_objects();
    void post_save_objects();

    // sprite related
    void set_view(int x, int y);
    void push_sprite(SDL_Texture *texture, SDL_FRect *src_rect, SDL_FRect *dst_rect, int depth_offset);

    // ui related
    void draw_rect(SDL_FRect *rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_BlendMode blend_mode);
    void draw_outline(SDL_FRect *rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_BlendMode blend_mode);
    void draw_ui_box(SDL_FRect *rect);
    void draw_text(const std::string &str, int font, int x, int y, int w);

    void step();

    int argc;
    const char **argv;

    float delta = 0;

    std::string title = "SDL3 Game";
    SDL_Texture *screen, *ui;
    std::vector<Font *> fonts;

    Uint64 ticks = 0;

    int view_x = SCREEN_WIDTH/2, view_y = SCREEN_HEIGHT/2;
    int corner_x = 0, corner_y = 0;
    int tile_width = 32, tile_height = 32, cols = 1, rows = 1;

    std::string map = ""; // set this to load a map

    bool delete_object = false; // set to true from an object's step to delete it

private:
    void unload();
    void load_map(const char *map_path);

    Uint64 last_ticks = 0, frame_ticks = 0;
    int frames;

    SDL_Texture *bg = nullptr, *fg = nullptr;
    int *collision = nullptr;
    Quad colliders[n_MapColliders];

    std::vector<Object *> objects;
    std::unordered_map<std::string, ObjectFactory *> factories;

    std::vector<SpriteRender> sprites;

    SDL_Texture *ui_box = nullptr;
    std::queue<Text> texts;
};

extern SDL_Renderer *renderer;
extern Game *game;
extern bool _running;

#endif
