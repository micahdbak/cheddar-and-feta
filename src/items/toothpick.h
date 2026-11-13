#pragma once

#include <SDL3/SDL.h>

#include "hitbox.h"
#include "items/item.h"
#include "mouse.h"
#include "object/entity.h"
#include "thoom/thoom.h"

#define ITEM_TOOTHPICK "item_toothpick"

// ---- dropped toothpick ----

class DroppedToothpick : public DroppedItem {
 public:
  DroppedToothpick(const thoom::Entity::Context& context, float x, float y)
      : DroppedItem(context, ITEM_TOOTHPICK, x, y, "sprites/item_toothpick.bmp",
                    16, 16, 0) {
    this->sprite->set_animation(1);
  }
  ~DroppedToothpick() = default;

  void take(Mouse* mouse) override { mouse->push_item(ITEM_TOOTHPICK); }
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);
};

// ---- thrown toothpick ----

class ThrownToothpick : public thoom::Entity, public HitSource {
 public:
  ThrownToothpick(const thoom::Entity::Context& context, float x, float y,
                  int x_dir, int y_dir, thoom_gid from_gid);
  ~ThrownToothpick();

  void step() override;
  void render() override;

  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }

  void hitsource_notify() override;
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

 private:
  Uint64 spawned_ticks;
  bool drop_item, did_hit;
  float x, y;
  int x_dir, y_dir;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
};
