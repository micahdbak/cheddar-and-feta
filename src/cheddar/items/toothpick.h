#ifndef ITEM_TOOTHPICK
#define ITEM_TOOTHPICK "item_toothpick"

#include "items/item.h"
#include "hitbox.h"

#include <iostream>

// ---- dropped toothpick ----

class DroppedToothpick : public DroppedItem {
public:
    DroppedToothpick(float x, float y)
        : DroppedItem(x, y, "sprites/item_toothpick.bmp", 16, 16, 0) {
        this->sprite->set_animation(1);
    }
    ~DroppedToothpick() = default;

    void step() override { this->dropped_step(); }

    void take(Mouse *mouse) override {
        mouse->push_item(ITEM_TOOTHPICK);
    }
};

class DroppedToothpickFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x, y;
        sscanf(options.c_str(), "%d,%d", &x, &y);
        return new DroppedToothpick(float(x), float(y));
    }
};

// ---- thrown toothpick ----

class ThrownToothpick : public Object, public HitSource {
public:
    ThrownToothpick(float x, float y, int x_dir, int y_dir, int from_id);
    ~ThrownToothpick();

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

class ThrownToothpickFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int x, y, x_dir, y_dir, from_id;
        sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir, &from_id);
        return new ThrownToothpick((float)x, (float)y, x_dir, y_dir, from_id);
    }
};

#endif
