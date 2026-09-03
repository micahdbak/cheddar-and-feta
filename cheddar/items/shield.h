#ifndef ITEM_SHIELD
#define ITEM_SHIELD "item_shield"

#include "item.h"
#include "object.h"

class DroppedShield : public DroppedItem {
 public:
  DroppedShield(float x, float y)
      : DroppedItem(ITEM_SHIELD, x, y, "sprites/item_shield.bmp", 16, 16, 100) {
  }
  ~DroppedShield() = default;

  void step() override { this->dropped_step(); }

  void take(Mouse* mouse) override { mouse->push_item(ITEM_SHIELD); }
};

class DroppedShieldFactory : public ObjectFactory {
 public:
  Object* create(const std::string& options) override {
    int x, y;
    if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) FATAL_ERROR

    return new DroppedShield(float(x), float(y));
  }
};

#endif
