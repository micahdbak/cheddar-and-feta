#ifndef ITEM_TOOTHPICK
#define ITEM_TOOTHPICK "item_toothpick"

#include "items/item.h"

#include <iostream>

// ---- dropped toothpick ----

class DroppedToothpick : public DroppedItem {
public:
    DroppedToothpick(float x, float y)
        : DroppedItem(x, y, "sprites/item_toothpick.bmp", 16, 16, 0) {
        this->sprite->set_animation(1);
    }
    ~DroppedToothpick() = default;

    bool take(Mouse *mouse) override {
        return mouse->push_item(ITEM_TOOTHPICK);
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

class ThrownToothpick : public ThrownItem {
public:
    ThrownToothpick(float x, float y, int x_dir, int y_dir);
    ~ThrownToothpick();

    void step() override;

    void on_hit(Foe *foe) override;

    void on_miss() override;

private:
    Sprite *sprite;
    SDL_FRect dst_rect;
};

class ThrownToothpickFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        float x, y;
        int x_dir, y_dir;
        ThrownItem::ParseOptions(options, &x, &y, &x_dir, &y_dir);
        return new ThrownToothpick(x, y, x_dir, y_dir);
    }
};

#endif
