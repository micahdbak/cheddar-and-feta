#pragma once

#include <SDL3/SDL.h>

#include "foes/spitter/spitter.h"
#include "hitbox.h"
#include "hurtbox.h"
#include "object/entity.h"
#include "sprite/sprite.h"
#include "thoom/thoom.h"

#define FRAMES_TO_SET_CUR_DIR 4

constexpr thoom_nid kAbdomenNid = (NUM_SEGMENTS * 3) - 2;
constexpr thoom_nid kAbdomenHurtboxNid = kAbdomenNid + 1;

class SpitterAbdomen : public thoom::Entity::Node, public HitReceiver {
 public:
  explicit SpitterAbdomen(Spitter& spitter);
  ~SpitterAbdomen();

  void step() override;
  void render() override;

  float hurt_x() const override { return this->x; }
  float hurt_y() const override { return this->y; }

  void receive_hit(const Hit& hit) override;

 private:
  Spitter& spitter;
  int cur_dir, dirs[FRAMES_TO_SET_CUR_DIR], dirs_i;
  float x, y;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
};
