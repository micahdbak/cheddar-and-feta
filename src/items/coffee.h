#pragma once

#include <SDL3/SDL.h>

#include <cstdio>
#include <optional>

#include "item.h"
#include "mouse.h"
#include "object/entity.h"
#include "scene/scene.h"
#include "thoom/log.h"

#define ITEM_COFFEE_BEAN "item_coffee_bean"

class DroppedCoffeeBean : public DroppedItem {
 public:
  DroppedCoffeeBean(const thoom::Entity::Context& context, float x, float y)
      : DroppedItem(context, ITEM_COFFEE_BEAN, x, y,
                    "sprites/item_coffee_bean.bmp", 16, 16, 0) {}
  ~DroppedCoffeeBean() = default;

  void take(Mouse* mouse) override { mouse->push_item(ITEM_COFFEE_BEAN); }
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options) {
    int x, y;
    if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) {
      thoom_log_fatal("%s: bad options '%s'", __func__, options.c_str());
      return nullptr;
    }

    return new DroppedCoffeeBean(context, float(x), float(y));
  }
};

class UsedCoffeeBean : public thoom::Entity::Node {
 public:
  explicit UsedCoffeeBean(Mouse& mouse)
      : thoom::Entity::Node(mouse, std::nullopt), mouse(mouse) {
    this->timer = thoom::Scene::instance->ticks + 5000;
    mouse.set_max_mov_speed(MOUSE_DEFAULT_SPEED * 2.0f);
  }
  ~UsedCoffeeBean() = default;

  void step() override {
    if (thoom::Scene::instance->ticks > this->timer) {
      this->mouse.set_max_mov_speed(MOUSE_DEFAULT_SPEED);
      this->deleted = true;
    }
  }

 private:
  Mouse& mouse;
  Uint64 timer = 0;
};
