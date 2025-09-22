#ifndef ITEM_MOLOTOV
#define ITEM_MOLOTOV "item_molotov"

#include "item.h"
#include "hitbox.h"

class DroppedMolotov : public DroppedItem {
public:
    DroppedMolotov(float x, float y):
        DroppedItem(ITEM_MOLOTOV, x, y, "sprites/item_molotov.bmp", 16, 16, 100) {}
    ~DroppedMolotov() = default;

    void step() override { this->dropped_step(); }

    void take(Mouse *mouse) override {
        mouse->push_item(ITEM_MOLOTOV);
    }
};

class DroppedMolotovFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x, y;
        sscanf(options.c_str(), "%d,%d", &x, &y);
        return new DroppedMolotov(float(x), float(y));
    }
};

// ----

class ThrownMolotov : public Object, public HitSource {
public:
    ThrownMolotov(float x, float y, int x_dir, int y_dir, int from_id);
    ~ThrownMolotov();

    void step() override;

    float hitsource_x() override { return this->x; }
    float hitsource_y() override { return this->y; }

    void hitsource_notify() override {
        this->did_hit = true;
    }

private:
    float x, y;
    bool did_hit;
    Sprite *sprite;
    SDL_FRect dst_rect;
    int x_dir, y_dir;
    Uint64 timer;
};

class ThrownMolotovFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int x, y, x_dir = 0, y_dir = 0, from_id = -1;
        sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir, &from_id);
        return new ThrownMolotov((float)x, (float)y, x_dir, y_dir, from_id);
    }
};

#endif
