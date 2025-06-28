#ifndef ITEM_MOLOTOV
#define ITEM_MOLOTOV "item_molotov"

#include "item.h"

class DroppedMolotov : public DroppedItem {
public:
    DroppedMolotov(float x, float y): DroppedItem(x, y, "sprites/item_molotov.bmp", 16, 16, 100) {}
    ~DroppedMolotov() = default;

    void step() override { this->dropped_step(); }

    bool take(Mouse *mouse) override {
        return mouse->push_item(ITEM_MOLOTOV);
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

class ThrownMolotov : public TrackingItem {
public:
    ThrownMolotov(float x, float y, int x_dir, int y_dir, int from_id);
    ~ThrownMolotov();

    void on_mouse(Mouse *mouse) override;

    void on_foe(Foe *foe) override;

    void step() override;

private:
    void spawn_fire();

    enum State { FLYING, DELETE_OBJ } state = State::FLYING;
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
