#pragma once

#include <cstdio>
#include <cstdlib>

#include "item.h"
#include "mouse.h"
#include "object/entity.h"

#define ITEM_HERMES_BOOT "item_hermes_boot"

class DroppedHermesBoot : public DroppedItem {
 public:
  DroppedHermesBoot(const thoom::Entity::Context& context, float x, float y)
      : DroppedItem(context, ITEM_HERMES_BOOT, x, y,
                    "sprites/item_hermes_boot.bmp", 16, 16, 100) {}
  ~DroppedHermesBoot() = default;

  void take(Mouse* mouse) override { mouse->push_item(ITEM_HERMES_BOOT); }
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options) {
    int x, y;
    if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) {
      std::exit(1);
    }

    return new DroppedHermesBoot(context, float(x), float(y));
  }
};
