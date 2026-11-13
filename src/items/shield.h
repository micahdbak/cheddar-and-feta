#pragma once

#include <cstdio>

#include "item.h"
#include "mouse.h"
#include "object/entity.h"
#include "thoom/log.h"

#define ITEM_SHIELD "item_shield"

class DroppedShield : public DroppedItem {
 public:
  DroppedShield(const thoom::Entity::Context& context, float x, float y)
      : DroppedItem(context, ITEM_SHIELD, x, y, "sprites/item_shield.bmp", 16,
                    16, 100) {}
  ~DroppedShield() = default;

  void take(Mouse* mouse) override { mouse->push_item(ITEM_SHIELD); }
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options) {
    int x, y;
    if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) {
      thoom_log_fatal("%s: bad options '%s'", __func__, options.c_str());
      return nullptr;
    }

    return new DroppedShield(context, float(x), float(y));
  }
};
