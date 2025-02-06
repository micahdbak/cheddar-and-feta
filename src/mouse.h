#ifndef MOUSE_OBJ
#define MOUSE_OBJ "mouse"

#include "enemy.h"
#include "game.h"
#include "object.h"
#include "sprite.h"

class Mouse : public Object {
public:
    Mouse(int x, int y);
    ~Mouse();

    void step() override;

    void save_data() override;
    void post_save_data() override;

    bool attack(int damage);

    int free_space() { return this->max_items - this->items.size(); }
    void push_item(const std::string &item_id);

    bool locked = false;
    float x, y;

    int throw_x = 0, throw_y = 0;

    int cheese = 0, max_cheese = 3;
    int health = 10, max_health = 10;
    int damage = 1, armour = 0;

    std::string name = "Cheddar";

private:
    SDL_FRect dst_rect;
    Sprite *sprite;

    enum { FALSE, ATTACKING, ATTACKED, THROWING, EATING } is_busy = FALSE;
    Uint64 busy_ticks = 0;
    bool did_miss = false;
    SDL_FRect miss_rect;

    int check_enemy = 0;
    Enemy *closest_enemy = nullptr;
    float closest_distance = FLT_MAX;

    std::vector<std::string> items;
    int max_items = 4;
};

class MouseFactory : public ObjectFactory {
public:
    MouseFactory() {}
    ~MouseFactory() {}

    Object *create(const std::string &options) override {
        int x, y;
        if (sscanf(options.c_str(), "%d,%d", &x, &y) < 2) {
            x = 0, y = 0;
        }
        return new Mouse(x, y);
    }
};

extern Mouse *mouse;

#endif
