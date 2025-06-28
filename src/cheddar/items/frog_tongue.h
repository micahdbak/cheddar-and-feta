#ifndef ITEM_FROG_TONGUE
#define ITEM_FROG_TONGUE "item_frog_tongue"

#include "item.h"

class DroppedFrogTongue : public DroppedItem {
public:
    DroppedFrogTongue(float x, float y): DroppedItem(x, y, "sprites/item_frog_tongue.bmp", 16, 16, 0) {}
    ~DroppedFrogTongue() = default;

    void step() override { this->dropped_step(); }

    bool take(Mouse *mouse) override {
        return mouse->push_item(ITEM_FROG_TONGUE);
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

class FrogTongue : public TrackingItem {
public:
    FrogTongue(float x, float y, int x_dir, int y_dir, int from_id);
    ~FrogTongue();

    void step() override;
    void on_foe(Foe *foe) override;
    void on_mouse(Mouse *mouse) override;

private:
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
