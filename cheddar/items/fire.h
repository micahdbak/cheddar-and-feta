#pragma once

#include "hitbox.h"
#include "item.h"

#define ITEM_FIRE "item_fire"

class Fire : public thoom::Object, public HitSource {
 public:
  Fire(float x, float y, int x_dir, int y_dir);
  ~Fire();

  void step() override;

  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }

 private:
  float x, y;
  enum State { MOVING, STATIONARY } state = MOVING;
  thoom::Sprite* sprite;
  int x_dir, y_dir, throw_time;
  Uint64 timer, attack_timer = 0;
  SDL_FRect dst_rect;
  float mov_speed;
};

class FireFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) {
    int x, y, x_dir = 0, y_dir = 0, from_id = -1;
    if (5 != sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir,
                    &from_id))
      FATAL_ERROR

    return new Fire((float)x, (float)y, x_dir, y_dir);
  }
};
