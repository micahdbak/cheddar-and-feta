#pragma once

#include <string>

#include "hitbox.h"
#include "object.h"
#include "scene/scene.h"
#include "sprite/sprite.h"

#define ITEM_FIRE "item_fire"

class Fire : public thoom::Object, public HitSource {
 public:
  Fire(float x, float y, int x_dir, int y_dir);
  ~Fire();

  void step() override;

  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  float x, y;
  enum State { MOVING, STATIONARY } state = MOVING;
  thoom::Sprite* sprite;
  int x_dir, y_dir, throw_time;
  Uint64 timer, attack_timer = 0;
  SDL_FRect dst_rect;
  float mov_speed;
};
