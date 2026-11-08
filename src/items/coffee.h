#pragma once

#include <cstdio>
#include <cstdlib>

#include "item.h"
#include "mouse.h"
#include "object.h"
#include "scene/scene.h"

#define ITEM_COFFEE_BEAN "item_coffee_bean"

class DroppedCoffeeBean : public DroppedItem {
 public:
  DroppedCoffeeBean(float x, float y)
      : DroppedItem(ITEM_COFFEE_BEAN, x, y, "sprites/item_coffee_bean.bmp", 16,
                    16, 0) {}
  ~DroppedCoffeeBean() = default;

  void step() override { this->dropped_step(); }

  void take(IMouse* mouse) override { mouse->push_item(ITEM_COFFEE_BEAN); }
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options) {
    int x, y;
    if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) {
      std::exit(1);
    }

    return new DroppedCoffeeBean(float(x), float(y));
  }
};

class UsedCoffeeBean : public thoom::Object {
 public:
  UsedCoffeeBean(bool is_feta) : is_feta(is_feta) {
    this->timer = thoom::Scene::instance->ticks + 5000;

    IMouse* mouse = is_feta ? remote_feta : cheddar;
    if (mouse != nullptr) mouse->set_max_mov_speed(MOUSE_DEFAULT_SPEED * 2.0f);
  }
  ~UsedCoffeeBean() = default;

  void step() override {
    IMouse* mouse = is_feta ? remote_feta : cheddar;

    if (mouse == nullptr) {
      this->deleted = true;
      return;
    }

    if (thoom::Scene::instance->ticks > this->timer) {
      mouse->set_max_mov_speed(MOUSE_DEFAULT_SPEED);
      this->deleted = true;
    }
  }
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options) {
    int from_id;
    if (1 != sscanf(options.c_str(), "%*d,%*d,%*d,%*d,%d", &from_id)) {
      std::exit(1);
    }

    bool is_feta = remote_feta != nullptr && remote_feta->id == from_id;

    return new UsedCoffeeBean(is_feta);
  }

 private:
  bool is_feta;
  Uint64 timer = 0;
};
