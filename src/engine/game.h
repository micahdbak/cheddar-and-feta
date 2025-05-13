#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <unordered_map>
#include <queue>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "font.h"
#include "map.h"
#include "object.h"

// useful math stuff

#define cnf_clamp(_x, _min, _max) ((_x) < (_min) ? (_min) : ((_x) > (_max) ? (_max) : (_x)))
#define cnf_min(_a, _b)           ((_b) < (_a) ? (_b) : (_a))
#define cnf_sign(_x)              ((_x) == 0 ? 0 : ((_x) > 0 ? 1 : -1))
#define cnf_abs(_x)               ((_x) < 0 ? (-1 * (_x)) : (_x))

#define distance_between_points(x1, y1, x2, y2) \
    (sqrt(pow((x2) - (x1), 2) + pow((y2) - (y1), 2)))

// for movement
#define DIAG_MULTIPLIER 0.7071f // for diagonal movement

// screen related constants
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// ui box related things
#define BOX_CONTAINER 0
#define BOX_OUT       1
#define BOX_OUT_SEL   2
#define BOX_MENU_CONT 3
#define BOX_UNDER     4
#define BOX_UNDER_SEL 5
#define BOX_OVERLAY   6

// icons
#define CHEDDAR_ICON          SDL_FRect{0.0f, 0.0f, 16.0f, 16.0f}
#define NOT_CONNECTED_ICON    SDL_FRect{16.0f, 0.0f, 16.0f, 16.0f}
#define WAITING_FOR_PEER_ICON SDL_FRect{32.0f, 0.0f, 16.0f, 16.0f}
#define FETA_ICON             SDL_FRect{48.0f, 0.0f, 16.0f, 16.0f}
#define SKULL_AND_BONES_ICON  SDL_FRect{0.0f, 16.0f, 16.0f, 16.0f}
#define EDITOR_OBJECT_ICON    SDL_FRect{16.0f, 16.0f, 16.0f, 16.0f}

// forces one object to the be the first/last object run per frame
// should only be used by something like:
// - cheddar/frame_compiler(.h|.cpp)
// - cheddar/remote_controller(.h|.cpp)
#define FIRST_OBJ "_first"
#define LAST_OBJ "_last"

// game_const.cpp

int direction_from_dirs(int x_dir, int y_dir);
void dirs_from_direction(int direction, int *x_dir, int *y_dir);
void dir_to_point(float x1, float y1, float x2, float y2, int *x_dir, int *y_dir);
const char *next_line(const char *arr);

class Game {
public:
    // game_step.cpp

    Game();
    ~Game();

    void unload();
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

    void draw_rect(SDL_Texture *texture, SDL_FRect *rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_BlendMode blend_mode);
    void draw_outline(SDL_Texture *texture, SDL_FRect *rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_BlendMode blend_mode);
    void draw_ui_box(SDL_Texture *texture, int type, SDL_FRect *rect);
    void draw_text(SDL_Texture *texture, const std::string &str, int font, int x, int y, int w);
    void draw_icon(SDL_Texture *texture, SDL_FRect src_rect, SDL_FRect *dst_rect);
    SDL_FRect draw_health_bar(SDL_Texture *texture, int health, int max_health, int x, int y);
    void draw_hud(SDL_Texture *texture, std::string item, int health, int max_health, int cheese);
    void draw_overlay();

    // game_sprite.cpp

    void set_view(int x, int y);
    void push_sprite(const std::string &tex_id, SDL_Texture *texture, SDL_FRect *src_rect, SDL_FRect *dst_rect, int depth_offset);
    void push_icon(SDL_FRect src_rect, SDL_FRect *dst_rect);

    // defined per game (e.g., map_editor.cpp, init.cpp)

    void init();

    std::string title = "SDL3 Game";
    int argc;
    const char **argv;

    Uint64 ticks = 0;
    Uint64 load_ticks = 0;
    bool displaying_load_screen = false;
    float delta = 0;

    SDL_Texture *screen, *ui, *overlay;

    std::vector<Font *> fonts;

    int corner_x = 0, corner_y = 0; // set in Game::step
    int tile_width = 32, tile_height = 32, cols = 1, rows = 1;
    int *collision = nullptr;

    std::string map = ""; // set this to load a map
    std::string current_map = ""; // readonly
    std::string map_title = "", map_description = ""; // readonly

    bool create_objects = true; // disable for feta launcher
    bool delete_object = false; // set to true from an object's step to delete it

    struct SpriteRender {
        std::string tex_id;
        SDL_Texture *texture = nullptr;
        SDL_FRect *src_rect, *dst_rect;
        int y;

        bool operator<(const SpriteRender &other) const {
            return this->y < other.y;
        }
    };

    // sprite rendering things
    std::vector<SpriteRender> sprites;

private:

    // for Game::texts
    struct Text {
        std::string str;
        int x, y;
    };

    // game_step.cpp

    void load_map(const char *map_path);

    // map things
    SDL_Texture *bg = nullptr, *fg = nullptr;
    Quad colliders[n_MapColliders];

    // object things
    std::vector<Object *> objects;
    std::queue<std::pair<std::string, std::string> > new_objects;
    std::unordered_map<std::string, ObjectFactory *> factories;
    Object *first_obj = nullptr, *last_obj = nullptr;

    // ui things
    SDL_Texture *ui_box = nullptr;
    std::queue<Text> texts;
    SDL_Texture *icons;

    // overlay things
    bool did_clear_overlay = false;
};

extern SDL_Renderer *renderer;
extern Game *game;
extern bool _running;

#endif
