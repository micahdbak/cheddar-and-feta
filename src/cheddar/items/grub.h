#ifndef ITEM_GRUB
#define ITEM_GRUB "item_grub"

#include "item.h"

class DroppedGrub : public DroppedItem {
public:
    DroppedGrub(float x, float y): DroppedItem(x, y, "sprites/item_grub.bmp", 16, 16, 100) {}
    ~DroppedGrub() = default;

    void step() override { this->dropped_step(); }

    bool take(Mouse *mouse) override {
        return mouse->push_item(ITEM_GRUB);
    }
};

class DroppedGrubFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x, y;
        sscanf(options.c_str(), "%d,%d", &x, &y);
        return new DroppedGrub(float(x), float(y));
    }
};

// ----

class ThrownGrub : public TrackingItem {
public:
    ThrownGrub(float x, float y, int x_dir, int y_dir, int from_id);
    ~ThrownGrub();

    void on_mouse(Mouse *mouse) override;

    void on_foe(Foe *foe) override;

    void step() override;

private:
    void hit_nearby_foes();

    enum State { FLYING, EXPLODING } state = State::FLYING;
    Sprite *sprite;
    SDL_FRect dst_rect;
    int x_dir, y_dir;
    Uint64 timer;
};

class ThrownGrubFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int x, y, x_dir = 0, y_dir = 0, from_id = -1;
        sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir, &from_id);
        return new ThrownGrub((float)x, (float)y, x_dir, y_dir, from_id);
    }
};

#endif
