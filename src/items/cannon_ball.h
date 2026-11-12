#pragma once

#include <SDL3/SDL.h>

#include "hitbox.h"
#include "items/item.h"
#include "mouse.h"
#include "object/entity.h"
#include "sprite/sprite.h"
#include "thoom/thoom.h"

#define ITEM_CANNON_BALL "item_cannon_ball"

// ---- dropped cannon ball ----

class DroppedCannonBall : public DroppedItem {
 public:
  DroppedCannonBall(const thoom::Entity::Context& context, float x, float y)
      : DroppedItem(context, ITEM_CANNON_BALL, x, y,
                    "sprites/item_cannon_ball.bmp", 16, 16, 0) {}
  ~DroppedCannonBall() = default;

  void take(Mouse* mouse) override { mouse->push_item(ITEM_CANNON_BALL); }
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);
};

// ---- thrown cannon ball ----

class ThrownCannonBall : public thoom::Entity, public HitSource {
 public:
  ThrownCannonBall(const thoom::Entity::Context& context, float x, float y,
                   int x_dir, int y_dir, thoom_gid from_gid);
  ~ThrownCannonBall();

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
