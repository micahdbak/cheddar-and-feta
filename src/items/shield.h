#pragma once

#include <cstdio>
#include <cstdlib>

#include "item.h"
#include "mouse.h"
#include "object.h"
#include "scene/scene.h"

#define ITEM_SHIELD "item_shield"

class DroppedShield : public DroppedItem {
 public:
  DroppedShield(float x, float y)
      : DroppedItem(ITEM_SHIELD, x, y, "sprites/item_shield.bmp", 16, 16, 100) {
  }
  ~DroppedShield() = default;

  void step() override { this->dropped_step(); }

  void take(Mouse* mouse) override { mouse->push_item(ITEM_SHIELD); }
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options) {
    int x, y;
    if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) {
      std::exit(1);
    }

    return new DroppedShield(float(x), float(y));
  }
};
