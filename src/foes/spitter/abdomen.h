#pragma once

#include <SDL3/SDL.h>

#include "foes/spitter/spitter.h"
#include "hurtbox.h"
#include "object/entity.h"
#include "sprite/sprite.h"

#define FRAMES_TO_SET_CUR_DIR 4

class SpitterAbdomen : public thoom::Entity::Node, public HurtSource {
 public:
  explicit SpitterAbdomen(Spitter& spitter);
  ~SpitterAbdomen();

  void step() override;
  void render() override;

  float hurtsource_x() override { return this->x; }
  float hurtsource_y() override { return this->y; }

  void attack(int damage) override;

 private:
  Spitter& spitter;
  int cur_dir, dirs[FRAMES_TO_SET_CUR_DIR], dirs_i;
  float x, y;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
};
