#ifndef ITEM_CANNON_BALL
#define ITEM_CANNON_BALL "item_cannon_ball"

#include "items/item.h"
#include "hitbox.h"

#include <iostream>

// ---- dropped cannon ball ----

class DroppedCannonBall : public DroppedItem {
public:
    DroppedCannonBall(float x, float y): DroppedItem(x, y, "sprites/item_cannon_ball.bmp", 16, 16, 0) {}
    ~DroppedCannonBall() = default;

    void step() override { this->dropped_step(); }

    void take(Mouse *mouse) override {
        mouse->push_item(ITEM_CANNON_BALL);
    }
};

class DroppedCannonBallFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x, y;
        sscanf(options.c_str(), "%d,%d", &x, &y);
        return new DroppedCannonBall(float(x), float(y));
    }
};

// ---- thrown cannon ball ----

class ThrownCannonBall : public Object, public HitSource {
public:
    ThrownCannonBall(float x, float y, int x_dir, int y_dir, int from_id);
    ~ThrownCannonBall();

    void step() override;

    float hitsource_x() override { return this->x; }
    float hitsource_y() override { return this->y; }

    void hitsource_notify() override {
        this->did_hit = true;
    }

private:
    Uint64 spawned_ticks;
    bool drop_item, did_hit;
    float x, y;
    int x_dir, y_dir;
    Sprite *sprite;
    SDL_FRect dst_rect;
};

class ThrownCannonBallFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int x, y, x_dir, y_dir, from_id;
        sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir, &from_id);
        return new ThrownCannonBall((float)x, (float)y, x_dir, y_dir, from_id);
    }
};

#endif
