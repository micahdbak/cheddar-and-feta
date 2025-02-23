#ifndef MOUSE_OBJ
#define MOUSE_OBJ "mouse"

#include "controller.h"
#include "enemy.h"
#include "game.h"
#include "object.h"
#include "sprite.h"

class Mouse : public Object {
public:
    Mouse(int x, int y, bool is_feta);
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

    int health = 10, max_health = 10;
    int damage = 1, armour = 0;

    std::string attack_item = "";

    std::string name = "Cheddar";

private:
    bool is_feta = true;

    SDL_FRect dst_rect;
    Sprite *sprite;

    enum { FALSE, ATTACKING, ATTACKED, THROWING, EATING } is_busy = FALSE;
    Uint64 busy_ticks = 0;

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
