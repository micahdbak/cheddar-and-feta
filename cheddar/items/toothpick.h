#pragma once

#include <iostream>

#include "hitbox.h"
#include "items/item.h"
#include "scene.h"

#define ITEM_TOOTHPICK "item_toothpick"

// ---- dropped toothpick ----

class DroppedToothpick : public DroppedItem {
 public:
  DroppedToothpick(float x, float y)
      : DroppedItem(ITEM_TOOTHPICK, x, y, "sprites/item_toothpick.bmp", 16, 16,
                    0) {
    this->sprite->set_animation(1);
  }
  ~DroppedToothpick() = default;

  void step() override { this->dropped_step(); }

  void take(Mouse* mouse) override { mouse->push_item(ITEM_TOOTHPICK); }
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);
};

// ---- thrown toothpick ----

class ThrownToothpick : public thoom::Object, public HitSource {
 public:
  ThrownToothpick(float x, float y, int x_dir, int y_dir, int from_id);
  ~ThrownToothpick();

  void step() override;

  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }

  void hitsource_notify() override;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  Uint64 spawned_ticks;
  bool drop_item, did_hit;
  float x, y;
  int x_dir, y_dir;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
};
