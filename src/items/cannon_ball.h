#pragma once

#include "hitbox.h"
#include "items/item.h"
#include "mouse.h"
#include "object.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "thoom/thoom.h"

#define ITEM_CANNON_BALL "item_cannon_ball"

// ---- dropped cannon ball ----

class DroppedCannonBall : public DroppedItem {
 public:
  DroppedCannonBall(float x, float y)
      : DroppedItem(ITEM_CANNON_BALL, x, y, "sprites/item_cannon_ball.bmp", 16,
                    16, 0) {}
  ~DroppedCannonBall() = default;

  void step() override { this->dropped_step(); }

  void take(Mouse* mouse) override { mouse->push_item(ITEM_CANNON_BALL); }
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);
};

// ---- thrown cannon ball ----

class ThrownCannonBall : public thoom::Object, public HitSource {
 public:
  ThrownCannonBall(float x, float y, int x_dir, int y_dir, thoom_lid from_lid);
  ~ThrownCannonBall();

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
