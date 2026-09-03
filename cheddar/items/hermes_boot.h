#ifndef ITEM_HERMES_BOOT
#define ITEM_HERMES_BOOT "item_hermes_boot"

#include "item.h"
#include "object.h"

class DroppedHermesBoot : public DroppedItem {
 public:
  DroppedHermesBoot(float x, float y)
      : DroppedItem(ITEM_HERMES_BOOT, x, y, "sprites/item_hermes_boot.bmp", 16,
                    16, 100) {}
  ~DroppedHermesBoot() = default;

  void step() override { this->dropped_step(); }

  void take(Mouse* mouse) override { mouse->push_item(ITEM_HERMES_BOOT); }
};

class DroppedHermesBootFactory : public ObjectFactory {
 public:
  Object* create(const std::string& options) override {
    int x, y;
    if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) FATAL_ERROR

    return new DroppedHermesBoot(float(x), float(y));
  }
};

#endif
