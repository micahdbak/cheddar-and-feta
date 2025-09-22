#ifndef ITEM_FROG_TONGUE
#define ITEM_FROG_TONGUE "item_frog_tongue"

#include "item.h"
#include "hitbox.h"

class DroppedFrogTongue : public DroppedItem {
public:
    DroppedFrogTongue(float x, float y):
        DroppedItem(ITEM_FROG_TONGUE, x, y, "sprites/item_frog_tongue.bmp", 16, 16, 0) {}
    ~DroppedFrogTongue() = default;

    void step() override { this->dropped_step(); }

    void take(Mouse *mouse) override {
        mouse->push_item(ITEM_FROG_TONGUE);
    }
};

class DroppedFrogTongueFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x, y;
        sscanf(options.c_str(), "%d,%d", &x, &y);
        return new DroppedFrogTongue(float(x), float(y));
    }
};

// use obj

class FrogTongue : public Object, public HitSource {
public:
    FrogTongue(float x, float y, int x_dir, int y_dir, int from_id);
    ~FrogTongue();

    void step() override;

    float hitsource_x() override { return this->x; }
    float hitsource_y() override { return this->y; }

private:
    float x, y;
    Sprite *sprite;
    SDL_FRect dst_rect;
    int x_dir, y_dir;
    Uint64 ticks;

    bool did_hit = false;
};

class FrogTongueFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x, y, x_dir, y_dir, from_id;
        sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir, &from_id);
        return new FrogTongue(float(x), float(y), x_dir, y_dir, from_id);
    }
};

#endif
