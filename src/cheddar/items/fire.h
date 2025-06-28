#ifndef ITEM_FIRE
#define ITEM_FIRE "item_fire"

#include "item.h"

class Fire : public TrackingItem {
public:
    Fire(float x, float y, int x_dir, int y_dir);
    ~Fire();

    void on_mouse(Mouse *mouse) override;

    void on_foe(Foe *foe) override;

    void step() override;

private:
    enum State { MOVING, STATIONARY } state = MOVING;
    Sprite *sprite;
    int x_dir, y_dir;
    Uint32 timer, attack_timer = 0;
    SDL_FRect dst_rect;
    float mov_speed;
};

class FireFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int x, y, x_dir = 0, y_dir = 0, from_id = -1;
        sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir, &from_id);
        return new Fire((float)x, (float)y, x_dir, y_dir);
    }
};

#endif
