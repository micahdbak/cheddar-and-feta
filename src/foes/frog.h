#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "foe.h"
#include "mouse.h"
#include "object/entity.h"
#include "sprite/sprite.h"

#define FOE_FROG_OBJ "foe_frog"

class FoeFrog : public Foe {
 public:
  FoeFrog(const thoom::Entity::Context& context, int x, int y, int spawner_id);
  ~FoeFrog();

  void action(Mouse* mouse) override;
  void attack_internal(int damage) override;

  void step() override;
  void render() override;

  Foe::State prev_state = Foe::State::IDLE;
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

 private:
  thoom::Sprite* sprite;
  SDL_FRect dst_rect, icon_src, icon_dst;
  Uint64 timer = 0, attack_timer = 0, hurt_timer = 0;

  int max_health = 10, health = 10;
};
