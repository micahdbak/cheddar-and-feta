#ifndef ITEM_SHIELD
#define ITEM_SHIELD "item_shield"

#include "object.h"
#include "item.h"

class DroppedShield : public DroppedItem {
public:
    DroppedShield(float x, float y):
        DroppedItem(ITEM_SHIELD, x, y, "sprites/item_shield.bmp", 16, 16, 100) {}
    ~DroppedShield() = default;

    void step() override { this->dropped_step(); }

    void take(Mouse *mouse) override {
        mouse->push_item(ITEM_SHIELD);
    }
};

class DroppedShieldFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x, y;
        sscanf(options.c_str(), "%d,%d", &x, &y);
        return new DroppedShield(float(x), float(y));
    }
};

#endif
