#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "foe.h"
#include "hitbox.h"
#include "mouse.h"
#include "object/entity.h"
#include "sprite/sprite.h"

#define FOE_BUG_OBJ "foe_bug"

class FoeBug : public Foe, public HitSource {
 public:
  FoeBug(const thoom::Entity::Context& context, int x, int y, int spawner_id);
  ~FoeBug();

  void action(Mouse* mouse) override;
  void attack_internal(int damage) override;

  void step() override;
  void render() override;

  float hitsource_x() override { return this->x + this->off_x; }
  float hitsource_y() override { return this->y + this->off_y; }

  Foe::State prev_state = Foe::State::IDLE;
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

 private:
  thoom::Sprite* sprite;
  SDL_FRect dst_rect, icon_src, icon_dst;
  Uint64 timer = 0, hurt_timer = 0;

  int max_health = 4, health = 4;
};
