#ifndef MOUSE_OBJ
#define MOUSE_OBJ "mouse"

#include "controller.h"
#include "foes/foe.h"
#include "game.h"
#include "object.h"
#include "sprite.h"

#define MOUSE_DEFAULT_SPEED 64.0f

class Mouse : public Object {
public:
    Mouse(int x, int y, bool is_feta);
    ~Mouse();

    void step() override;

    void save_data() override;
    void post_save_data() override;

    bool attack(int damage);

    bool push_item(const std::string &item_id);
    bool remove_item(const std::string &item_id); // true if no more
    int add_cheese(int amount);

    bool is_feta = true;
    float x, y;

    int throw_x = 0, throw_y = 0;

    std::string name = "Cheddar";

    int health = 10, max_health = 10;
    int damage = 1, armour = 0;

    struct Item {
        std::string item_id;
        int count;
    };

    std::vector<Item> items;
    int sel_item = -1;

    int cheese = 0, max_cheese = 10;

    Foe *closest_foe = nullptr;

    float max_mov_speed = MOUSE_DEFAULT_SPEED;

private:
    SDL_FRect dst_rect, hat_dst, icon_src, icon_dst;
    Sprite *sprite, *hat;

    enum { FALSE, ATTACKING, ATTACKED, THROWING, EATING } is_busy = FALSE;
    Uint64 busy_ticks = 0;

    int check_foe = 0;
    float closest_distance = FLT_MAX;
    bool did_miss = false;

    int tile_x, tile_y;
};

class MouseFactory : public ObjectFactory {
public:
    MouseFactory() {}
    ~MouseFactory() {}

    Object *create(const std::string &options) override {
        int x = 0, y = 0, is_feta;
        if (sscanf(options.c_str(), "%d,%d,%d", &x, &y, &is_feta) < 3) {
            is_feta = 0;
        }
        return new Mouse(x, y, is_feta != 0);
    }
};

Mouse *closest_mouse(float x, float y, float min_distance);

extern bool mice_locked;
extern Mouse *cheddar, *feta;

#endif
