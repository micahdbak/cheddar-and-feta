#ifndef MOUSE_OBJ
#define MOUSE_OBJ "mouse"

#include "enemy.h"
#include "object.h"
#include "sprite.h"

class Mouse : public Object {
public:
    Mouse(int x, int y);
    ~Mouse();

    void step() override;

    void save_data() override;
    void post_save_data() override;

    bool attack();

    bool locked = false;
    float x, y;

private:
    SDL_FRect dst_rect;
    Sprite *sprite;

    enum { FALSE, ATTACKING, ATTACKED } is_attack = FALSE;
    Uint64 attack_ticks = 0;

    int check_enemy = 0;
    Enemy *closest_enemy = nullptr;
    float closest_distance = 9999999.0f;
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
