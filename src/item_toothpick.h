#ifndef ITEM_TOOTHPICK
#define ITEM_TOOTHPICK "item.toothpick"

#include "inventory.h"
#include "item.h"

#include <iostream>

// ---- dropped toothpick ----

class DroppedToothpick : public DroppedItem {
public:
    DroppedToothpick(float x, float y)
        : DroppedItem(x, y, "sprites/toothpick.bmp", 16, 16, 0) {
        this->sprite->set_animation(1);
    }
    ~DroppedToothpick() = default;

    std::string prompt_text() override {
        return "A toothpick is on the floor. Pick it up?";
    }

    std::string leave_text() override {
        return "Who needs a toothpick anyways.";
    }

    std::pair<bool, std::string> take() override {
        if (!inventory->push_item(ITEM_TOOTHPICK)) {
            return { false, "Your inventory is full." };
        }

        return { true, "You took the toothpick." };
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

    void on_hit(Enemy *enemy) override;

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
