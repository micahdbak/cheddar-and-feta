#pragma once

#include <iostream>

#include "item.h"
#include "mouse.h"

#define ITEM_COFFEE_BEAN "item_coffee_bean"

class DroppedCoffeeBean : public DroppedItem {
 public:
  DroppedCoffeeBean(float x, float y)
      : DroppedItem(ITEM_COFFEE_BEAN, x, y, "sprites/item_coffee_bean.bmp", 16,
                    16, 0) {}
  ~DroppedCoffeeBean() = default;

  void step() override { this->dropped_step(); }

  void take(Mouse* mouse) override { mouse->push_item(ITEM_COFFEE_BEAN); }
};

class DroppedCoffeeBeanFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) override {
    int x, y;
    if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) FATAL_ERROR

    return new DroppedCoffeeBean(float(x), float(y));
  }
};

class UsedCoffeeBean : public thoom::Object {
 public:
  UsedCoffeeBean(bool is_feta) : is_feta(is_feta) {
    this->timer = thoom::game->ticks + 5000;

    Mouse* mouse = is_feta ? feta : cheddar;
    if (mouse != nullptr) mouse->set_max_mov_speed(MOUSE_DEFAULT_SPEED * 2.0f);
  }
  ~UsedCoffeeBean() = default;

  void step() override {
    Mouse* mouse = is_feta ? feta : cheddar;

    if (mouse == nullptr) {
      thoom::game->delete_object = true;
      return;
    }

    if (thoom::game->ticks > this->timer) {
      mouse->set_max_mov_speed(MOUSE_DEFAULT_SPEED);
      thoom::game->delete_object = true;
    }
  }

 private:
  bool is_feta;
  Uint64 timer = 0;
};

class UsedCoffeeBeanFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) override {
    int from_id;
    if (1 != sscanf(options.c_str(), "%*d,%*d,%*d,%*d,%d", &from_id))
      FATAL_ERROR

    bool is_feta = feta != nullptr && feta->id == from_id;

    return new UsedCoffeeBean(is_feta);
  }
};
