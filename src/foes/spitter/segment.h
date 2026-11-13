#pragma once

#include <SDL3/SDL.h>

#include "foes/spitter/spitter.h"
#include "hitbox.h"
#include "hurtbox.h"
#include "object/entity.h"
#include "sprite/sprite.h"
#include "thoom/thoom.h"

#define FRAMES_TO_SET_CUR_DIR 4

constexpr thoom_nid spitter_segment_nid(int segment_id) {
  return static_cast<thoom_nid>((segment_id * 3) - 2);
}

constexpr thoom_nid spitter_segment_hurtbox_nid(int segment_id) {
  return spitter_segment_nid(segment_id) + 1;
}

class SpitterSegment : public thoom::Entity::Node,
                       public HitReceiver,
                       public HitSource {
 public:
  SpitterSegment(Spitter& spitter, int segment_id);
  ~SpitterSegment();

  void step() override;
  void render() override;

  float hurt_x() const override { return this->x; }
  float hurt_y() const override { return this->y; }

  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }

  void receive_hit(const Hit&) override {}

 private:
  Spitter& spitter;
  int segment_id, cur_dir, dirs[FRAMES_TO_SET_CUR_DIR], dirs_i;
  float x, y;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
};
