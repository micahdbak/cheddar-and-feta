#pragma once

#include "item.h"
#include "object.h"

#define ITEM_SHIELD "item_shield"

class DroppedShield : public DroppedItem {
 public:
  DroppedShield(float x, float y)
      : DroppedItem(ITEM_SHIELD, x, y, "sprites/item_shield.bmp", 16, 16, 100) {
  }
  ~DroppedShield() = default;

  void step() override { this->dropped_step(); }

  void take(Mouse* mouse) override { mouse->push_item(ITEM_SHIELD); }
};

class DroppedShieldFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) override {
    int x, y;
    if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) FATAL_ERROR

    return new DroppedShield(float(x), float(y));
  }
};
