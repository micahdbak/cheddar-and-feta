#pragma once

#include <SDL3/SDL.h>

#include "foes/spitter/spitter.h"
#include "hitbox.h"
#include "hurtbox.h"
#include "object/entity.h"
#include "sprite/sprite.h"

#define FRAMES_TO_SET_CUR_DIR 4

class SpitterSegment : public thoom::Entity::Node,
                       public HurtSource,
                       public HitSource {
 public:
  SpitterSegment(Spitter& spitter, int segment_id);
  ~SpitterSegment();

  void step() override;
  void render() override;

  float hurtsource_x() override { return this->x; }
  float hurtsource_y() override { return this->y; }

  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }

  void attack(int) override {}

 private:
  Spitter& spitter;
  int segment_id, cur_dir, dirs[FRAMES_TO_SET_CUR_DIR], dirs_i;
  float x, y;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
};
