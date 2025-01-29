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

// useful math stuff

#define clamp(_x, _min, _max) ((_x) < (_min) ? (_min) : ((_x) > (_max) ? (_max) : (_x)))
#define min(_a, _b)           ((_b) < (_a) ? (_b) : (_a))
#define max(_a, _b)           ((_b) > (_a) ? (_b) : (_a))
#define sign(_x)              ((_x) == 0 ? 0 : ((_x) > 0 ? 1 : -1))

#define distance_between_points(x1, y1, x2, y2) \
    (std::sqrt(std::pow((x2) - (x1), 2) + std::pow((y2) - (y1), 2)))

// for movement

#define DIAG_MULTIPLIER 0.7071f // for diagonal movement

// screen related constants

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// fonts and text related things

#define MONO_FONT    0
#define SMALL_FONT   1
#define DEFAULT_FONT 2
#define NUM_FONTS    3

// special chars in SMALL_FONT
#define CHAR_EQUIP        '{'
#define CHAR_TEXTBOX_NEXT '|'
#define CHAR_DOWN_ARROW   '}'

// icon related things

#define ICON_SIZE 16

// icons
#define SKULL_AND_BONES_ICON 0

class Game {
public:
    // game_step.cpp

    Game();
    ~Game();

    void create_object(const std::string &id, const std::string &options);
    void push_object(const std::string &id, const std::string &options);
    void save_objects();
    void post_save_objects();
    void step();

    // game_const.cpp

    void make_map_rect(int x, int y, int w, int h, SDL_FRect *src_rect, SDL_FRect *dst_rect) const;
    bool point_in_collider(float x, float y) const;
    bool in_sight(int x0, int y0, int x1, int y1, int *next_x, int *next_y) const; // true if no colliders in way
    void random_target(int x, int y, int *next_x, int *next_y) const; // chooses a random tile to move to, with no collider

    // game_draw.cpp

    void draw_rect(SDL_FRect *rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_BlendMode blend_mode);
    void draw_outline(SDL_FRect *rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_BlendMode blend_mode);
    void draw_ui_box(SDL_FRect *rect);
    void draw_text(const std::string &str, int font, int x, int y, int w);
    void draw_icon(int icon, SDL_FRect *dst_rect);

    // game_sprite.cpp

    void set_view(int x, int y);
    void push_sprite(SDL_Texture *texture, SDL_FRect *src_rect, SDL_FRect *dst_rect, int depth_offset);

    // defined per game (e.g., map_editor.cpp, init.cpp)

    void init();

    int argc;
    const char **argv;

    float delta = 0;

    std::string title = "SDL3 Game";
    SDL_Texture *screen, *ui;
    std::vector<Font *> fonts;

    Uint64 ticks = 0;

    int view_x = SCREEN_WIDTH/2, view_y = SCREEN_HEIGHT/2;
    int corner_x = 0, corner_y = 0; // set in Game::step
    int tile_width = 32, tile_height = 32, cols = 1, rows = 1;

    std::string map = ""; // set this to load a map

    bool delete_object = false; // set to true from an object's step to delete it

private:
    // for Game::sprites
    struct SpriteRender {
        SDL_Texture *texture;
        SDL_FRect *src_rect, *dst_rect;
        int y; // for depth calculations

        bool operator<(const SpriteRender &other) const {
            return this->y < other.y;
        }
    };

    // for Game::texts
    struct Text {
        std::string str;
        int x, y;
    };

    // game_step.cpp

    void unload();
    void load_map(const char *map_path);

    // for calculating delta time and fps
    Uint64 last_ticks = 0, frame_ticks = 0;
    int frames;

    // map things
    SDL_Texture *bg = nullptr, *fg = nullptr;
    int *collision = nullptr;
    Quad colliders[n_MapColliders];

    // object things
    std::vector<Object *> objects;
    std::queue<std::pair<std::string, std::string>> new_objects;
    std::unordered_map<std::string, ObjectFactory *> factories;

    // sprite rendering things
    std::vector<SpriteRender> sprites;

    // ui things
    SDL_Texture *ui_box = nullptr;
    std::queue<Text> texts;
    SDL_Texture *icons;
};

extern SDL_Renderer *renderer;
extern Game *game;
extern bool _running;

#endif
