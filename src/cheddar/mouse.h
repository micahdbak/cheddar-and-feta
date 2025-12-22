#ifndef MOUSE_OBJ
#define MOUSE_OBJ "mouse"

#include <vector>
#include <iostream>

#include "controller.h"
#include "foes/foe.h"
#include "game.h"
#include "object.h"
#include "sprite.h"
#include "hitbox.h"

#define MOUSE_DEFAULT_SPEED 80.0f

// save keys
#define MOUSE_X         "_x"
#define MOUSE_Y         "_y"
#define MOUSE_ANIMATION "_animation"
#define MOUSE_ITEMS     "_items"
#define MOUSE_SPAWN_AT  DONT_WRITE "spawn_at"

// animations
#define RUNPREP_ANIMATION   8
#define ATTACKING_ANIMATION 16
#define ATTACKED_ANIMATION  24
#define THROWING_ANIMATION  32
#define DOWN_ANIMATION      40
#define EATING_ANIMATION    48
#define DANCING_ANIMATION   49
#define SLEEPING_ANIMATION  50

class Mouse : public Object {
public:
    struct SpawnCoord {
        float x, y;
        int animation;
    };

    Mouse() = default;
    ~Mouse() = default;

    virtual void attack(int damage) = 0;
    virtual void push_item(const std::string &item_id) = 0;
    virtual void push_cheese(int amount) = 0;
    virtual void remove_item(const std::string &item_id) = 0;
    virtual void force_dance(Uint64 timeout_ms) = 0;
    virtual void set_throw(int throw_x, int throw_y) = 0;
    virtual void set_max_mov_speed(float max_mov_speed) = 0;
    virtual void signal_down() = 0;

    static bool check_collision(float x, float y);
    static std::string encode_items(const std::vector<Game::HudItem> &items);
    static std::vector<Game::HudItem> read_items(const std::string &items_s);
    static Mouse *closest_mouse(float x, float y, float min_distance, bool forced);

    float x, y;
    bool is_down, is_feta;
};

extern Mouse *cheddar, *feta;
extern bool mice_locked;

#endif
